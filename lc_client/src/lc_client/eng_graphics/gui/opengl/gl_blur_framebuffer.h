#pragma once

#include "lc_client/eng_graphics/opengl/gl_framebuffer.h"


class BlurFramebuffer : public Framebuffer {
public:
	BlurFramebuffer(int width, int height);
};