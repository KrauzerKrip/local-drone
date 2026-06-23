#pragma once

#include <array>
#include <cmath>
#include <entt/entt.hpp>
#include <limits>
#include <optional>
#include <vector>

#include "lc_client/eng_physics/entt/components.h"
#include "lc_client/eng_scene/entt/components.h"
#include "raycast/ray.h"
#include "collision/aabb.h"
#include "collision/sphere.h"

struct CollisionHit {
	entt::entity ownerEntity;
	entt::entity colliderEntity;
	ColliderType colliderType;

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
	std::optional<CollisionHit> AABBIntersect(
		const AABBOverlapQuery& aabbQuery, entt::entity ownerEntity, entt::entity colliderEntity);
	/**
	 * @brief Computes contact between a sphere and a single collider.
	 *
	 * This function performs no ECS lookups.
	 * It only fills geometric contact fields:
	 * - normal
	 * - point
	 * - penetrationDepth
	 *
	 * The caller is responsible for filling:
	 * - ownerEntity
	 * - colliderEntity
	 * - colliderType
	 */
	bool sphereIntersectCollider(const Sphere& sphere, ColliderType colliderType, const Transform& colliderTransform,
		CollisionHit& outHit) const;
	bool AABBIntersectCollider(const AABBOverlapQuery& query, ColliderType colliderType,
		const Transform& colliderTransform, CollisionHit& outHit) const;

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

	for (auto&& [entity, colliders, transform] : entities.each()) {
		for (auto&& [colliderEnt, colliderType] : colliders.colliders) {
			if (colliderType != ColliderType::BOX) {
				continue;
			}

			if (std::optional<CollisionHit> hit = AABBIntersect(query, entity, colliderEnt)) {
				outHits.push_back(*hit);
			}
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
