#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "CSpline.h"
#include "Component.h"

namespace Physic {

	class WindZone : public Component
	{
		REFLEXION_HEADER(WindZone)
		COMPONENT_IMPLEMENTATION_HEADER(WindZone)

	public:
		enum EmissionType {DIRECTIONNAL = 0, RADIAL = 1};
	private:
		Math::CSpline<float> m_cspline;
		glm::vec3 m_position;
		glm::vec3 m_direction;
		float m_amplitude;
		float m_randomFactor;
		float m_frequency;
		float m_offset;
		EmissionType m_emissionType;
		bool m_isAttenuated;
		float m_radius;

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
		
		virtual void drawInInspector(Scene & scene) override;
		virtual void drawInInspector(Scene & scene, const std::vector<Component*>& components) override;

		virtual void save(Json::Value& componentRoot) const override;
		virtual void load(const Json::Value& componentRoot) override;
	};
}

REFLEXION_CPP(Physic::WindZone)
REFLEXION_InheritFrom(Physic::WindZone, Component)

