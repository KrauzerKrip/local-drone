#include "cable_system.h"
#include <algorithm>
#include <entt/entity/fwd.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <vector>
#include "lc_client/eng_graphics/entt/components.h"
#include "lc_client/eng_physics/entt/components.h"
#include "lc_client/eng_physics/physics.h"
#include "lc_client/exceptions/component_exception.h"
#include <limits>

CableSystem::CableSystem(Physics* pPhysics, entt::registry* pRegistry) {
	m_pPhysics = pPhysics;
	m_pRegistry = pRegistry;
	m_gravity = 9.81;
}

void CableSystem::update(double updateInterval) {
	const int substeps = 10;
	const int solverIterations = 3;

	std::vector<CollisionHit> hits;
	hits.reserve(8);
	constexpr float particleRadius = 0.01f;

	for (auto&& [entity, cable] : m_pRegistry->view<Cable>().each()) {
		std::vector<bool> particleHadCollision(cable.particles.size(), false);
		std::vector<glm::vec3> particleCollisionNormal(cable.particles.size(), glm::vec3(0.0f));
		for (int substep = 0; substep < substeps; substep++) {
			const float deltaTime = updateInterval / substeps;
			std::fill(particleHadCollision.begin(), particleHadCollision.end(), false);
			std::fill(particleCollisionNormal.begin(), particleCollisionNormal.end(), glm::vec3(0.0f));

			for (CableParticle& particle : cable.particles) {
				particle.prevPosition = particle.position;
				if (particle.inverseMass == 0.0f) {
					continue;
				}
				this->applyExternalForces(particle, deltaTime);
			}

			for (CableDistanceConstraint& constraint : cable.constraints) {
				constraint.lambda = 0.0f;
			}

			const float collisionMargin = 0.25f;
			CableBounds cableBounds = computeCableBounds(cable);
			cableBounds.size += glm::vec3(particleRadius + collisionMargin);

			cable.colliderCandidates.clear();
			m_pPhysics->queryAABBOverlaps(
				AABBOverlapQuery{.position = cableBounds.position, .size = cableBounds.size}, cable.colliderCandidates);

			for (int iter = 0; iter < solverIterations; iter++) {
				// Distance constraints
				for (CableDistanceConstraint& constraint : cable.constraints) {
					glm::vec3 correction = this->calculateDistanceConstraint(cable, constraint, deltaTime);
					auto& particleA = cable.particles[constraint.indexParticleA];
					auto& particleB = cable.particles[constraint.indexParticleB];
					particleA.position += particleA.inverseMass * correction;
					particleB.position -= particleB.inverseMass * correction;
				}

				// Collision constraints
				cable.collisionConstraints.clear();

				for (size_t i = 0; i < cable.particles.size(); i++) {
					hits.clear();
					auto& particle = cable.particles[i];
					SphereOverlapQuery query = {.center = particle.position, .radius = particleRadius};
					for (CollisionHit& cableCandidate : cable.colliderCandidates) {
						auto maybeCollisionHit = m_pPhysics->sphereIntersect(
							query, cableCandidate.ownerEntity, cableCandidate.colliderEntity);
						if (maybeCollisionHit) {
							CollisionHit hit = *maybeCollisionHit;
							cable.collisionConstraints.push_back({.particleIndex = i,
								.normal = hit.normal,
								.point = hit.point,
								.particleRadius = particleRadius,
								.lambda = 0.0f,
								.compliance = 0.0f});

							particleHadCollision[i] = true;
							particleCollisionNormal[i] += hit.normal;
						}
					}
				}

				for (CableCollisionConstraint& constraint : cable.collisionConstraints) {
					glm::vec3 correction = this->calculateCollisionConstraint(cable, constraint, deltaTime);
					CableParticle& particle = cable.particles[constraint.particleIndex];
					particle.position += particle.inverseMass * correction;
				}
			}

			// Update velocities from final corrected positions
			for (size_t i = 0; i < cable.particles.size(); i++) {
				CableParticle& particle = cable.particles[i];
				if (particle.inverseMass == 0.0f) {
					continue;
				}
				particle.linearVelocity = (particle.position - particle.prevPosition) / deltaTime;
				if (particleHadCollision[i] &&
					glm::dot(particleCollisionNormal[i], particleCollisionNormal[i]) > 0.0f) {
					glm::vec3 n = glm::normalize(particleCollisionNormal[i]);
					float vn = glm::dot(particle.linearVelocity, n);
					if (vn < 0.0f) {
						particle.linearVelocity -= vn * n;
					}
					// crude damping/friction
					particle.linearVelocity *= 0.95f;
				}
			}
		}

		if (m_pRegistry->any_of<PrimitiveLines>(entity)) {
			auto& lines = m_pRegistry->get<PrimitiveLines>(entity);
			for (size_t i = 1; i < cable.particles.size(); i++) {
				auto& particleA = cable.particles[i - 1];
				auto& particleB = cable.particles[i];
				lines.lines[i - 1].startPoint = particleA.position;
				lines.lines[i - 1].endPoint = particleB.position;
			}
		}
		else {
			auto& lines = m_pRegistry->emplace<PrimitiveLines>(entity);
			lines.lines.reserve(cable.particles.size());
			for (size_t i = 1; i < cable.particles.size(); i++) {
				auto& particleA = cable.particles[i - 1];
				auto& particleB = cable.particles[i];
				lines.lines.push_back(PrimitiveLine(particleA.position, particleB.position, glm::vec3(0, 0, 1)));
			}
		}
	}
}

void CableSystem::connectWithCable(entt::entity anchor, entt::entity attachment) {
	if (!m_pRegistry->any_of<CableAnchor>(anchor)) {
		throw ComponentNotFoundException("CableAnchor");
	}
	if (!m_pRegistry->any_of<CableAttachment>(attachment)) {
		throw ComponentNotFoundException("CableAttachment");
	}
}

glm::vec3 CableSystem::calculateCollisionConstraint(
	Cable& cable, CableCollisionConstraint& constraint, double deltaTime) {
	auto& particle = cable.particles[constraint.particleIndex];
	const float c = glm::dot(particle.position - constraint.point, constraint.normal) - constraint.particleRadius;
	if (c >= 0.0f) {
		return glm::vec3(0.0f);
	}
	if (particle.inverseMass == 0.0f) {
		return glm::vec3(0.0f);
	}
	const glm::vec3 grad = constraint.normal;
	const float w = particle.inverseMass;
	const float alpha = static_cast<float>(constraint.compliance / (deltaTime * deltaTime));
	const float denominator = w * glm::dot(grad, grad) + alpha;

	if (denominator <= 0.0f) {
		return glm::vec3(0.0f);
	}

	float deltaLambda = (-c - alpha * constraint.lambda) / denominator;
	const float oldLambda = constraint.lambda;

	// lambda must stay >= 0, because contact can push but cannot pull.
	constraint.lambda = std::max(0.0f, constraint.lambda + deltaLambda);
	deltaLambda = constraint.lambda - oldLambda;

	return deltaLambda * grad;
}

glm::vec3 CableSystem::calculateDistanceConstraint(
	Cable& cable, CableDistanceConstraint& constraint, double deltaTime) {
	auto& particleA = cable.particles[constraint.indexParticleA];
	auto& particleB = cable.particles[constraint.indexParticleB];

	const float currentDist = glm::length(particleB.position - particleA.position);
	if (currentDist < 0.0001f) {
		return glm::vec3(0, 0, 0);
	}
	const float c = currentDist - constraint.restDistance;
	glm::vec3 normal = glm::normalize(particleB.position - particleA.position);
	const float alpha = constraint.compliance / (deltaTime * deltaTime);

	const float inverseMassSum = particleA.inverseMass + particleB.inverseMass;
	if (inverseMassSum == 0.0f) {
		return glm::vec3(0, 0, 0);
	}

	const float deltaLambda = (-c - alpha * constraint.lambda) / (inverseMassSum + alpha);
	constraint.lambda += deltaLambda;

	const glm::vec3 correction = deltaLambda * normal;
	return -correction; // now with "-" sign when the cable is too long, A moves toward B, and B moves toward A.
}

void CableSystem::applyExternalForces(CableParticle& particle, double deltaTime) {
	if (particle.inverseMass == 0.0f) {
		particle.linearVelocity = glm::vec3(0.0f);
		return;
	}
	const glm::vec3 gravityAccel(0.0f, -static_cast<float>(m_gravity), 0.0f);

	particle.linearVelocity += gravityAccel * static_cast<float>(deltaTime);
	particle.position += particle.linearVelocity * static_cast<float>(deltaTime);
}

CableBounds CableSystem::computeCableBounds(const Cable& cable) {
	if (cable.particles.empty()) {
		return CableBounds{.position = glm::vec3(0.0f), .size = glm::vec3(0.0f)};
	}
	glm::vec3 min(std::numeric_limits<float>::infinity());
	glm::vec3 max(-std::numeric_limits<float>::infinity());

	for (const CableParticle& particle : cable.particles) {
		auto& pos = particle.position;

		for (const CableParticle& particle : cable.particles) {
			const glm::vec3& pos = particle.position;

			min.x = std::min(min.x, pos.x);
			min.y = std::min(min.y, pos.y);
			min.z = std::min(min.z, pos.z);

			max.x = std::max(max.x, pos.x);
			max.y = std::max(max.y, pos.y);
			max.z = std::max(max.z, pos.z);
		}
	}

	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 size = max - min;

	return CableBounds{.position = center, .size = size};
}
