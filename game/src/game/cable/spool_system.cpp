#include "spool_system.h"

#include "components.h"
#include "lc_client/eng_input/key_code.h"
#include <cstddef>
#include <optional>

SpoolSystem::SpoolSystem(Physics* pPhysics, entt::registry* pRegistry) {
	m_pPhysics = pPhysics;
	m_pRegistry = pRegistry;
}

void SpoolSystem::update(double updateInterval) {
	auto cables = m_pRegistry->view<Cable, Spool, CableProperties>();
	const float holdTension = 5.0f;
	const float maxPayoutTension = 30.0f;
	const float maxPayoutSpeed = 2.0f;
	const float particleMass = 0.001;

	for (auto&& [entity, cable, spool, properties] : cables.each()) {
		if (cable.constraints.size() == 0) {
			continue;
		}
		CableDistanceConstraint& spoolConstraint = cable.constraints.back();
		CableParticle& spoolParticle = cable.particles.back();

		float tension = spoolConstraint.maxTension;
		float u = glm::clamp((tension - holdTension) / (maxPayoutTension - holdTension), 0.0f, 1.0f);

		// u = u * u;

		float payoutSpeed = u * maxPayoutSpeed;
		float payoutLength = payoutSpeed * updateInterval;

		spoolConstraint.restDistance += payoutLength;

		if (cable.currentLength > spool.maxLength) {
			// we snap!!!
			cable.particles.back().attachment = std::nullopt;
		}

		// if we snapped
		if (!cable.particles.back().attachment.has_value()) {
			// we do nothing
			continue;
		}

		while (spoolConstraint.restDistance > properties.segmentInterval) {
			spoolConstraint = cable.constraints.back();
			spoolParticle = cable.particles.back();

			auto spoolAttachment = spoolParticle.attachment;
			const float diff = spoolConstraint.restDistance - properties.segmentInterval;

			spoolConstraint.restDistance = properties.segmentInterval;

			CableParticle particle = {.inverseMass = 1 / particleMass,
				.inverseInertia = glm::vec3(0, 0, 0),
				.position = spoolParticle.position,
				.rotation = glm::vec3(0, 0, 0),
				.linearVelocity = glm::vec3(0, 0, 0),
				.angularVelocity = glm::vec3(0, 0, 0),
				.attachment = spoolAttachment};
			cable.particles.push_back(particle);
			CableDistanceConstraint constraint = {
				.indexParticleA = cable.particles.size() - 2,
				.indexParticleB = cable.particles.size() - 1,
				.lambda = 0,
				.restDistance = diff,
				.compliance = 0.0f,
				.maxTension = 0.0f,
			};
			cable.constraints.push_back(constraint);
			cable.particles[cable.particles.size() - 2].attachment = std::nullopt;
		}
	}
}
