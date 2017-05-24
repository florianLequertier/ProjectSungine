#pragma once

#include <vector>
#include "glew/glew.h"
#include "glm/common.hpp"
#include "IDrawable.h"
#include "MaterialConfig.h"

class RenderDatas;

class MaterialAggregation
{
public:
	virtual void initParameters(GLuint programID) = 0;
	virtual void pushParametersToGPU(const RenderDatas& renderDatas) const = 0;
};

class PerInstanceMaterialAggregation
{
public:
	virtual void initParameters(GLuint programID) = 0;
	virtual void pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const = 0;
};

class MaterialAggregationWithSkeleton : public PerInstanceMaterialAggregation
{
private:
	std::vector<GLuint> uniform_bonesTransform;
	GLuint uniform_useSkeleton;

public:
	virtual void initParameters(GLuint programID);
	virtual void pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const;

private:
	void setUniformBonesTransform(unsigned int idx, const glm::mat4& boneTransform) const;
	void setUniformUseSkeleton(bool useSkeleton) const;
};

class MaterialAggregationReflectivePlane : public PerInstanceMaterialAggregation
{
private:
	GLuint uniform_reflectionTexture;
	GLuint uniform_ModelMatrix;
	GLuint uniform_ViewMatrix;
	GLuint uniform_ProjectionMatrix;

public:
	virtual void initParameters(GLuint programID);
	virtual void pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const;

private:
	void setUniformReflectionTexture(int texUnitId) const;
	void setUniformModelMatrix(const glm::mat4& modelMatrix) const;
	void setUniformViewMatrix(const glm::mat4& viewMatrix) const;
	void setUniformProjectionMatrix(const glm::mat4& projectionMatrix) const;
};

class MaterialAggregationMesh : public PerInstanceMaterialAggregation
{
private:
	GLuint uniform_ModelMatrix;
	GLuint uniform_ViewMatrix;
	GLuint uniform_ProjectionMatrix;
	GLuint uniform_ClipPlane;

public:
	virtual void initParameters(GLuint programID);
	virtual void pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const;

private:
	void setUniformModelMatrix(const glm::mat4& modelMatrix) const;
	void setUniformViewMatrix(const glm::mat4& viewMatrix) const;
	void setUniformProjectionMatrix(const glm::mat4& projectionMatrix) const;
	void setUniformClipPlane(const glm::vec4& clipPlane) const;
};


class MaterialAggregationBillboard : public PerInstanceMaterialAggregation
{
private:
	GLuint uniform_MVP;
	GLuint uniform_Scale;
	GLuint uniform_Translation;
	GLuint uniform_Texture;
	GLuint uniform_CameraRight;
	GLuint uniform_CameraUp;
	GLuint uniform_Color;
	GLuint uniform_ClipPlane;

public:
	virtual void initParameters(GLuint programID);
	virtual void pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const;

private:
	void setUniformMVP(const glm::mat4& VP) const;
	void setUniformScale(const glm::vec2& scale) const;
	void setUniformTranslation(const glm::vec3& translation) const;
	void setUniformTexture(int texId) const;
	void setUniformCameraRight(const glm::vec3& camRight) const;
	void setUniformCameraUp(const glm::vec3& camUp) const;
	void setUniformColor(const glm::vec4& color) const;
	void setUniformClipPlane(const glm::vec4& clipPlane) const;
};


class MaterialAggregationParticles : public PerInstanceMaterialAggregation
{
private:
	GLuint m_uniformVP;
	GLuint m_uniformTexture;
	GLuint m_uniformCameraRight;
	GLuint m_uniformCameraUp;
	GLuint m_uniformClipPlane;

public:
	virtual void initParameters(GLuint programID);
	virtual void pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const;

private:
	void glUniform_VP(const glm::mat4& VP) const;
	void setUniformTexture(int texId) const;
	void setUniformCameraRight(const glm::vec3& camRight) const;
	void setUniformCameraUp(const glm::vec3& camUp) const;
	void setUniformClipPlane(const glm::vec4& clipPlane) const;
};

///////////////////////////////////////////////////////////////

class MaterialAggregationForward : public MaterialAggregation
{
protected:
	// Lighting
	GLuint uniform_PointLight_position[MAX_POINT_LIGHT_COUNT];
	GLuint uniform_PointLight_color[MAX_POINT_LIGHT_COUNT];
	GLuint uniform_PointLight_intensity[MAX_POINT_LIGHT_COUNT];

	GLuint uniform_SpotLight_position[MAX_SPOT_LIGHT_COUNT];
	GLuint uniform_SpotLight_direction[MAX_SPOT_LIGHT_COUNT];
	GLuint uniform_SpotLight_color[MAX_SPOT_LIGHT_COUNT];
	GLuint uniform_SpotLight_intensity[MAX_SPOT_LIGHT_COUNT];
	GLuint uniform_SpotLight_angle[MAX_SPOT_LIGHT_COUNT];

	GLuint uniform_DirectionalLight_direction[MAX_DIRECTIONAL_LIGHT_COUNT];
	GLuint uniform_DirectionalLight_color[MAX_DIRECTIONAL_LIGHT_COUNT];
	GLuint uniform_DirectionalLight_intensity[MAX_DIRECTIONAL_LIGHT_COUNT];

	GLuint uniform_PointLightCount;
	GLuint uniform_SpotLightCount;
	GLuint uniform_DirectionalLightCount;

	// Transform
	GLuint uniform_ScreenToView; // mat4

public:
	void initParameters(GLuint programID) override;
	void pushParametersToGPU(const RenderDatas& renderDatas) const override;

private:
	void setUniformPointLight(int index, const glm::vec3& position, const glm::vec3& color, float intensity) const;
	void setUniformSpotLight(int index, const glm::vec3& position, const glm::vec3& direction, float angle, const glm::vec3& color, float intensity) const;
	void setUniformDirectionalLight(int index, const glm::vec3& direction, const glm::vec3& color, float intensity) const;
	void setUniformPointLightCount(int count) const;
	void setUniformSpotLightCount(int count) const;
	void setUniformDirectionalLightCount(int count) const;
	void setUniformScreenToView(const glm::mat4& screenToView) const;

};
