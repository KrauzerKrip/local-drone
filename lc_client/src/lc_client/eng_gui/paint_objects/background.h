#pragma once

#include "lc_client/eng_gui/widgets/layer.h"
#include "lc_client/eng_gui/widgets/rectangle.h"
#include <vector>

class Background {
public:
	virtual ~Background() = default;

	virtual void setStencil(Rectangle& rectangle) = 0;
	virtual void render(const Rectangle& rectangle, const Layer& layer) = 0;
	virtual void setStencils(std::vector<Rectangle> rectangles) = 0;
};
