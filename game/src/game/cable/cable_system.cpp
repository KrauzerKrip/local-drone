#include "cable_system.h"
#include <entt/entity/fwd.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include "lc_client/exceptions/component_exception.h"

CableSystem::CableSystem(entt::registry* pRegistry) {
	m_pRegistry = pRegistry;
	m_gravity = 9.81;
}

void CableSystem::update(double updateInterval) {
	const int substeps = 20;
	const int solverIterations = 5;

	for (auto&& [entity, cable] : m_pRegistry->view<Cable>().each()) {
		for (int substep = 0; substep < substeps; substep++) {
			const float deltaTime = updateInterval / substeps;

			for (CableParticle& particle : cable.particles) {
				particle.prevPosition = particle.position;
			}

			for (CableParticle& particle : cable.particles) {
				this->applyExternalForces(particle, deltaTime);
			}

			// @TODO collide constraints
			//
			for (CableConstraint& constraint : cable.constraints) {
				constraint.lambda = 0.0f;
			}

			for (CableCollisionConstraint& contact : contacts) {
				contact.lambda = 0.0f;
			}

			for (int iter = 0; iter < solverIterations; iter++) {
				for (CableConstraint& constraint : cable.constraints) {
					glm::vec3 correction = this->calculateDistanceConstraint(cable, constraint, deltaTime);
					auto& particleA = cable.particles[constraint.indexA];
					auto& particleB = cable.particles[constraint.indexB];
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

glm::vec3 CableSystem::calculateDistanceConstraint(Cable& cable, CableConstraint& constraint, double deltaTime) {
	auto& particleA = cable.particles[constraint.indexA];
	auto& particleB = cable.particles[constraint.indexB];

	float currentDist = glm::length(particleB.position - particleA.position);
	if (currentDist < 0.0001f) {
		return glm::vec3(0, 0, 0);
	}
	float c = currentDist - constraint.restDistance;
	glm::vec3 normal = glm::normalize(particleB.position - particleA.position);
	float alpha = constraint.compliance / (deltaTime * deltaTime);

	float inverseMassSum = particleA.inverseMass + particleB.inverseMass;
	if (inverseMassSum == 0.0f) {
		return glm::vec3(0, 0, 0);
	}

	float deltaLambda = (-c - alpha * constraint.lambda) / (inverseMassSum + alpha);
	constraint.lambda += deltaLambda;

	glm::vec3 correction = deltaLambda * normal;
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
