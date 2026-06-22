#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include "components.h"
#include "lc_client/eng_physics/physics.h"

struct CableBounds {
	glm::vec3 position;
	glm::vec3 size;
};

class CableSystem {
public:
	CableSystem(Physics* pPhysics, entt::registry* pRegistry);

	void update(double updateInterval);

private:
	void connectWithCable(entt::entity anchor, entt::entity attachment);
	glm::vec3 calculateCollisionConstraint(Cable& cable, CableCollisionConstraint& constraint, double deltaTime);
	glm::vec3 calculateDistanceConstraint(Cable& cable, CableDistanceConstraint& constraint, double deltaTime);
	void applyExternalForces(CableParticle& particle, double deltaTime);
	CableBounds computeCableBounds(const Cable& cable);

	Physics* m_pPhysics = nullptr;
	entt::registry* m_pRegistry = nullptr;
	float m_gravity;
};
