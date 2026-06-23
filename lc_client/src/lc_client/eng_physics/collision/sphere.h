#pragma once

#include "lc_client/eng_physics/entt/components.h"
#include "lc_client/eng_scene/entt/components.h"

struct SphereCollisionHit {
	glm::vec3 point;
	glm::vec3 normal;
	float penetrationDepth;
};


class Sphere {
public:
	Sphere(glm::vec3 center, float radius);

	std::optional<SphereCollisionHit> getOverlapWithOBB(const Transform& boxTransform) const;

private:
	glm::vec3 getClosestPointOnOBB(const Transform& boxTransform) const;

	glm::vec3 m_center;
	float m_radius;
};
