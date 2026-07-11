#include "model_manager.h"
#include "model_manager.h"

#include <iostream>
#include <stdexcept>
#include <memory>

#include "entt/components.h"
#include "lc_client/exceptions/io_exceptions.h"
#include "lc_client/eng_model/model_loading.h"
#include "lc_client/eng_graphics/entt/components.h"
#include "lc_client/tier0/log.h"



ModelManager::ModelManager(
	TextureManager* pTextureManager, eng::IResource* pResource, entt::registry* pUtilRegistry, IConsole* pConsole) {
	m_pTextureManager = pTextureManager;
	m_pResource = pResource;
	m_pUtilRegistry = pUtilRegistry;
	m_pConsole = pConsole;
}

Model* ModelManager::getModel(
	const std::string modelPath, const std::string texturesDirPath, const std::string materialType) {
	try {
		return m_modelMap.at(modelPath);
	}
	catch (std::out_of_range) {
		LE_CORE_DEBUG("model '{}' not found in cache, loading...", modelPath);
		return loadModel(modelPath, texturesDirPath, materialType);
	}
}

Model* ModelManager::loadModel(
	const std::string modelPath, const std::string texturesDirPath, const std::string materialType) {
	Model* pModel = nullptr;

	bool success = false;

	try {
		eng::ModelLoading modelLoading(modelPath, FILE_FORMAT, m_pResource, m_pUtilRegistry);
		pModel = modelLoading.loadModel();

		success = true;
	}
	catch (ResourceFileNotFoundException& exception) {
		LE_CORE_WARN("Resource not found for model {}: {}", modelPath, exception.what());

		// "gmod vibe" here just to occur exception and load black-purple textures
		eng::ModelLoading modelLoading(ERROR_MODEL_PATH, FILE_FORMAT, m_pResource, m_pUtilRegistry);
		pModel = modelLoading.loadModel();
	}
	catch (AssimpException& exception) {
		LE_CORE_WARN("Failed to load model {}: {}", modelPath, exception.what());

		// "gmod vibe" here just to occur exception and load black-purple textures
		eng::ModelLoading modelLoading(ERROR_MODEL_PATH, FILE_FORMAT, m_pResource, m_pUtilRegistry);

		pModel = modelLoading.loadModel();
	}

	if (pModel == nullptr) {
		throw std::runtime_error(
			"ModelManager: pModel of  " + modelPath + "  OR  " + ERROR_MODEL_PATH + "  is nullptr.");
	}

	m_modelMap.emplace(modelPath, pModel);

	if (success) {
		LE_CORE_DEBUG("Model '{}' loaded", modelPath);
	}
	else {
		LE_CORE_WARN("Model '" + modelPath + " wasn`t loaded successfully. Set default instead.");
	}

	pModel->materialDir = texturesDirPath;
	pModel->materialType = materialType;

	return pModel;
}

const std::string ModelManager::FILE_FORMAT = ".";
const std::string ModelManager::ERROR_MODEL_PATH = "dev/models/eng_model_not_found/model.obj";
