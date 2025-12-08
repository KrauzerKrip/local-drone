#pragma once

#include <entt/entt.hpp>

#include "tree_system.h"


class AgricultureSystem {
public:
	AgricultureSystem(entt::registry* pRegistry);

	void update(double updateInterval);
	void machineUpdate(double updateInterval);

private:
	TreeSystem m_treeSystem;

	entt::registry* m_pRegistry = nullptr;
};