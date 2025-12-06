#include "physics_loading_system.h"

#include "lc_client/exceptions/physics_exception.h"
#include "lc_client/tier0/log.h"


PhysicsLoadingSystem::PhysicsLoadingSystem(PhysicsLoader* pPhysicsLoader, entt::registry* pRegistry, IConsole* pConsole) {
	m_pRegistry = pRegistry;
	m_pPhysicsLoader = pPhysicsLoader;
	m_pConsole = pConsole;
}

void PhysicsLoadingSystem::update() {
	auto physicsRequest = m_pRegistry->view<PhysicsRequest>();

	for (auto&& [entity, request] : physicsRequest.each()) {
		try {
			m_pPhysicsLoader->loadPhysics(entity, request.filePath);
		}
		catch (PhysicsLoadingException& exception) {
			try {
				std::rethrow_if_nested(exception);
			}
			catch (const std::exception& nestedException) {
				LE_CORE_ERROR(std::string(exception.what()) + "\nReason: " + nestedException.what());
			}
		}
		m_pRegistry->remove<PhysicsRequest>(entity);
	}
}
