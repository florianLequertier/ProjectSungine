

#include "WindZone.h"
#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "Factories.h"

namespace Physic {

	COMPONENT_IMPLEMENTATION_CPP(WindZone)

	WindZone::WindZone() : Component(ComponentType::WIND_ZONE), m_direction(0, 0, 1), m_amplitude(1), m_frequency(1), m_randomFactor(0), m_offset(0), m_emissionType(EmissionType::DIRECTIONNAL), m_isAttenuated(false), m_radius(1.f)
	{
		//for UI : 
		m_emissionTypeNames = new const char*[2];
		m_emissionTypeNames[0] = "directionnal";
		m_emissionTypeNames[1] = "radial";

		m_cspline.resize(10);
		updateSpline();
	}


	WindZone::~WindZone()
	{
		delete[] m_emissionTypeNames;
	}

	//Warning : bypass the attenuation, force emissionType to be directional : 
	glm::vec3 WindZone::getForce(float t)
	{
		t *= m_frequency;
		t *= 1000.f;
		t = ((int)t) % 1000;
		t *= 0.001f;
		return m_direction * m_cspline.get(t);
	}

	glm::vec3 WindZone::getForce(float t, glm::vec3 position)
	{
		t *= m_frequency;
		t *= 1000.f;
		t = ((int)t) % 1000;
		t *= 0.001f;
		float distance = glm::length(position - m_position) + 0.000001f;
		
		float attenuation = m_isAttenuated ? std::max(0.f, (1 - distance / (m_radius + 0.0000001f))) : 1;

		glm::vec3 direction = (m_emissionType == EmissionType::DIRECTIONNAL) ? m_direction : glm::normalize(position - m_position);

		return direction * m_cspline.get(t) * attenuation;
	}

	void WindZone::updateSpline()
	{
		for (int i = 0; i < m_cspline.size(); i++)
		{
			float direction = (i % 2 == 0) ? 1 : -1;
			float newPoint = m_offset + m_amplitude * direction + m_randomFactor * (float)(rand() % 100) / 100.f;
			m_cspline[i] = (newPoint);
		}
	}

	glm::vec3 WindZone::getPosition() const
	{
		return m_position;
	}

	float WindZone::getAmplitude() const
	{
		return m_amplitude;
	}

	glm::vec3 WindZone::getDirection() const
	{
		return m_direction;
	}

	float WindZone::getRandomFactor() const
	{
		return m_randomFactor;
	}

	float WindZone::getFrequency() const
	{
		return m_frequency;
	}

	void WindZone::setAmplitude(float amplitude)
	{
		m_amplitude = amplitude;
	}

	void WindZone::setDirection(glm::vec3 direction)
	{
		m_direction = direction;
	}

	void WindZone::setRandomFactor(float randomFactor)
	{
		m_randomFactor = randomFactor;
	}

	void WindZone::setFrequency(float frequency)
	{
		m_frequency = frequency;
	}

	void WindZone::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
	{
		m_position = translation;
		m_direction = rotation * glm::vec3(0, 0, 1);
	}

	void WindZone::drawInInspector(Scene & scene)
	{
		if (ImGui::InputFloat("amplitude", &m_amplitude))
			updateSpline();
		if (ImGui::InputFloat("frequency", &m_frequency))
			updateSpline();
		if (ImGui::InputFloat("offset", &m_offset))
			updateSpline();
		if (ImGui::InputFloat("random factor", &m_randomFactor))
			updateSpline();
		int currentItemTypeEmission = (int)m_emissionType;
		if (ImGui::ListBox("emission type", &currentItemTypeEmission, m_emissionTypeNames, 2))
			m_emissionType = (EmissionType)currentItemTypeEmission;
		if (ImGui::RadioButton("attenuation", m_isAttenuated))
			m_isAttenuated = !m_isAttenuated;
		ImGui::SliderFloat("radius", &m_radius, 0.1f, 500.f);
	}

	void WindZone::drawInInspector(Scene & scene, const std::vector<Component*>& components)
	{
		if (ImGui::InputFloat("amplitude", &m_amplitude))
		{
			for(auto component : components)
			{
				WindZone* castedComponent = static_cast<WindZone*>(component);
				castedComponent->m_amplitude = m_amplitude;
				castedComponent->updateSpline();
			}
		}
		if (ImGui::InputFloat("frequency", &m_frequency))
		{
			for (auto component : components)
			{
				WindZone* castedComponent = static_cast<WindZone*>(component);
				castedComponent->m_frequency = m_frequency;
				castedComponent->updateSpline();
			}
		}
		if (ImGui::InputFloat("offset", &m_offset))
		{
			for (auto component : components)
			{
				WindZone* castedComponent = static_cast<WindZone*>(component);
				castedComponent->m_offset = m_offset;
				castedComponent->updateSpline();
			}
		}
		if (ImGui::InputFloat("random factor", &m_randomFactor))
		{
			for (auto component : components)
			{
				WindZone* castedComponent = static_cast<WindZone*>(component);
				castedComponent->m_randomFactor;
				castedComponent->updateSpline();
			}
		}
		int currentItemTypeEmission = (int)m_emissionType;
		if (ImGui::ListBox("emission type", &currentItemTypeEmission, m_emissionTypeNames, 2))
		{
			for (auto component : components)
			{
				WindZone* castedComponent = static_cast<WindZone*>(component);
				castedComponent->m_emissionType = (EmissionType)currentItemTypeEmission;
			}
		}
		if (ImGui::RadioButton("attenuation", m_isAttenuated))
		{
			m_isAttenuated = !m_isAttenuated;
			for (auto component : components)
			{
				if (component == this) continue;
				WindZone* castedComponent = static_cast<WindZone*>(component);
				castedComponent->m_isAttenuated = m_isAttenuated;
			}
		}
		if (ImGui::SliderFloat("radius", &m_radius, 0.1f, 500.f))
		{
			for (auto component : components)
			{
				WindZone* castedComponent = static_cast<WindZone*>(component);
				castedComponent->m_radius = m_radius;
			}
		}
	}

	void WindZone::save(Json::Value & componentRoot) const
	{
		Component::save(componentRoot);

		componentRoot["cspline"] = toJsonValue(m_cspline);
		componentRoot["position"] = toJsonValue(m_position);
		componentRoot["direction"] = toJsonValue(m_direction);
		componentRoot["amplitude"] = m_amplitude;
		componentRoot["randomFactor"] = m_randomFactor;
		componentRoot["frequency"] = m_frequency;
		componentRoot["emissionType"] = (int)m_emissionType;
		componentRoot["isAttenuated"] = m_isAttenuated;
		componentRoot["radius"] = m_radius;

		//no need to save emissionTypeNames
	}
	void WindZone::load(const Json::Value & componentRoot)
	{
		Component::load(componentRoot);

		m_cspline = fromJsonValue<Math::CSpline<float>>(componentRoot["cspline"], Math::CSpline<float>());
		m_position = fromJsonValue<glm::vec3>(componentRoot["position"], glm::vec3());
		m_direction = fromJsonValue<glm::vec3>(componentRoot["direction"], glm::vec3());
		m_amplitude = componentRoot.get("amplitude", 0).asFloat();
		m_randomFactor = componentRoot.get("randomFactor", 0).asFloat();
		m_frequency = componentRoot.get("frequency", 0).asFloat();
		m_emissionType = (Physic::WindZone::EmissionType)componentRoot.get("emissionType", 0).asInt();
		m_isAttenuated = componentRoot.get("isAttenuated", 0).asBool();
		m_radius = componentRoot.get("radius", 0).asFloat();

		//set default values to emissionTypeNames : 
		m_emissionTypeNames = new const char*[2];
		m_emissionTypeNames[0] = "directionnal";
		m_emissionTypeNames[1] = "radial";
	}
}
