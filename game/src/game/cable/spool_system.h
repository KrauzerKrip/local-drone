#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include "components.h"
#include "lc_client/eng_physics/physics.h"
#include "lc_client/eng_world/world.h"

class SpoolSystem {
public:
	SpoolSystem(World* pWorld);

	void update(double updateInterval);

private:
	Physics* m_pPhysics = nullptr;
	entt::registry* m_pRegistry = nullptr;
};
