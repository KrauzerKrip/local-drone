#pragma once

#include <entt/entt.hpp>

#include "components.h"
#include "game/control/mouse_raycast_observer.h"
#include "game/control/action_control.h"
#include "mouse_raycast.h"
#include "game/machine/components.h"
#include "lc_client/eng_graphics/entt/components.h"
#include "game/machine/machine_connector.h"


class DroneControlSystem : public MouseRaycastObserver {
public:
	DroneControlSystem(MouseRaycast* pMouseRaycast, ActionControl* pActionControl, entt::registry* pRegistry);

	void input(double deltaTime);
	void update();
	void onAction(std::string action, entt::entity entity, glm::vec3 position, float distance) override;
	void onMouseMove(entt::entity entity, glm::vec3 position, float distance) override;

private:
	MouseRaycast* m_pMouseRaycast = nullptr;
	ActionControl* m_pActionControl = nullptr;

	entt::registry* m_pRegistry = nullptr;
};
