#include "texture_manager.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <memory>

#include "lc_client/tier0/log.h"
#include "lc_client/util/image.h"
#include "lc_client/exceptions/io_exceptions.h"
#include "lc_client/tier0/tier0.h"


TextureManager::TextureManager(eng::IResource* pResource) {
	m_pResource = pResource;
}

Texture* TextureManager::getTexture(std::string path) {
	try {
		return m_textureMap.at(path);
	}
	catch (std::out_of_range) {
		LE_CORE_DEBUG("texture '{}' not found in cache, loading now...", path);
		Texture* pTexture = nullptr;

		try {
			pTexture = loadTexture(path);
			m_textureMap.emplace(path, pTexture);
		}
		catch (ResourceFileNotFoundException& exception) {
			LE_CORE_WARN(exception.what());
			Tier0::getIConsole()->warn(exception.what());
			pTexture = m_textureMap.at("no_css?");
		}
		catch (FileTooLargeException) {
			throw FileTooLargeException("Image is too large to load it: " + path);
		}
		catch (ImageLoadFailureException& exception) {
			LE_CORE_WARN("failed to load texture '{}': {}", path, exception.what());
			pTexture = m_textureMap.at("no_css?");
		}

		if (pTexture == nullptr) {
			std::string str = "TextureManagerGL: pTexture is nullptr. Path given: " + path;
			assert(str.c_str());
		}

		return pTexture;

	}
}

void TextureManager::reload() {
	Texture* pNoCssTexture = loadTexture("dev/textures/eng_texture_not_found/color");
	m_textureMap.emplace(std::string("no_css?"), pNoCssTexture);

	for (auto [path, pTexture] : m_textureMap) {
		if (path == "no_css?") {
			continue;
		}

		try {
			delete pTexture;
			pTexture = nullptr;
			pTexture = loadTexture(path);
			m_textureMap[path] = pTexture;
		}
		catch (ResourceFileNotFoundException& exception) {
			LE_CORE_WARN(exception.what());
			pTexture = m_textureMap.at("no_css?");
		}
		catch (FileTooLargeException) {
			LE_CORE_WARN("image is too large to load it: {}", path);
		}
		catch (ImageLoadFailureException& exception) {
			LE_CORE_WARN("failed to load texture {}: {}", path, exception.what());
			pTexture = m_textureMap.at("no_css?");
		}

		if (pTexture == nullptr) {
		    LE_CORE_WARN("pTexture is nullptt. Path given {}", path);
			assert(false && "pTexture is nullptt");
		}
	}
}


const std::string TextureManager::FILE_FORMAT = ".png";
