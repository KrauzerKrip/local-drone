#pragma once

#include <entt/entt.hpp>
#include <optional>

#include "lc_client/eng_physics/entt/components.h"
#include "raycast/ray.h"
#include "collision/sphere.h"

struct CollisionHit {
	entt::entity entity;
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
	void querySphereOverlaps(const SphereOverlapQuery& collider, std::vector<CollisionHit>& outHits,
		entt::exclude_t<Exclude...> exclude = entt::exclude_t{});

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

	Sphere sphere(query.center, query.radius);

	for (auto&& [entity, colliders, transform] : entities.each()) {
		for (auto&& [colliderEnt, colliderType] : colliders.colliders) {
			if (colliderType == ColliderType::BOX) {
				Transform& colliderTransform = m_pRegistry->get<Transform>(colliderEnt);
				std::optional<SphereCollisionHit> maybeHit = sphere.getOverlapWithOBB(colliderTransform);
				if (maybeHit) {
					auto hit = *maybeHit;
					outHits.push_back(CollisionHit{.entity = entity,
						.normal = hit.normal,
						.point = hit.point,
						.penetrationDepth = hit.penetrationDepth});
				}
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
