#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>


struct CableAnchor {};

struct CableAttachment {};



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

struct CableConstraint {
	size_t indexA;
	size_t indexB;
	float lambda;
	float restDistance;
	float compliance;
};

struct Cable {
	// cache friendly data allocation
	std::vector<CableParticle> particles;
	std::vector<CableConstraint> constraints;
};

struct CableCollisionConstraint {
	glm::vec3 normal;
	float c;
	float lambda;
	float compliance;
}
