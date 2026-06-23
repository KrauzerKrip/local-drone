#include "lc_client/eng_physics/physics.h"
#include <cstddef>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <entt/entt.hpp>

struct CableAnchor {};

struct CableAttachment {};

struct CableColliderCandidate {
	entt::entity ownerEntity = entt::null;
	entt::entity colliderEntity = entt::null;
	ColliderType colliderType;
	Transform colliderTransform;
};

struct CableParticle {
	float inverseMass;
	glm::vec3 inverseInertia;

	// current state
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 linearVelocity;
	glm::vec3 angularVelocity;

	// previous state (for velocity derivation)
	glm::vec3 prevPosition;
	glm::quat prevRotation;

};

struct CableDistanceConstraint {
	size_t indexParticleA;
	size_t indexParticleB;
	float lambda;
	float restDistance;
	float compliance;
};

struct CableCollisionConstraint {
	size_t particleIndex;
	glm::vec3 normal;
	glm::vec3 point;
	float particleRadius;
	float lambda;
	float compliance;
};

struct Cable {
	// cache friendly data allocation
	std::vector<CableParticle> particles;
	std::vector<CableDistanceConstraint> constraints;
	std::vector<std::vector<size_t>> coloredConstraints;
	bool coloredConstraintsDirty = true;
	size_t coloredConstraintCount = 0;
	std::vector<CableCollisionConstraint> collisionConstraints;

	std::vector<CableColliderCandidate> colliderCandidates;
	std::vector<size_t> particleColliderCandidateOffsets;
	std::vector<size_t> particleColliderCandidateIndices;
	std::vector<bool> particleHadCollisionScratch;
	std::vector<glm::vec3> particleCollisionNormalScratch;
};
