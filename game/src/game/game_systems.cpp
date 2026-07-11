#include "game_systems.h"


GameSystems::GameSystems(World* pWorld, eng::IResource* pResource, PhysicalConstants* pPhysicalConstants,
	IConsole* pConsole, Parameters* pParameters)
	: m_characterSystem(pWorld->getRegistry()),
	  m_machineSystem(pResource, pWorld->getRegistry(), pPhysicalConstants, pParameters),
	  m_agricultureSystem(pWorld->getRegistry()),
	  m_inventorySystem(pWorld->getRegistry()),
	  m_tradeSystem(pWorld->getRegistry()),
	  m_depositSystem(pWorld->getRegistry(), pConsole),
	  m_cableSystem(pWorld),
	  m_spoolSystem(pWorld) {}

void GameSystems::input(double deltaTime) {
	m_characterSystem.input();
	m_machineSystem.input(deltaTime);
}

void GameSystems::update(double updateInterval) {
	m_characterSystem.update(updateInterval);
	m_machineSystem.update(updateInterval);
	m_agricultureSystem.update(updateInterval);
	m_tradeSystem.update();
	m_depositSystem.update(updateInterval);
	m_inventorySystem.update();
	m_cableSystem.update(updateInterval);
	m_spoolSystem.update(updateInterval);
}

void GameSystems::machineUpdate(double updateInterval) {
	m_agricultureSystem.machineUpdate(updateInterval);
	m_machineSystem.machineUpdate(updateInterval);
}

void GameSystems::frame(double deltaTime) { m_machineSystem.frame(deltaTime); }
