#include "physics.h"

#include "lc_client/eng_physics/entt/components.h"
#include "lc_client/eng_scene/entt/components.h"
#include <glm/glm.hpp>

Physics::Physics(entt::registry* pRegistry) { m_pRegistry = pRegistry; }

std::optional<CollisionHit> Physics::sphereIntersect(
	const SphereOverlapQuery& sphereQuery, entt::entity ownerEntity, entt::entity colliderEntity) {
	if (!m_pRegistry->all_of<BoxCollider, Transform>(colliderEntity)) {
		return std::nullopt;
	}

	Sphere sphere(sphereQuery.center, sphereQuery.radius);
	const Transform& colliderTransform = m_pRegistry->get<Transform>(colliderEntity);
	const std::optional<SphereCollisionHit> sphereHit = sphere.getOverlapWithOBB(colliderTransform);
	if (!sphereHit) {
		return std::nullopt;
	}

	return CollisionHit{
		.ownerEntity = ownerEntity,
		.colliderEntity = colliderEntity,
		.colliderType = ColliderType::BOX,
		.normal = sphereHit->normal,
		.point = sphereHit->point,
		.penetrationDepth = sphereHit->penetrationDepth,
	};
}

bool Physics::sphereIntersectCollider(
	const Sphere& sphere, ColliderType colliderType, const Transform& colliderTransform, CollisionHit& outHit) const {
	switch (colliderType) {
	case ColliderType::BOX: {
		const auto maybeHit = sphere.getOverlapWithOBB(colliderTransform);
		if (maybeHit) {
			SphereCollisionHit sphereHit = *maybeHit;
			outHit.normal = sphereHit.normal;
			outHit.point = sphereHit.point;
			outHit.penetrationDepth = sphereHit.penetrationDepth;
			return true;
		}
		return false;
	}

	default:
		return false;
	}
}


std::pair<entt::entity, RaycastIntersection> Physics::getMinimumDistanceIntersection(
	std::unordered_map<entt::entity, RaycastIntersection>& intersections) {

	auto firstElement = *intersections.begin();

	entt::entity minDistanceEntity = firstElement.first;
	RaycastIntersection minRaycastIntersection(firstElement.second.point, firstElement.second.distance);

	for (auto& [ent, raycastIntersection] : intersections) {
		if (raycastIntersection.distance < minRaycastIntersection.distance) {
			minDistanceEntity = ent;
			minRaycastIntersection = raycastIntersection;
		}
	}

	return std::pair<entt::entity, RaycastIntersection>(minDistanceEntity, minRaycastIntersection);
}
