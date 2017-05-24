#pragma once

#include <vector>
#include <memory>

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "Materials.h"
#include "Lights.h"

class RenderDatas;
struct BaseCamera;
class DebugDrawRenderer;

struct ShadowMap
{
	int textureWidth;
	int textureHeight;

	GLuint shadowFrameBuffer;
	GLuint shadowRenderBuffer;
	GLuint shadowTexture;

	ShadowMap(int _textureWidth = 1024, int _textureHeight = 1024);
	ShadowMap(ShadowMap&& other) noexcept;
	ShadowMap& operator=(ShadowMap&& other) noexcept;
	~ShadowMap();

	ShadowMap(const ShadowMap& other) = delete;
	ShadowMap& operator=(const ShadowMap& other) = delete;

	GLuint getTextureId()
	{
		return shadowTexture;
	}
};

struct OmniShadowMap // omnidirectional shadow map for point lights
{
	int textureWidth;
	int textureHeight;

	GLuint shadowFrameBuffer;
	GLuint shadowRenderBuffer;
	GLuint shadowTexture;

	OmniShadowMap(int _textureWidth = 1024, int _textureHeight = 1024);
	OmniShadowMap(OmniShadowMap&& other) noexcept;
	OmniShadowMap& operator=(OmniShadowMap&& other) noexcept;
	~OmniShadowMap();

	OmniShadowMap(const OmniShadowMap& other) = delete;
	OmniShadowMap& operator=(const OmniShadowMap& other) = delete;
};

class LightManager
{
public:
	enum LightType {POINT, DIRECTIONAL, SPOT};

private:

	std::shared_ptr<MaterialPointLight> m_pointLightMaterial;
	std::shared_ptr<MaterialDirectionalLight> m_directionalLightMaterial;
	std::shared_ptr<MaterialSpotLight> m_spotLightMaterial;

	//shadows : 
	std::vector<ShadowMap> spot_shadowMaps;
	std::vector<ShadowMap> directional_shadowMaps;
	std::vector<OmniShadowMap> point_shadowMaps;

	float directionalShadowMapViewportSize;
	float directionalShadowMapViewportNear;
	float directionalShadowMapViewportFar;

	// For shadows :
	std::shared_ptr<MaterialShadowPass> shadowPassMaterial;
	std::shared_ptr<MaterialShadowPassOmni> shadowPassOmniMaterial;

public:
	LightManager();

	void setShadowMapCount(LightType lightType, unsigned int count);
	size_t getShadowMapCount(LightType lightType);
	//bind shadow map and resize viewport to cover the right area on screen
	void bindShadowMapFBO(LightType lightType, int index);
	void unbindShadowMapFBO(LightType lightType);
	void bindShadowMapTexture(LightType lightType, int index);
	GLuint getShadowMapTextureId(LightType lightType, int index);

	void setLightingMaterials(std::shared_ptr<MaterialPointLight> pointLightMat, std::shared_ptr<MaterialDirectionalLight> directionalLightMat, std::shared_ptr<MaterialSpotLight> spotLightMat);
	void uniformPointLight(PointLight& light, const glm::mat4& view);
	void uniformDirectionalLight(DirectionalLight& light, const glm::mat4& view);
	void uniformSpotLight(SpotLight& light, const glm::mat4& view);

	float getDirectionalShadowMapViewportSize() const;
	float getDirectionalShadowMapViewportNear() const;
	float getDirectionalShadowMapViewportFar() const;

	ShadowMap& getDirectionalShadowMap(int i)
	{
		return directional_shadowMaps[i];
	}

	ShadowMap& getSpotShadowMap(int i)
	{
		return spot_shadowMaps[i];
	}

	void generateShadowMaps(const BaseCamera& camera, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer);

	//render a shadow on a shadow map
	void renderShadows(const glm::mat4& lightProjection, const glm::mat4& lightView, const IDrawable& drawable);

	//render a shadow on a shadow map
	void renderShadows(float farPlane, const glm::vec3 & lightPos, const std::vector<glm::mat4>& lightVPs, const IDrawable& drawable);
};

