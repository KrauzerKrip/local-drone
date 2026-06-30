#include "drone_control_system.h"

#include "game/drone/components.h"
#include "lc_client/eng_scene/entt/components.h"
#include <glm/geometric.hpp>

DroneControlSystem::DroneControlSystem(
	MouseRaycast* pMouseRaycast, ActionControl* pActionControl, entt::registry* pRegistry) {
	this->m_pActionControl = pActionControl;
	this->m_pMouseRaycast = pMouseRaycast;
	this->m_pRegistry = pRegistry;
}

void DroneControlSystem::input(double deltaTime) {
	auto drones = m_pRegistry->view<Drone, Transform>();

	for (auto&& [entity, drone, transform] : drones.each()) {
		if (m_pActionControl->isAction("kb_drone_forward")) {
			glm::vec3 localDir = glm::vec3(0.0f, 0.0f, 1.0f);
			glm::vec3 dir = glm::normalize(transform.rotation * localDir);
			transform.position += dir * drone.speed * static_cast<float>(deltaTime);
		}
		if (m_pActionControl->isAction("kb_drone_back")) {
			glm::vec3 localDir = glm::vec3(0.0f, 0.0f, -1.0f);
			glm::vec3 dir = glm::normalize(transform.rotation * localDir);
			transform.position += dir * drone.speed * static_cast<float>(deltaTime);
		}
		if (m_pActionControl->isAction("kb_drone_left")) {
			glm::vec3 localDir = glm::vec3(-1.0f, 0.0f, 0.0f);
			glm::vec3 dir = glm::normalize(transform.rotation * localDir);
			transform.position += dir * drone.speed * static_cast<float>(deltaTime);
		}
		if (m_pActionControl->isAction("kb_drone_right")) {
			glm::vec3 localDir = glm::vec3(1.0f, 0.0f, 0.0f);
			glm::vec3 dir = glm::normalize(transform.rotation * localDir);
			transform.position += dir * drone.speed * static_cast<float>(deltaTime);
		}
		if (m_pActionControl->isAction("kb_drone_up")) {
			glm::vec3 localDir = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 dir = glm::normalize(transform.rotation * localDir);
			transform.position += dir * drone.speed * static_cast<float>(deltaTime);
		}
		if (m_pActionControl->isAction("kb_drone_down")) {
			glm::vec3 localDir = glm::vec3(0.0f, -1.0f, 0.0f);
			glm::vec3 dir = glm::normalize(transform.rotation * localDir);
			transform.position += dir * drone.speed * static_cast<float>(deltaTime);
		}
	}
}
void DroneControlSystem::update() {}
void DroneControlSystem::onAction(std::string action, entt::entity entity, glm::vec3 position, float distance) {}
void DroneControlSystem::onMouseMove(entt::entity entity, glm::vec3 position, float distance) {}
