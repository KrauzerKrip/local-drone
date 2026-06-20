#include "cable_system.h"
#include <entt/entity/fwd.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <vector>
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
		for (int substep = 0; substep < substeps; substep++) {
			const float deltaTime = updateInterval / substeps;

			for (size_t i = 0; i < cable.particles.size(); i++) {
				auto& particle = cable.particles[i];
				particle.prevPosition = particle.position;
				this->applyExternalForces(particle, deltaTime);
				SphereOverlapQuery query = {.center = particle.position, .radius = 0.01};
				m_pPhysics->querySphereOverlaps(query, hits);

				for (const CollisionHit& hit : hits) {
					cable.collisionConstraints.push_back({.particleIndex = i,
						.normal = hit.normal,
						.c = -hit.penetrationDepth,
						.lambda = 0.0f,
						.compliance = 0.0f});
				}
			}

			for (CableDistanceConstraint& constraint : cable.constraints) {
				constraint.lambda = 0.0f;
			}

			for (int iter = 0; iter < solverIterations; iter++) {
				for (CableCollisionConstraint& constraint : cable.collisionConstraints) {
					glm::vec3 correction = this->calculateCollisionConstraint(cable, constraint, deltaTime);
					CableParticle& particle = cable.particles[constraint.particleIndex];

					particle.position += particle.inverseMass * correction;
				}
				for (CableDistanceConstraint& constraint : cable.constraints) {
					glm::vec3 correction = this->calculateDistanceConstraint(cable, constraint, deltaTime);
					auto& particleA = cable.particles[constraint.indexParticleA];
					auto& particleB = cable.particles[constraint.indexParticleB];
					particleA.position += particleA.inverseMass * correction;

					particleB.position -= particleB.inverseMass * correction;
				}
			}

			for (CableParticle& particle : cable.particles) {
				particle.linearVelocity = (particle.position - particle.prevPosition) / deltaTime;
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
	return correction;
}

void CableSystem::applyExternalForces(CableParticle& particle, double deltaTime) {
	if (particle.inverseMass == 0.0f) {
		particle.linearVelocity = glm::vec3(0.0f);
		return;
	}
	particle.linearVelocity = glm::vec3(0, -m_gravity * deltaTime, 0);
	particle.position += particle.linearVelocity * static_cast<float>(deltaTime);
}
