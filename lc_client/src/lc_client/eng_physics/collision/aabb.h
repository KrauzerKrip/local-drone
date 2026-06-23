#pragma once

#include "lc_client/eng_physics/entt/components.h"
#include "lc_client/eng_scene/entt/components.h"

struct AABBCollisionHit {
	glm::vec3 point;
	glm::vec3 normal;
	float penetrationDepth;
};

class AABB {
public:
	AABB(glm::vec3 position, glm::vec3 size);

	std::optional<AABBCollisionHit> getOverlapWithOBB(const Transform& boxTransform) const;

private:
	glm::vec3 m_position;
	glm::vec3 m_size;
};
