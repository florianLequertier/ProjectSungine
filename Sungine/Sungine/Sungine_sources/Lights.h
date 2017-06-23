#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr
#include "glm/gtc/quaternion.hpp"

#include "Utils.h"

#include "Component.h"

#include "Collider.h"

//forward
class Entity;
class MaterialFlares;
struct PointLight;
struct SpotLight;
struct DirectionalLight;

struct PointLightRenderDatas
{
	glm::vec4 viewport;
	PointLight* light;
	GLuint shadowMapTextureId;

	inline PointLightRenderDatas(const glm::vec4& _viewport, PointLight* _light) : viewport(_viewport), light(_light), shadowMapTextureId(0) {}
};

struct SpotLightRenderDatas
{
	glm::vec4 viewport;
	SpotLight* light;
	GLuint shadowMapTextureId;

	inline SpotLightRenderDatas(const glm::vec4& _viewport, SpotLight* _light) : viewport(_viewport), light(_light), shadowMapTextureId(0) {}
};

struct DirectionalLightRenderDatas
{
	DirectionalLight* light;
	GLuint shadowMapTextureId;

	inline DirectionalLightRenderDatas(DirectionalLight* _light) : light(_light), shadowMapTextureId(0) {}
};


struct Light : public Component
{
	CLASS((Light, Component),
	((PUBLIC)
		// Base light parameters
		(float		, intensity					)
		(glm::vec3	, color						)
		(bool		, castShadows				)
		(bool		, useFlare					)
	)
	)

public:

	ResourcePtr<Material> flareMaterial;

	Light(float _intensity, glm::vec3 _color);
	virtual ~Light();

	float getIntensity() const;
	glm::vec3 getColor() const;
	bool getCastShadows() const;
	bool getUseFlare() const;
	const MaterialFlares* getFlareMaterial() const;
	void setIntensity(float i);
	void setColor(const glm::vec3& c);
	void setCastShadows(bool state);
	void setUseFlare(bool state);

	virtual void updateBoundingBox();

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;

};

REGISTER_CLASS(Light)

struct PointLight : public Light
{
	COMPONENT_IMPLEMENTATION_HEADER(PointLight)

	CLASS((PointLight, Light),
	((PUBLIC)
		// Point light parameters
		(glm::vec3		, intensity		)
		(BoxCollider	, boundingBox	)
	)
	)

public:

	PointLight(float _intensity = 10, glm::vec3 _color = glm::vec3(1, 1, 1), glm::vec3 _position = glm::vec3(0, 0, 0));
	virtual ~PointLight();

	virtual void updateBoundingBox() override;

	virtual void drawInInspector(Scene& scene) override;
	virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation  = glm::quat()) override;

	void setBoundingBoxVisual(ResourcePtr<Mesh> visualMesh, ResourcePtr<Material> visualMaterial);
	void renderBoundingBox(const glm::mat4& projectile, const glm::mat4& view, glm::vec3 color);

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;

};

REGISTER_CLASS(PointLight)

struct DirectionalLight : public Light
{
	COMPONENT_IMPLEMENTATION_HEADER(DirectionalLight)

	CLASS((DirectionalLight, Light),
	((PUBLIC)
		// Directional light parameters
		(glm::vec3		, up			)
		(glm::vec3		, direction		)
		(glm::vec3		, position		)
	)
	)

public:

	DirectionalLight(float _intensity = 0.2f, glm::vec3 _color = glm::vec3(1, 1, 1), glm::vec3 _direction = glm::vec3(0, -1, 0));
	virtual ~DirectionalLight();

	virtual void drawInInspector(Scene& scene) override;
	virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;
};

REGISTER_CLASS(DirectionalLight)

struct SpotLight : public Light
{
	COMPONENT_IMPLEMENTATION_HEADER(SpotLight)

	CLASS((SpotLight, Light),
	((PUBLIC)
		// Spot light parameters
		(glm::vec3		, up			)
		(glm::vec3		, direction		)
		(glm::vec3		, position		)
		(float			, angle			)
		(BoxCollider	, boundingBox	)
	)
	)

public:

	SpotLight(float _intensity = 10, glm::vec3 _color = glm::vec3(1, 1, 1), glm::vec3 _position = glm::vec3(0, 0, 0), glm::vec3 _direction = glm::vec3(0, -1, 0), float _angle = glm::radians(30.f));
	virtual ~SpotLight();

	virtual void updateBoundingBox() override;

	virtual void drawInInspector(Scene& scene) override;
	virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;

	void setBoundingBoxVisual(ResourcePtr<Mesh> visualMesh, ResourcePtr<Material> visualMaterial);
	void renderBoundingBox(const glm::mat4& projectile, const glm::mat4& view, glm::vec3 color);

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;
};

REGISTER_CLASS(SpotLight)
