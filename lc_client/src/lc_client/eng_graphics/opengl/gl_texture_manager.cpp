#include "gl_texture_manager.h"

#include <iostream>

#include "lc_client/eng_graphics/opengl/gl_texture.h"
#include "lc_client/tier0/log.h"


TextureManagerGl::TextureManagerGl(eng::IResource* pResource) : TextureManager(pResource){}

Texture* TextureManagerGl::loadTexture(std::string path) {
	eng::Image* pImage;

	const std::vector<unsigned char>& buffer = m_pResource->getFileResource(path + FILE_FORMAT);

	pImage = new eng::Image(buffer);

	Texture* pTexture = new TextureGL(pImage); // TODO

	LE_CORE_DEBUG("texture {} loaded", path);

	return pTexture;
}
