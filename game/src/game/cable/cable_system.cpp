#include "cable_system.h"
#include <algorithm>
#include <entt/entity/fwd.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <vector>
#include "lc_client/eng_graphics/entt/components.h"
#include "lc_client/eng_physics/collision/aabb.h"
#include "lc_client/eng_physics/collision/sphere.h"
#include "lc_client/eng_physics/entt/components.h"
#include "lc_client/eng_physics/physics.h"
#include "lc_client/eng_scene/entt/components.h"
#include "lc_client/exceptions/component_exception.h"
#include <limits>

#if defined(TRACY_ENABLE)
#include <tracy/Tracy.hpp>
#else
#define ZoneScopedN(name)
#endif

namespace {
	bool containsCandidate(const std::vector<CableColliderCandidate>& candidates, entt::entity colliderEntity) {
		return std::any_of(
			candidates.begin(), candidates.end(), [colliderEntity](const CableColliderCandidate& candidate) {
				return candidate.colliderEntity == colliderEntity;
			});
	}

	std::vector<std::vector<size_t>> colorConstraints(const Cable& cable) {
		std::vector<std::vector<size_t>> colors;
		std::vector<std::vector<bool>> colorUsesParticle;

		for (size_t constraintIndex = 0; constraintIndex < cable.constraints.size(); constraintIndex++) {
			const CableDistanceConstraint& constraint = cable.constraints[constraintIndex];
			size_t colorIndex = 0;

			for (; colorIndex < colors.size(); colorIndex++) {
				if (!colorUsesParticle[colorIndex][constraint.indexParticleA] &&
					!colorUsesParticle[colorIndex][constraint.indexParticleB]) {
					break;
				}
			}

			if (colorIndex == colors.size()) {
				colors.emplace_back();
				colorUsesParticle.emplace_back(cable.particles.size(), false);
			}

			colors[colorIndex].push_back(constraintIndex);
			colorUsesParticle[colorIndex][constraint.indexParticleA] = true;
			colorUsesParticle[colorIndex][constraint.indexParticleB] = true;
		}

		return colors;
	}

	void updateColoredConstraints(Cable& cable) {
		if (!cable.coloredConstraintsDirty && cable.coloredConstraintCount == cable.constraints.size()) {
			return;
		}

		cable.coloredConstraints = colorConstraints(cable);
		cable.coloredConstraintCount = cable.constraints.size();
		cable.coloredConstraintsDirty = false;
	}
}

CableSystem::CableSystem(Physics* pPhysics, entt::registry* pRegistry) {
	m_pPhysics = pPhysics;
	m_pRegistry = pRegistry;
	m_gravity = 9.81;
}

void CableSystem::update(double updateInterval) {
	ZoneScopedN("CableSystem::update");

	const int substeps = 10;
	const int solverIterations = 3;

	std::vector<CollisionHit> cableColliderHits;
	cableColliderHits.reserve(16);
	constexpr float particleRadius = 0.01f;

	for (auto&& [entity, cable] : m_pRegistry->view<Cable>().each()) {
		ZoneScopedN("CableSystem::update cable");

		updateColoredConstraints(cable);

		// std::cout << "particles: " << cable.particles.size() << std::endl;
		// std::cout << "constraints: " << cable.constraints.size() << std::endl;
		// std::cout << "coloredConstraints: " << cable.coloredConstraints.size() << std::endl;
		// std::cout << "collisionConstraints: " << cable.collisionConstraints.size() << std::endl;
		// std::cout << "colliderCandidates: " << cable.colliderCandidates.size() << std::endl;

		cable.particleHadCollisionScratch.resize(cable.particles.size());
		cable.particleCollisionNormalScratch.resize(cable.particles.size());
		auto& particleHadCollision = cable.particleHadCollisionScratch;
		auto& particleCollisionNormal = cable.particleCollisionNormalScratch;
		for (int substep = 0; substep < substeps; substep++) {
			ZoneScopedN("CableSystem::substep");

			const float deltaTime = updateInterval / substeps;
			std::fill(particleHadCollision.begin(), particleHadCollision.end(), false);
			std::fill(particleCollisionNormal.begin(), particleCollisionNormal.end(), glm::vec3(0.0f));

			{
				ZoneScopedN("CableSystem::apply external forces");

				for (CableParticle& particle : cable.particles) {
					particle.prevPosition = particle.position;
					if (particle.inverseMass == 0.0f) {
						continue;
					}
					this->applyExternalForces(particle, deltaTime);
					this->applyAttachment(particle, deltaTime);
				}
			}

			{
				ZoneScopedN("CableSystem::reset distance lambdas");

				for (CableDistanceConstraint& constraint : cable.constraints) {
					constraint.lambda = 0.0f;
				}
			}

			const float collisionMargin = 0.25f;
			CableBounds cableBounds;
			{
				ZoneScopedN("CableSystem::compute bounds");

				cableBounds = computeCableBounds(cable);
			}
			cableBounds.size += glm::vec3(particleRadius + collisionMargin);

			{
				ZoneScopedN("CableSystem::physics AABB overlap query");

				cableColliderHits.clear();
				m_pPhysics->queryAABBOverlaps(
					AABBOverlapQuery{.position = cableBounds.position, .size = cableBounds.size}, cableColliderHits);
			}
			{
				ZoneScopedN("CableSystem::map collider candidates");
				cable.colliderCandidates.clear();
				cable.colliderCandidates.reserve(cableColliderHits.size());

				for (const CollisionHit& hit : cableColliderHits) {
					if (containsCandidate(cable.colliderCandidates, hit.colliderEntity)) {
						continue;
					}

					const Transform& colliderTransform = m_pRegistry->get<Transform>(hit.colliderEntity);

					cable.colliderCandidates.push_back(CableColliderCandidate{.ownerEntity = hit.ownerEntity,
						.colliderEntity = hit.colliderEntity,
						.colliderType = hit.colliderType,
						.colliderTransform = colliderTransform});
				}
			}

			{
				ZoneScopedN("CableSystem::finding particle collision candidates");

				const float padding = particleRadius + collisionMargin;
				cable.particleColliderCandidateOffsets.resize(cable.particles.size() + 1);
				cable.particleColliderCandidateIndices.clear();

				for (size_t i = 0; i < cable.particles.size(); i++) {
					const CableParticle& particle = cable.particles[i];
					cable.particleColliderCandidateOffsets[i] = cable.particleColliderCandidateIndices.size();

					const glm::vec3 minPos = glm::min(particle.prevPosition, particle.position);
					const glm::vec3 maxPos = glm::max(particle.prevPosition, particle.position);

					const glm::vec3 center = (minPos + maxPos) * 0.5f;
					const glm::vec3 fullSize = (maxPos - minPos) + glm::vec3(padding * 2.0f);

					AABBOverlapQuery query{.position = center, .size = fullSize};

					for (size_t candidateIndex = 0; candidateIndex < cable.colliderCandidates.size();
						candidateIndex++) {
						const CableColliderCandidate& candidate = cable.colliderCandidates[candidateIndex];
						CollisionHit hit;
						if (m_pPhysics->AABBIntersectCollider(
								query, candidate.colliderType, candidate.colliderTransform, hit)) {
							cable.particleColliderCandidateIndices.push_back(candidateIndex);
						}
					}
				}

				cable.particleColliderCandidateOffsets[cable.particles.size()] =
					cable.particleColliderCandidateIndices.size();
			}

			for (int iter = 0; iter < solverIterations; iter++) {
				ZoneScopedN("CableSystem::solver iteration");

				{
					ZoneScopedN("CableSystem::solve distance constraints");

					for (const std::vector<size_t>& batch : cable.coloredConstraints) {
						for (size_t constraintIndex : batch) {
							CableDistanceConstraint& constraint = cable.constraints[constraintIndex];
							glm::vec3 correction = this->calculateDistanceConstraint(cable, constraint, deltaTime);
							auto& particleA = cable.particles[constraint.indexParticleA];
							auto& particleB = cable.particles[constraint.indexParticleB];
							particleA.position += particleA.inverseMass * correction;
							particleB.position -= particleB.inverseMass * correction;
						}
					}
				}

				{
					ZoneScopedN("CableSystem::build collision constraints");

					cable.collisionConstraints.clear();

					for (size_t i = 0; i < cable.particles.size(); i++) {
						auto& particle = cable.particles[i];
						Sphere sphere(particle.position, particleRadius);
						const size_t beginCandidate = cable.particleColliderCandidateOffsets[i];
						const size_t endCandidate = cable.particleColliderCandidateOffsets[i + 1];
						for (size_t candidateOffset = beginCandidate; candidateOffset < endCandidate;
							candidateOffset++) {
							const size_t candidateIndex = cable.particleColliderCandidateIndices[candidateOffset];
							const CableColliderCandidate& particleCandidate = cable.colliderCandidates[candidateIndex];
							CollisionHit hit;
							bool result = m_pPhysics->sphereIntersectCollider(
								sphere, particleCandidate.colliderType, particleCandidate.colliderTransform, hit);
							if (result) {
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
				}

				{
					ZoneScopedN("CableSystem::solve collision constraints");

					for (CableCollisionConstraint& constraint : cable.collisionConstraints) {
						glm::vec3 correction = this->calculateCollisionConstraint(cable, constraint, deltaTime);
						CableParticle& particle = cable.particles[constraint.particleIndex];
						particle.position += particle.inverseMass * correction;
					}
				}
			}

			{
				ZoneScopedN("CableSystem::update velocities");

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
		}

		{
			ZoneScopedN("CableSystem::sync primitive lines");

			if (m_pRegistry->any_of<PrimitiveLines>(entity)) {
				auto& lines = m_pRegistry->get<PrimitiveLines>(entity);
				const size_t requiredLineCount = cable.particles.size() > 0 ? cable.particles.size() - 1 : 0;
				lines.lines.resize(
					requiredLineCount, PrimitiveLine(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0, 0, 1)));
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
}

void CableSystem::connectWithCable(entt::entity anchor, entt::entity attachment) {}

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

void CableSystem::applyAttachment(CableParticle& particle, double deltaTime) {
	if (!particle.attachment.has_value()) {
		return;
	}

	const auto attachment = *particle.attachment;

	Transform& transform = m_pRegistry->get<Transform>(attachment.anchor);
	glm::vec3 anchorWorldPos = transform.position + attachment.offset;
	particle.position = anchorWorldPos;
	particle.linearVelocity = (particle.position - particle.prevPosition) / static_cast<float>(deltaTime);
}

CableBounds CableSystem::computeCableBounds(const Cable& cable) {
	if (cable.particles.empty()) {
		return CableBounds{.position = glm::vec3(0.0f), .size = glm::vec3(0.0f)};
	}
	glm::vec3 min(std::numeric_limits<float>::infinity());
	glm::vec3 max(-std::numeric_limits<float>::infinity());

	for (const CableParticle& particle : cable.particles) {
		const glm::vec3& pos = particle.position;

		min.x = std::min(min.x, pos.x);
		min.y = std::min(min.y, pos.y);
		min.z = std::min(min.z, pos.z);

		max.x = std::max(max.x, pos.x);
		max.y = std::max(max.y, pos.y);
		max.z = std::max(max.z, pos.z);
	}

	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 size = max - min;

	return CableBounds{.position = center, .size = size};
}
