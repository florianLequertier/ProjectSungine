#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "CSpline.h"
#include "Component.h"

#include "Enums.h"

namespace Physic {

	class WindZone : public Component
	{
	public:
		ENUM(EmissionType, DIRECTIONAL, RADIAL)
		//enum EmissionType { DIRECTIONNAL = 0, RADIAL = 1 };

		COMPONENT_IMPLEMENTATION_HEADER(WindZone)

		CLASS((WindZone, Component),
			((PRIVATE)
				(Math::CSpline<float>	, m_cspline			, "hide"			) // pas de display dans l'ui meme si ça serai cool
				(glm::vec3				, m_position		, "hide"			)
				(glm::vec3				, m_direction							)
				(float					, m_amplitude		, "", updateSpline	)
				(float					, m_randomFactor	, "", updateSpline	)
				(float					, m_frequency		, "", updateSpline	)
				(float					, m_offset			, "", updateSpline	)
				(EmissionType			, m_emissionType	, "", updateSpline	)
				(bool					, m_isAttenuated	, "", updateSpline	)
				(float					, m_radius								)
			)
		)

	private:

		// TODO : remove ?
		//for UI : 
		const char** m_emissionTypeNames;

	public:
		WindZone();
		~WindZone();

		glm::vec3 getForce(float t);
		glm::vec3 WindZone::getForce(float t, glm::vec3 position);

		void updateSpline();

		glm::vec3 getPosition() const;
		float getAmplitude() const;
		glm::vec3 getDirection() const;
		float getRandomFactor() const;
		float getFrequency() const;

		void setAmplitude(float amplitude);
		void setDirection(glm::vec3 direction);
		void setRandomFactor(float randomFactor);
		void setFrequency(float frequency);

		virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;
		
		//virtual void drawInInspector(Scene & scene) override;
		//virtual void drawInInspector(Scene & scene, const std::vector<Component*>& components) override;

		virtual void save(Json::Value& componentRoot) const override;
		virtual void load(const Json::Value& componentRoot) override;
	};
}

REGISTER_CLASS(Physic::WindZone)

