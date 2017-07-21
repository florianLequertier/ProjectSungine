

#include "Lights.h"
#include "Scene.h"
#include "SceneAccessor.h"
#include "Factories.h"


Light::Light(float _intensity, glm::vec3 _color) 
	: Component(LIGHT)
	, intensity(_intensity)
	, color(_color)
	, castShadows(true)
	, useFlare(true)
{

}

Light::~Light()
{
}

float Light::getIntensity() const
{
	return intensity;
}

glm::vec3 Light::getColor() const
{
	return color;
}

bool Light::getCastShadows() const
{
	return castShadows;
}

bool Light::getUseFlare() const
{
	return useFlare;
}

const MaterialFlares * Light::getFlareMaterial() const
{
	return static_cast<const MaterialFlares*>(flareMaterial.get());
}

void Light::setIntensity(float i)
{
	intensity = i;

	//light bounding box is based on the intensity, so we have to update it when we change the light intensity
	updateBoundingBox();
}

void Light::setColor(const glm::vec3 & c)
{
	color = c;

	//light bounding box is based on the colot, so we have to update it when we change the light color
	updateBoundingBox();
}

void Light::setCastShadows(bool state)
{
	castShadows = state;
}

void Light::setUseFlare(bool state)
{
	useFlare = state;
}

void Light::updateBoundingBox()
{
	//nothing by default
}

void Light::save(Json::Value & rootComponent) const
{
	Component::save(rootComponent);

	rootComponent["intensity"] = intensity;
	rootComponent["color"] = toJsonValue(color);
}

void Light::load(const Json::Value & rootComponent)
{
	Component::load(rootComponent);

	intensity = rootComponent.get("intensity", 1).asFloat();
	color = fromJsonValue<glm::vec3>(rootComponent["color"], glm::vec3(1,1,1));
}

///////////////////////////////

COMPONENT_IMPLEMENTATION_CPP(PointLight)

PointLight::PointLight(float _intensity, glm::vec3 _color, glm::vec3 _position) : Light(_intensity, _color), position(_position)
{
	m_type = POINT_LIGHT;

	updateBoundingBox();
}

PointLight::~PointLight()
{
}

void PointLight::updateBoundingBox()
{
	float lightRadius = std::sqrt(intensity * (0.3*color.r + 0.6*color.g + 0.1*color.b) / 0.003f); // radius based on light intensity and light luminance
	
	boundingBox.applyTranslation(position);
	boundingBox.applyScale(glm::vec3(lightRadius, lightRadius, lightRadius));
}

void PointLight::drawInInspector(Scene& scene)
{
	if (ImGui::SliderFloat("light intensity", &intensity, 0.f, 50.f))
		updateBoundingBox();
	if (ImGui::ColorEdit3("light color", &color[0]))
		updateBoundingBox();
	if (ImGui::RadioButton("cast shadows", castShadows))
		castShadows = !castShadows;
	if (ImGui::RadioButton("use flare", useFlare))
		useFlare = !useFlare;
	EditorGUI::ResourceField("Flare material", flareMaterial);
}

void PointLight::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	float _intensity = intensity;
	if (ImGui::SliderFloat("light intensity", &_intensity, 0.f, 50.f))
	{
		for (auto component : components)
		{
			PointLight* castedComponent = static_cast<PointLight*>(component);

			castedComponent->intensity = _intensity;
			castedComponent->updateBoundingBox();
		}
	}
	glm::vec3 _color = color;
	if (ImGui::ColorEdit3("light color", &_color[0]))
	{
		for (auto component : components)
		{
			PointLight* castedComponent = static_cast<PointLight*>(component);

			castedComponent->color = _color;
			castedComponent->updateBoundingBox();
		}
	}
	if (ImGui::RadioButton("cast shadows", castShadows))
	{
		castShadows = !castShadows;
		for (auto component : components)
		{
			PointLight* castedComponent = static_cast<PointLight*>(component);
			castedComponent->castShadows = castShadows;
		}
	}
	if (ImGui::RadioButton("use Flare", useFlare))
	{
		useFlare = !useFlare;
		for (auto component : components)
		{
			PointLight* castedComponent = static_cast<PointLight*>(component);
			castedComponent->useFlare = useFlare;
		}
	}
	if (EditorGUI::ResourceField("Flare material", flareMaterial))
	{
		for (auto component : components)
		{
			PointLight* castedComponent = static_cast<PointLight*>(component);
			castedComponent->flareMaterial = flareMaterial;
		}
	}

}

void PointLight::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	position = translation;

	updateBoundingBox();
}

void PointLight::setBoundingBoxVisual(ResourcePtr<Mesh> visualMesh, ResourcePtr<MaterialInstance> visualMaterial)
{
	boundingBox.setVisual(visualMesh, visualMaterial);
}

void PointLight::renderBoundingBox(const glm::mat4& projectile, const glm::mat4& view, glm::vec3 _color)
{
	boundingBox.render(projectile, view, _color);
}

void PointLight::save(Json::Value & rootComponent) const
{
	Light::save(rootComponent);

	rootComponent["position"] = toJsonValue(position);
	rootComponent["boundingBox"] = toJsonValue(boundingBox);
}

void PointLight::load(const Json::Value & rootComponent)
{
	Light::load(rootComponent);

	position = fromJsonValue<glm::vec3>(rootComponent["position"], glm::vec3());
	boundingBox = fromJsonValue<BoxCollider>(rootComponent["boundingBox"], BoxCollider());
}

////////////////////////////////

COMPONENT_IMPLEMENTATION_CPP(DirectionalLight)

DirectionalLight::DirectionalLight(float _intensity, glm::vec3 _color, glm::vec3 _direction) :
	Light(_intensity, _color), direction(_direction), up(1,0,0)
{
	m_type = DIRECTIONAL_LIGHT;
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::drawInInspector(Scene& scene)
{
	ImGui::SliderFloat("light intensity", &intensity, 0.f, 10.f);
	ImGui::ColorEdit3("light color", &color[0]);
	if (ImGui::RadioButton("cast shadows", castShadows))
		castShadows = !castShadows;
	if (ImGui::RadioButton("use flare", useFlare))
		useFlare = !useFlare;
	EditorGUI::ResourceField("Flare material", flareMaterial);
}

void DirectionalLight::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	float _intensity = intensity;
	if (ImGui::SliderFloat("light intensity", &_intensity, 0.f, 10.f))
	{
		for (auto component : components)
		{
			DirectionalLight* castedComponent = static_cast<DirectionalLight*>(component);
			castedComponent->intensity = _intensity;
		}
	}
	glm::vec3 _color = color;
	if (ImGui::ColorEdit3("light color", &_color[0]))
	{
		for (auto component : components)
		{
			DirectionalLight* castedComponent = static_cast<DirectionalLight*>(component);
			castedComponent->color = _color;
		}
	}
	if (ImGui::RadioButton("cast shadows", castShadows))
	{
		castShadows = !castShadows;
		for (auto component : components)
		{
			DirectionalLight* castedComponent = static_cast<DirectionalLight*>(component);
			castedComponent->castShadows = castShadows;
		}
	}
	if (ImGui::RadioButton("use Flare", useFlare))
	{
		useFlare = !useFlare;
		for (auto component : components)
		{
			DirectionalLight* castedComponent = static_cast<DirectionalLight*>(component);
			castedComponent->useFlare = useFlare;
		}
	}
	if (EditorGUI::ResourceField("Flare material", flareMaterial))
	{
		for (auto component : components)
		{
			DirectionalLight* castedComponent = static_cast<DirectionalLight*>(component);
			castedComponent->flareMaterial = flareMaterial;
		}
	}
}

void DirectionalLight::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	glm::mat3 rotMat = glm::mat3_cast(rotation);
	up = glm::normalize( rotMat * glm::vec3(1, 0, 0) );
	direction = glm::normalize( rotMat * glm::vec3(0, -1, 0) );
	position = translation;
}

void DirectionalLight::save(Json::Value & rootComponent) const
{
	Light::save(rootComponent);

	rootComponent["up"] = toJsonValue(up);
	rootComponent["direction"] = toJsonValue(direction);
}

void DirectionalLight::load(const Json::Value & rootComponent)
{
	Light::load(rootComponent);

	up = fromJsonValue<glm::vec3>(rootComponent["up"], glm::vec3());
	direction = fromJsonValue<glm::vec3>(rootComponent["direction"], glm::vec3());
}

////////////////////////////////////

COMPONENT_IMPLEMENTATION_CPP(SpotLight)

SpotLight::SpotLight(float _intensity, glm::vec3 _color, glm::vec3 _position, glm::vec3 _direction, float _angle) :
	Light(_intensity, _color), position(_position), direction(_direction), angle(_angle), up(1,0,0)
{
	m_type = SPOT_LIGHT;

	updateBoundingBox();
}

SpotLight::~SpotLight()
{
}

void SpotLight::updateBoundingBox()
{
	float lightRadius = std::sqrt(intensity * (0.3*color.r + 0.6*color.g + 0.1*color.b) / 0.003f); //radius based on light intensity and light luminance

	boundingBox.applyTranslation(position);
	boundingBox.applyScale(glm::vec3(lightRadius, lightRadius, lightRadius));
}

void SpotLight::drawInInspector(Scene& scene)
{
	if (ImGui::SliderFloat("light intensity", &intensity, 0.f, 50.f))
		updateBoundingBox();
	if (ImGui::ColorEdit3("light color", &color[0]))
		updateBoundingBox();

	ImGui::SliderFloat("light angles", &angle, 0.f, glm::pi<float>());
	if (ImGui::RadioButton("cast shadows", castShadows))
		castShadows = !castShadows;
	if (ImGui::RadioButton("use flare", useFlare))
		useFlare = !useFlare;
	EditorGUI::ResourceField("Flare material", flareMaterial);
}

void SpotLight::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	float _intensity = intensity;
	if (ImGui::SliderFloat("light intensity", &_intensity, 0.f, 10.f))
	{
		for (auto component : components)
		{
			SpotLight* castedComponent = static_cast<SpotLight*>(component);
			castedComponent->intensity = _intensity;
			updateBoundingBox();
		}
	}
	glm::vec3 _color = color;
	if (ImGui::ColorEdit3("light color", &_color[0]))
	{
		for (auto component : components)
		{
			SpotLight* castedComponent = static_cast<SpotLight*>(component);
			castedComponent->color = _color;
			updateBoundingBox();
		}
	}
	float _angle = angle;
	if (ImGui::SliderFloat("light angles", &_angle, 0.f, glm::pi<float>()))
	{
		for (auto component : components)
		{
			SpotLight* castedComponent = static_cast<SpotLight*>(component);
			castedComponent->angle = _angle;
		}
	}
	if (ImGui::RadioButton("cast shadows", castShadows))
	{
		castShadows = !castShadows;
		for (auto component : components)
		{
			SpotLight* castedComponent = static_cast<SpotLight*>(component);
			castedComponent->castShadows = castShadows;
		}
	}
	if (ImGui::RadioButton("use Flare", useFlare))
	{
		useFlare = !useFlare;
		for (auto component : components)
		{
			SpotLight* castedComponent = static_cast<SpotLight*>(component);
			castedComponent->useFlare = useFlare;
		}
	}
	if (EditorGUI::ResourceField("Flare material", flareMaterial))
	{
		for (auto component : components)
		{
			SpotLight* castedComponent = static_cast<SpotLight*>(component);
			castedComponent->flareMaterial = flareMaterial;
		}
	}
}

void SpotLight::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	position = translation;

	glm::mat3 rotMat = glm::mat3_cast(rotation);
	up = glm::normalize(rotMat * glm::vec3(1, 0, 0));
	direction = glm::normalize(rotMat * glm::vec3(0, -1, 0));

	updateBoundingBox();
}

void SpotLight::setBoundingBoxVisual(ResourcePtr<Mesh> visualMesh, ResourcePtr<MaterialInstance> visualMaterial)
{
	boundingBox.setVisual(visualMesh, visualMaterial);
}

void SpotLight::renderBoundingBox(const glm::mat4& projectile, const glm::mat4& view, glm::vec3 color)
{
	boundingBox.render(projectile, view, color);
}

void SpotLight::save(Json::Value & rootComponent) const
{
	Light::save(rootComponent);

	rootComponent["up"] = toJsonValue(up);
	rootComponent["position"] = toJsonValue(position);
	rootComponent["direction"] = toJsonValue(direction);
	rootComponent["angle"] = angle;
	rootComponent["boundingBox"] = toJsonValue(boundingBox);
}

void SpotLight::load(const Json::Value & rootComponent)
{
	Light::load(rootComponent);

	up = fromJsonValue<glm::vec3>(rootComponent["up"], glm::vec3());
	position = fromJsonValue<glm::vec3>(rootComponent["position"], glm::vec3());
	direction = fromJsonValue<glm::vec3>(rootComponent["direction"], glm::vec3());
	angle = rootComponent.get("angle", glm::pi<float>() / 4.f).asFloat();
	boundingBox = fromJsonValue<BoxCollider>(rootComponent["boundingBox"], BoxCollider());
}
