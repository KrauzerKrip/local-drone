#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "lc_client/eng_graphics/camera/camera.h"
#include "gl_mesh_render.h"
#include "gl_outline_render.h"


class TransparentRenderGl {
public:
	TransparentRenderGl(Camera* pCamera, MeshRenderGl* pMeshRenderGl, OutlineRenderGl* pOutlineRender, entt::registry* pRegistry);

	void render(const glm::mat4& projection, const glm::mat4& view);

private:
	entt::registry* m_pRegistry = nullptr;
	entt::registry* m_pUtilRegistry = nullptr;

	Camera* m_pCamera = nullptr;
	MeshRenderGl* m_pMeshRender = nullptr;
	OutlineRenderGl* m_pOutlineRender = nullptr;
};