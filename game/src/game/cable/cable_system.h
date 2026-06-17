#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include "components.h"

class CableSystem {
public:
	CableSystem(entt::registry* pRegistry);

	void update(double updateInterval);

private:
	void connectWithCable(entt::entity anchor, entt::entity attachment);
	glm::vec3 calculateDistanceConstraint(Cable& cable, CableConstraint& constraint, double deltaTime);
	void applyExternalForces(CableParticle& particle, double deltaTime);

	entt::registry* m_pRegistry = nullptr;
	float m_gravity;
};
