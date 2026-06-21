#include "sphere.h"
#include "lc_client/eng_physics/entt/components.h"
#include <array>
#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <limits>
#include <optional>

Sphere::Sphere(glm::vec3 center, float radius) : m_center(center), m_radius(radius) {}


std::optional<SphereCollisionHit> Sphere::getOverlapWithOBB(const Transform& boxTransform) {
	glm::vec3 closestPoint = getClosestPointOnOBB(boxTransform);
	glm::vec3 fromBoxToSphere = m_center - closestPoint;

	float distSq = glm::dot(fromBoxToSphere, fromBoxToSphere);
	float radiusSq = m_radius * m_radius;

	if (distSq > radiusSq) {
		return std::nullopt;
	}

	float dist = std::sqrt(distSq);

	glm::vec3 normal;
	float penetrationDepth;

	if (dist > 0.00001f) {
		normal = fromBoxToSphere / dist;
		penetrationDepth = m_radius - dist;
	}
	else {
		const std::array<glm::vec3, 3> axes{
			boxTransform.rotation * glm::vec3(1, 0, 0),
			boxTransform.rotation * glm::vec3(0, 1, 0),
			boxTransform.rotation * glm::vec3(0, 0, 1),
		};
		const glm::vec3 centerOffset = m_center - boxTransform.position;
		float distanceToFace = std::numeric_limits<float>::max();

		for (size_t dimension = 0; dimension < axes.size(); dimension++) {
			const float localPosition = glm::dot(centerOffset, axes[dimension]);
			const float faceDistance = std::abs(boxTransform.scale[dimension]) - std::abs(localPosition);
			if (faceDistance < distanceToFace) {
				distanceToFace = faceDistance;
				normal = localPosition < 0.0f ? -axes[dimension] : axes[dimension];
			}
		}

		closestPoint = m_center + normal * distanceToFace;
		penetrationDepth = m_radius + distanceToFace;
	}

	return SphereCollisionHit{.point = closestPoint, .normal = normal, .penetrationDepth = penetrationDepth};
}

glm::vec3 Sphere::getClosestPointOnOBB(const Transform& boxTransform) {
	glm::vec3 distanceVec = m_center - boxTransform.position;
	glm::vec3 closesPoint = boxTransform.position;

	std::array<glm::vec3, 3> axes{
		boxTransform.rotation * glm::vec3(1, 0, 0),
		boxTransform.rotation * glm::vec3(0, 1, 0),
		boxTransform.rotation * glm::vec3(0, 0, 1),
	};
	for (uint dimIdx = 0; dimIdx < 3; dimIdx++) {
		float dist = glm::dot(distanceVec, axes[dimIdx]);

		float halfExtent = boxTransform.scale[dimIdx];

		dist = glm::clamp(dist, -halfExtent, halfExtent);

		closesPoint += dist * axes[dimIdx];
	}

	return closesPoint;
}
