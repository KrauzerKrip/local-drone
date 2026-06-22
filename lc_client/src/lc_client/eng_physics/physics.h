#pragma once

#include <array>
#include <cmath>
#include <entt/entt.hpp>
#include <limits>
#include <optional>
#include <vector>

#include "lc_client/eng_physics/entt/components.h"
#include "raycast/ray.h"
#include "collision/sphere.h"

struct CollisionHit {
	entt::entity ownerEntity;
	entt::entity colliderEntity;

	glm::vec3 normal;
	glm::vec3 point;
	float penetrationDepth;
};

class Physics {
public:
	Physics(entt::registry* pRegistry);

	template <typename... Components, typename... Exclude>
	RaycastResult raycast(RaycastQuery query, entt::exclude_t<Exclude...> exclude = entt::exclude_t{});
	template <typename... Components, typename... Exclude>
	void querySphereOverlaps(const SphereOverlapQuery& query, std::vector<CollisionHit>& outHits,
		entt::exclude_t<Exclude...> exclude = entt::exclude_t{});
	template <typename... Components, typename... Exclude>
	void queryAABBOverlaps(const AABBOverlapQuery& query, std::vector<CollisionHit>& outHits,
		entt::exclude_t<Exclude...> exclude = entt::exclude_t{});
	std::optional<CollisionHit> sphereIntersect(
		const SphereOverlapQuery& sphereQuery, entt::entity ownerEntity, entt::entity colliderEntity);

private:
	template <typename... Components, typename... Exclude>
	std::unordered_map<entt::entity, RaycastIntersection> getIntersections(
		RaycastQuery query, entt::exclude_t<Exclude...> exclud = entt::exclude_t{});
	std::pair<entt::entity, RaycastIntersection> getMinimumDistanceIntersection(
		std::unordered_map<entt::entity, RaycastIntersection>& intersections);

	entt::registry* m_pRegistry = nullptr;
};

template <typename... Components, typename... Exclude>
void Physics::querySphereOverlaps(
	const SphereOverlapQuery& query, std::vector<CollisionHit>& outHits, entt::exclude_t<Exclude...> exclude) {
	outHits.clear();

	auto entities = m_pRegistry->view<Colliders, Transform, Components...>(exclude);

	for (auto&& [entity, colliders, transform] : entities.each()) {
		for (auto&& [colliderEnt, colliderType] : colliders.colliders) {
			if (colliderType != ColliderType::BOX) {
				continue;
			}

			if (std::optional<CollisionHit> hit = sphereIntersect(query, entity, colliderEnt)) {
				outHits.push_back(*hit);
			}
		}
	}
}

template <typename... Components, typename... Exclude>
void Physics::queryAABBOverlaps(
	const AABBOverlapQuery& query, std::vector<CollisionHit>& outHits, entt::exclude_t<Exclude...> exclude) {
	outHits.clear();

	auto entities = m_pRegistry->view<Colliders, Transform, Components...>(exclude);
	const glm::vec3 queryExtents = glm::abs(query.size);
	const std::array<glm::vec3, 3> queryAxes{
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
	};

	for (entt::entity entity : entities) {
		const Colliders& colliders = m_pRegistry->get<Colliders>(entity);
		for (auto&& [colliderEnt, colliderType] : colliders.colliders) {
			if (colliderType != ColliderType::BOX) {
				continue;
			}

			const Transform& colliderTransform = m_pRegistry->get<Transform>(colliderEnt);
			const glm::vec3 colliderExtents = glm::abs(colliderTransform.scale);
			const std::array<glm::vec3, 3> colliderAxes{
				colliderTransform.rotation * queryAxes[0],
				colliderTransform.rotation * queryAxes[1],
				colliderTransform.rotation * queryAxes[2],
			};
			const glm::vec3 centerOffset = query.position - colliderTransform.position;

			float minimumPenetration = std::numeric_limits<float>::max();
			glm::vec3 collisionNormal(0.0f);
			bool overlaps = true;

			auto testAxis = [&](glm::vec3 axis) {
				const float axisLengthSquared = glm::dot(axis, axis);
				if (axisLengthSquared < 0.00000001f) {
					return;
				}

				axis /= std::sqrt(axisLengthSquared);
				float queryRadius = 0.0f;
				float colliderRadius = 0.0f;
				for (std::size_t dimension = 0; dimension < 3; ++dimension) {
					queryRadius += queryExtents[dimension] * std::abs(glm::dot(queryAxes[dimension], axis));
					colliderRadius += colliderExtents[dimension] * std::abs(glm::dot(colliderAxes[dimension], axis));
				}

				const float signedDistance = glm::dot(centerOffset, axis);
				const float penetration = queryRadius + colliderRadius - std::abs(signedDistance);
				if (penetration < 0.0f) {
					overlaps = false;
					return;
				}

				if (penetration < minimumPenetration) {
					minimumPenetration = penetration;
					collisionNormal = signedDistance < 0.0f ? -axis : axis;
				}
			};

			for (const glm::vec3& axis : queryAxes) {
				testAxis(axis);
				if (!overlaps)
					break;
			}
			for (const glm::vec3& axis : colliderAxes) {
				if (!overlaps)
					break;
				testAxis(axis);
			}
			for (const glm::vec3& queryAxis : queryAxes) {
				for (const glm::vec3& colliderAxis : colliderAxes) {
					if (!overlaps)
						break;
					testAxis(glm::cross(queryAxis, colliderAxis));
				}
				if (!overlaps)
					break;
			}

			if (!overlaps) {
				continue;
			}

			glm::vec3 point = colliderTransform.position;
			for (std::size_t dimension = 0; dimension < 3; ++dimension) {
				const float normalProjection = glm::dot(collisionNormal, colliderAxes[dimension]);
				const float localPoint =
					std::abs(normalProjection) > 0.00001f
						? (normalProjection < 0.0f ? -colliderExtents[dimension] : colliderExtents[dimension])
						: glm::clamp(glm::dot(centerOffset, colliderAxes[dimension]), -colliderExtents[dimension],
							  colliderExtents[dimension]);
				point += colliderAxes[dimension] * localPoint;
			}

			outHits.push_back(CollisionHit{.ownerEntity = entity,
				.colliderEntity = colliderEnt,
				.normal = collisionNormal,
				.point = point,
				.penetrationDepth = minimumPenetration});
		}
	}
}

template <typename... Components, typename... Exclude>
RaycastResult Physics::raycast(RaycastQuery query, entt::exclude_t<Exclude...> exclude) {
	std::unordered_map<entt::entity, RaycastIntersection> intersections =
		getIntersections<Components...>(query, exclude);

	if (intersections.size() > 0) {
		auto intersection = getMinimumDistanceIntersection(intersections);

		auto optionalEntity = std::make_optional<entt::entity>(intersection.first);
		auto optionalPoint = std::make_optional<glm::vec3>(intersection.second.point);
		auto optionalDistance = std::make_optional<float>(intersection.second.distance);

		return RaycastResult(optionalEntity, optionalPoint, optionalDistance);
	}
	else {
		return RaycastResult(std::nullopt, std::nullopt, std::nullopt);
	}
}

template <typename... Components, typename... Exclude>
std::unordered_map<entt::entity, RaycastIntersection> Physics::getIntersections(
	RaycastQuery query, entt::exclude_t<Exclude...> exclude) {
	auto colliderEntities = m_pRegistry->view<Colliders, Transform, Components...>(exclude);

	std::unordered_map<entt::entity, RaycastIntersection> intersections;

	for (entt::entity entity : colliderEntities) {
		auto&& [colliders, transform] = m_pRegistry->get<Colliders, Transform>(entity);

		Ray ray(query.position, query.direction);

		for (auto&& [colliderEnt, colliderType] : colliders.colliders) {
			std::optional<RaycastIntersection> raycastResult;

			if (colliderType == ColliderType::BOX) {
				Transform& colliderTransform = m_pRegistry->get<Transform>(colliderEnt);
				Transform boxTransform;
				boxTransform.position = transform.position + colliderTransform.position;
				boxTransform.rotation = transform.rotation * colliderTransform.rotation;
				boxTransform.scale = colliderTransform.scale;
				raycastResult = ray.getIntersectionWithOBB(colliderTransform);
			}

			if (raycastResult) {
				intersections.emplace(entity, *raycastResult);
			}
		}
	}

	return intersections;
}
