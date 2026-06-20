#include "cable_system.h"
#include <entt/entity/fwd.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <vector>
#include "lc_client/eng_graphics/entt/components.h"
#include "lc_client/eng_physics/entt/components.h"
#include "lc_client/eng_physics/physics.h"
#include "lc_client/exceptions/component_exception.h"

CableSystem::CableSystem(Physics* pPhysics, entt::registry* pRegistry) {
	m_pPhysics = pPhysics;
	m_pRegistry = pRegistry;
	m_gravity = 9.81;
}

void CableSystem::update(double updateInterval) {
	const int substeps = 20;
	const int solverIterations = 5;

	std::vector<CollisionHit> hits;
	hits.reserve(8);

	for (auto&& [entity, cable] : m_pRegistry->view<Cable>().each()) {
		std::vector<bool> particleHadCollision(cable.particles.size(), false);
		std::vector<glm::vec3> particleCollisionNormal(cable.particles.size(), glm::vec3(0.0f));
		for (int substep = 0; substep < substeps; substep++) {
			const float deltaTime = updateInterval / substeps;
			cable.collisionConstraints.clear();

			for (size_t i = 0; i < cable.particles.size(); i++) {
				auto& particle = cable.particles[i];
				particle.prevPosition = particle.position;
				this->applyExternalForces(particle, deltaTime);
				hits.clear();
				SphereOverlapQuery query = {.center = particle.position, .radius = 0.01};
				m_pPhysics->querySphereOverlaps(query, hits);

				for (const CollisionHit& hit : hits) {
					cable.collisionConstraints.push_back({.particleIndex = i,
						.normal = hit.normal,
						.c = -hit.penetrationDepth,
						.lambda = 0.0f,
						.compliance = 0.0f});


					particleHadCollision[i] = true;
					particleCollisionNormal[i] += hit.normal;
				}
			}

			for (CableDistanceConstraint& constraint : cable.constraints) {
				constraint.lambda = 0.0f;
			}

			for (int iter = 0; iter < solverIterations; iter++) {
				for (CableDistanceConstraint& constraint : cable.constraints) {
					glm::vec3 correction = this->calculateDistanceConstraint(cable, constraint, deltaTime);
					auto& particleA = cable.particles[constraint.indexParticleA];
					auto& particleB = cable.particles[constraint.indexParticleB];
					particleA.position += particleA.inverseMass * correction;

					particleB.position -= particleB.inverseMass * correction;
				}
				for (CableCollisionConstraint& constraint : cable.collisionConstraints) {
					glm::vec3 correction = this->calculateCollisionConstraint(cable, constraint, deltaTime);
					CableParticle& particle = cable.particles[constraint.particleIndex];
					particle.position += particle.inverseMass * correction;
				}
			}

			for (size_t i = 0; i < cable.particles.size(); i++) {
				CableParticle& particle = cable.particles[i];

				particle.linearVelocity = (particle.position - particle.prevPosition) / deltaTime;

				if (particleHadCollision[i]) {
					glm::vec3 n = glm::normalize(particleCollisionNormal[i]);

					float vn = glm::dot(particle.linearVelocity, n) * 0.95;

					// Remove all normal velocity from contact.
					// This kills both fake upward bounce and velocity into the collider.
					particle.linearVelocity -= vn * n;

					// Optional damping/friction
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
	if (constraint.c >= 0.0f) {
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

	float deltaLambda = (-constraint.c - alpha * constraint.lambda) / denominator;
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
