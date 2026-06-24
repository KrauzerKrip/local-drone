#include "aabb.h"
#include <array>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include <limits>
#include <optional>

AABB::AABB(glm::vec3 position, glm::vec3 size) : m_position(position), m_size(size) {}

std::optional<AABBCollisionHit> AABB::getOverlapWithOBB(const Transform& boxTransform) const {
	const glm::vec3 aabbExtents = glm::abs(m_size);
	const std::array<glm::vec3, 3> aabbAxes{
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
	};
	const glm::vec3 colliderExtents = glm::abs(boxTransform.scale);
	const glm::mat3 colliderRotation = glm::toMat3(boxTransform.rotation);
	const std::array<glm::vec3, 3> colliderAxes{
		colliderRotation[0],
		colliderRotation[1],
		colliderRotation[2],
	};
	const glm::vec3 centerOffset = m_position - boxTransform.position;

	float minimumPenetration = std::numeric_limits<float>::max();
	glm::vec3 collisionNormal(0.0f);
	bool overlaps = true;

	auto testAxis = [&](glm::vec3 axis) {
		const float axisLengthSquared = glm::dot(axis, axis);
		if (axisLengthSquared < 0.00000001f) {
			return;
		}

		axis /= std::sqrt(axisLengthSquared);
		float aabbRadius = 0.0f;
		float colliderRadius = 0.0f;
		for (std::size_t dimension = 0; dimension < 3; ++dimension) {
			aabbRadius += aabbExtents[dimension] * std::abs(glm::dot(aabbAxes[dimension], axis));
			colliderRadius += colliderExtents[dimension] * std::abs(glm::dot(colliderAxes[dimension], axis));
		}

		const float signedDistance = glm::dot(centerOffset, axis);
		const float penetration = aabbRadius + colliderRadius - std::abs(signedDistance);
		if (penetration < 0.0f) {
			overlaps = false;
			return;
		}

		if (penetration < minimumPenetration) {
			minimumPenetration = penetration;
			collisionNormal = signedDistance < 0.0f ? -axis : axis;
		}
	};

	for (const glm::vec3& axis : aabbAxes) {
		testAxis(axis);
		if (!overlaps) {
			break;
		}
	}
	for (const glm::vec3& axis : colliderAxes) {
		if (!overlaps) {
			break;
		}
		testAxis(axis);
	}
	for (const glm::vec3& aabbAxis : aabbAxes) {
		for (const glm::vec3& colliderAxis : colliderAxes) {
			if (!overlaps) {
				break;
			}
			testAxis(glm::cross(aabbAxis, colliderAxis));
		}
		if (!overlaps) {
			break;
		}
	}

	if (!overlaps) {
		return std::nullopt;
	}

	glm::vec3 point = boxTransform.position;
	for (std::size_t dimension = 0; dimension < 3; ++dimension) {
		const float normalProjection = glm::dot(collisionNormal, colliderAxes[dimension]);
		const float localPoint =
			std::abs(normalProjection) > 0.00001f
				? (normalProjection < 0.0f ? -colliderExtents[dimension] : colliderExtents[dimension])
				: glm::clamp(glm::dot(centerOffset, colliderAxes[dimension]), -colliderExtents[dimension],
					  colliderExtents[dimension]);
		point += colliderAxes[dimension] * localPoint;
	}

	return AABBCollisionHit{.point = point, .normal = collisionNormal, .penetrationDepth = minimumPenetration};
}
