#include "image_background.h"

#include <algorithm>

ImageBackground::ImageBackground(std::string path, const GuiDependencies& dependencies) : m_dependencies(dependencies) {
	m_path = path;
	m_pBackgroundRender = dependencies.pBackgroundRender;
	m_hasStencil = false;
}

void ImageBackground::setStencil(Rectangle& rectangle) {
	m_hasStencil = true;
	m_stencil = rectangle;
}

void ImageBackground::setStencils(std::vector<Rectangle> stencils) { m_stencils = stencils; }

void ImageBackground::render(const Rectangle& rectangle, const Layer& layer) {
	if (m_hasStencil) {
		std::vector<RectangleVertices> vertices(m_stencils.size());
		std::transform(
			m_stencils.begin(), m_stencils.end(), vertices.begin(), [](Rectangle rect) { return rect.getVertices(); });

		m_pBackgroundRender->renderImageStencils(
			ImageQuad({m_path, rectangle.getVertices(),
				m_dependencies.pWidgetZOffsetCalculator->calculateZOffset(layer.number)}),
			vertices);
	}
	else {
		m_pBackgroundRender->renderImage(ImageQuad({m_path, rectangle.getVertices(),
			m_dependencies.pWidgetZOffsetCalculator->calculateZOffset(layer.number)}));
	}
}
