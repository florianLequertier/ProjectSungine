#pragma once

#include <memory>
#include <unordered_map>

#include "glew/glew.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Utils.h"
#include "Texture.h"

#include "ISerializable.h"
//#include "ResourcePointer.h"
#include "ShaderParameters.h"
#include "ShaderProgram.h"
#include "FileHandler.h"
#include "Resource.h"
#include "MaterialHelper.h"
#include "MaterialAggregation.h"
#include "MaterialConfig.h"

class IRenderBatch;
struct RenderDatas;

//Materials :
class Material : public Resource, public ISerializable
{
protected:
	GLuint m_glProgramId;
	//Rendering::PipelineType m_pipelineType;
	std::vector<std::shared_ptr<InternalShaderParameterBase>> m_internalParameters;

	ResourcePtr<ShaderProgram> m_programPtr;
	//std::string m_glProgramName; //a key to found the program this material is based on.
	//const ShaderProgram* m_programPtr;

	std::unordered_map<std::string, std::shared_ptr<MaterialAggregation>> m_aggregations;
	std::unordered_map<std::string, std::shared_ptr<PerInstanceMaterialAggregation>> m_perInstanceAggregations;

public:
	Material();
	Material(const ShaderProgram& shaderProgram);
	Material(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath);
	virtual ~Material();

	//void changePipeline(Rendering::PipelineType pipelineType);
	//void addAggregation(const std::string& key, std::shared_ptr<MaterialAggregation> aggregation);
	//void removeAggregation(const std::string& key);
	//const std::shared_ptr<MaterialAggregation> getAggregation(const std::string& key) const;

	//init internal params. Should be called in constructor, or just after construction.
	void initInternalParameters();
	void init(const ShaderProgram& shaderProgram);

	virtual void init(const FileHandler::CompletePath& path, const ID& id) override;
	virtual void save() override;
	virtual void resolvePointersLoading() override;
	//void save(const FileHandler::CompletePath& path) const;
	// Save and load internal parameters 
	virtual void save(Json::Value & entityRoot) const override;
	virtual void load(const Json::Value & entityRoot) override;

	void setAggregates(std::unordered_map<std::string, std::shared_ptr<MaterialAggregation>>& aggregations);
	void setPerInstanceAggregates(std::unordered_map<std::string, std::shared_ptr<PerInstanceMaterialAggregation>>& perInstanceAggregations);
	void setProgramId(GLuint programId);
	void setinternalParameters(std::vector<std::shared_ptr<InternalShaderParameterBase>>& internalShaderParameters);

	void drawUI();
	void pushGlobalsToGPU(const RenderDatas& renderDatas) const;
	void pushInternalsToGPU(int& boundTextureCount) const;
	void pushExternalsToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const;
	virtual void setExternalParameters(/*const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters*/) { };
	void use() const;
	void use();
	GLuint getGLId() const;
	Rendering::PipelineType getPipelineType() const;
	template<typename T>
	T* getInternalData(const std::string& parameterName) const
	{
		auto& found = std::find_if(m_internalParameters.begin(), m_internalParameters.end(), [parameterName](const std::shared_ptr<InternalShaderParameterBase>& item) { return item->getName() == parameterName; });
		void* foundData = nullptr;
		if(found != m_internalParameters.end())
			(*found)->getData(foundData);
		return static_cast<T*>(foundData);
	}
	template<typename T>
	void setInternalData(const std::string& parameterName, const T* data)
	{
		auto& found = std::find_if(m_internalParameters.begin(), m_internalParameters.end(), [parameterName](const std::shared_ptr<InternalShaderParameterBase>& item) { return item->getName() == parameterName; });
		(*found)->setData(data);
	}

	void loadFromShaderProgramDatas(GLuint glProgramId, const std::vector<std::shared_ptr<InternalShaderParameterBase>>& internalParameters/*, std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters*/);

	virtual void drawInInspector(Scene & scene) override;

	virtual std::shared_ptr<IRenderBatch> makeSharedRenderBatch() const;
};

template<typename BatchMaterialType, typename ParentMaterialType>
class BatchableMaterial : public ParentMaterialType
{
public:
	BatchableMaterial() 
		: ParentMaterialType()
	{}

	BatchableMaterial(const ShaderProgram& shaderProgram) 
		: ParentMaterialType(shaderProgram)
	{}

	BatchableMaterial(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: ParentMaterialType(shaderProgram, completePath)
	{}

	std::shared_ptr<IRenderBatch> makeSharedRenderBatch() const override;
};

//engine materials : 

//Lightning materials : 

class MaterialLight : public Material
{
private:
	GLuint uniform_ColorTexture;
	GLuint uniform_NormalTexture;
	GLuint uniform_DepthTexture;
	GLuint uniform_SSAOTexture;
	GLuint uniform_ScreenToView;
	//GLuint uniform_CameraPosition; //No need in eye space
	GLuint uniform_ShadowTexture;
	GLuint uniform_ShadowFactor;

public:
	MaterialLight()
		: Material()
	{}

	MaterialLight(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialLight(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialLight()
	{}

	virtual void setExternalParameters() override
	{
		uniform_ColorTexture = MaterialHelper::getUniform(m_glProgramId, "ColorBuffer");
		uniform_NormalTexture = MaterialHelper::getUniform(m_glProgramId, "NormalBuffer");
		uniform_DepthTexture = MaterialHelper::getUniform(m_glProgramId, "DepthBuffer");
		uniform_SSAOTexture = MaterialHelper::getUniform(m_glProgramId, "SSAOTexture");
		uniform_ScreenToView = MaterialHelper::getUniform(m_glProgramId, "ScreenToView");
		//uniform_CameraPosition = MaterialHelper::getUniform(m_glProgramId, "CameraPosition");
		uniform_ShadowTexture = MaterialHelper::getUniform(m_glProgramId, "Shadow");
		uniform_ShadowFactor = MaterialHelper::getUniform(m_glProgramId, "ShadowFactor");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformColorTexture(int texUnitId)
	{
		GlHelper::pushParameterToGPU(uniform_ColorTexture, texUnitId);
	}
	void setUniformNormalTexture(int texUnitId)
	{
		GlHelper::pushParameterToGPU(uniform_NormalTexture, texUnitId);
	}
	void setUniformDepthTexture(int texUnitId)
	{
		GlHelper::pushParameterToGPU(uniform_DepthTexture, texUnitId);
	}
	void setUniformSSAOTexture(int texUnitId)
	{
		GlHelper::pushParameterToGPU(uniform_SSAOTexture, texUnitId);
	}
	void setUniformScreenToView(const glm::mat4& screenToWorldMat)
	{
		GlHelper::pushParameterToGPU(uniform_ScreenToView, screenToWorldMat);
	}
	//void setUniformCameraPosition(const glm::vec3& cameraPosition)
	//{
	//	GlHelper::pushParameterToGPU(uniform_CameraPosition, cameraPosition);
	//}
	void setUniformShadowTexture(int texUnitId)
	{
		GlHelper::pushParameterToGPU(uniform_ShadowTexture, texUnitId);
	}
	void setUniformShadowFactor(float factor)
	{
		GlHelper::pushParameterToGPU(uniform_ShadowFactor, factor);
	}
};

class MaterialPointLight final : public MaterialLight
{
private:
	GLuint uniform_Resize;
	GLuint uniform_ViewToWorld;
	GLuint uniform_FarPlane;
	GLuint uniform_lightPosition;
	GLuint uniform_lightColor;
	GLuint uniform_lightIntensity;

public:
	MaterialPointLight()
		: MaterialLight()
	{}

	MaterialPointLight(const ShaderProgram& shaderProgram)
		: MaterialLight(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialPointLight(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialLight(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialPointLight()
	{}

	void setExternalParameters() override
	{
		uniform_Resize = MaterialHelper::getUniform(m_glProgramId, "Resize");
		uniform_ViewToWorld = MaterialHelper::getUniform(m_glProgramId, "ViewToWorld");
		uniform_FarPlane = MaterialHelper::getUniform(m_glProgramId, "FarPlane");
		uniform_lightPosition = MaterialHelper::getUniform(m_glProgramId, "pointLight.position");
		uniform_lightColor = MaterialHelper::getUniform(m_glProgramId, "pointLight.color");
		uniform_lightIntensity = MaterialHelper::getUniform(m_glProgramId, "pointLight.intensity");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformResize(const glm::vec2& resize)
	{
		GlHelper::pushParameterToGPU(uniform_Resize, resize);
	}

	void setUniformViewToWorld(const glm::mat4& ViewToWorld)
	{
		GlHelper::pushParameterToGPU(uniform_ViewToWorld, ViewToWorld);
	}

	void setUniformFarPlane(float farPlane)
	{
		GlHelper::pushParameterToGPU(uniform_FarPlane, farPlane);
	}

	void setUniformLightPosition(const glm::vec3& lightPos)
	{
		GlHelper::pushParameterToGPU(uniform_lightPosition, lightPos);
	}
	void setUniformLightColor(const glm::vec3& lightCol)
	{
		GlHelper::pushParameterToGPU(uniform_lightColor, lightCol);
	}
	void setUniformLightIntensity(float lightIntensity)
	{
		GlHelper::pushParameterToGPU(uniform_lightIntensity, lightIntensity);
	}
};

class MaterialDirectionalLight final : public MaterialLight
{
private:
	GLuint uniform_Resize;
	GLuint uniform_ViewToLight;
	GLuint uniform_lightDirection;
	GLuint uniform_lightColor;
	GLuint uniform_lightIntensity;

public:
	MaterialDirectionalLight()
		: MaterialLight()
	{}

	MaterialDirectionalLight(const ShaderProgram& shaderProgram)
		: MaterialLight(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialDirectionalLight(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialLight(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialDirectionalLight()
	{}

	void setExternalParameters() override
	{
		uniform_Resize = MaterialHelper::getUniform(m_glProgramId, "Resize");
		uniform_ViewToLight = MaterialHelper::getUniform(m_glProgramId, "ViewToLightScreen");
		uniform_lightDirection = MaterialHelper::getUniform(m_glProgramId, "directionalLight.direction");
		uniform_lightColor = MaterialHelper::getUniform(m_glProgramId, "directionalLight.color");
		uniform_lightIntensity = MaterialHelper::getUniform(m_glProgramId, "directionalLight.intensity");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformResize(const glm::vec2& resize)
	{
		GlHelper::pushParameterToGPU(uniform_Resize, resize);
	}

	void setUniformViewToLight(const glm::mat4& worldToLightMat)
	{
		GlHelper::pushParameterToGPU(uniform_ViewToLight, worldToLightMat);
	}

	void setUniformLightDirection(const glm::vec3& lightDir)
	{
		GlHelper::pushParameterToGPU(uniform_lightDirection, lightDir);
	}
	void setUniformLightColor(const glm::vec3& lightCol)
	{
		GlHelper::pushParameterToGPU(uniform_lightColor, lightCol);
	}
	void setUniformLightIntensity(float lightIntensity)
	{
		GlHelper::pushParameterToGPU(uniform_lightIntensity, lightIntensity);
	}
};

class MaterialSpotLight final : public MaterialLight
{
private:

	GLuint uniform_Resize;
	GLuint uniform_ViewToLight;
	GLuint uniform_lightDirection;
	GLuint uniform_lightPosition;
	GLuint uniform_lightAngle;
	GLuint uniform_lightColor;
	GLuint uniform_lightIntensity;

public:
	MaterialSpotLight()
		: MaterialLight()
	{}

	MaterialSpotLight(const ShaderProgram& shaderProgram)
		: MaterialLight(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialSpotLight(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialLight(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialSpotLight()
	{}

	void setExternalParameters() override
	{
		uniform_Resize = MaterialHelper::getUniform(m_glProgramId, "Resize");
		uniform_ViewToLight = MaterialHelper::getUniform(m_glProgramId, "ViewToLightScreen");
		uniform_lightDirection = MaterialHelper::getUniform(m_glProgramId, "spotLight.direction");
		uniform_lightAngle = MaterialHelper::getUniform(m_glProgramId, "spotLight.angle");
		uniform_lightPosition = MaterialHelper::getUniform(m_glProgramId, "spotLight.position");
		uniform_lightColor = MaterialHelper::getUniform(m_glProgramId, "spotLight.color");
		uniform_lightIntensity = MaterialHelper::getUniform(m_glProgramId, "spotLight.intensity");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformResize(const glm::vec2& resize)
	{
		GlHelper::pushParameterToGPU(uniform_Resize, resize);
	}

	void setUniformViewToLight(const glm::mat4& viewToLightMat)
	{
		GlHelper::pushParameterToGPU(uniform_ViewToLight, viewToLightMat);
	}

	void setUniformLightPosition(const glm::vec3& lightPos)
	{
		GlHelper::pushParameterToGPU(uniform_lightPosition, lightPos);
	}
	void setUniformLightDirection(const glm::vec3& lightDir)
	{
		GlHelper::pushParameterToGPU(uniform_lightDirection, lightDir);
	}
	void setUniformLightAngle(float lightAngle)
	{
		GlHelper::pushParameterToGPU(uniform_lightAngle, lightAngle);
	}
	void setUniformLightColor(const glm::vec3& lightCol)
	{
		GlHelper::pushParameterToGPU(uniform_lightColor, lightCol);
	}
	void setUniformLightIntensity(float lightIntensity)
	{
		GlHelper::pushParameterToGPU(uniform_lightIntensity, lightIntensity);
	}
};

//shadowing : 
class MaterialShadowPass final : public Material
{
private:

	GLuint uniform_MVP;

public:
	MaterialShadowPass()
		: Material()
	{}

	MaterialShadowPass(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialShadowPass(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialShadowPass()
	{}

	void setExternalParameters() override
	{
		uniform_MVP = MaterialHelper::getUniform(m_glProgramId, "MVP");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformMVP(const glm::mat4& mvp)
	{
		GlHelper::pushParameterToGPU(uniform_MVP, mvp);
	}
};


class MaterialShadowPassOmni final : public Material
{
private:

	GLuint uniform_ModelMatrix;
	GLuint uniform_VPLight[6];
	GLuint uniform_LightPos;
	GLuint uniform_FarPlane;

public:
	MaterialShadowPassOmni()
		: Material()
	{}

	MaterialShadowPassOmni(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialShadowPassOmni(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialShadowPassOmni()
	{}

	void setExternalParameters() override
	{
		uniform_ModelMatrix = MaterialHelper::getUniform(m_glProgramId, "ModelMatrix");
		for(int i = 0; i < 6; i++)
			uniform_VPLight[i] = MaterialHelper::getUniform(m_glProgramId, "VPLight[" + std::to_string(i) + "]");
		uniform_LightPos = MaterialHelper::getUniform(m_glProgramId, "LightPos");
		uniform_FarPlane = MaterialHelper::getUniform(m_glProgramId, "FarPlane");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformModelMatrix(const glm::mat4& modelMatrix)
	{
		GlHelper::pushParameterToGPU(uniform_ModelMatrix, modelMatrix);
	}

	void setUniformVPLight(const glm::mat4& vpLight, int index)
	{
		assert(index < 6);
		GlHelper::pushParameterToGPU(uniform_VPLight[index], vpLight);
	}

	void setUniformLightPos(const glm::vec3& lightPos)
	{
		GlHelper::pushParameterToGPU(uniform_LightPos, lightPos);
	}

	void setUniformFarPlane(float farPlane)
	{
		GlHelper::pushParameterToGPU(uniform_FarPlane, farPlane);
	}
};


//blit material : 

//Default materials :
class MaterialBlit final : public Material
{
private:
	GLuint uniform_TextureBlit;

public:
	MaterialBlit()
		: Material()
	{}

	MaterialBlit(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialBlit(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialBlit()
	{}

	void setExternalParameters() override
	{
		uniform_TextureBlit = MaterialHelper::getUniform(m_glProgramId, "Texture");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformBlitTexture(int texUnitId)
	{
		GlHelper::pushParameterToGPU(uniform_TextureBlit, texUnitId);
	}
};


class MaterialResizedBlit final : public Material
{
private:
	GLuint uniform_TextureBlit;
	GLuint uniform_Resize;

public:
	MaterialResizedBlit()
		: Material()
	{}

	MaterialResizedBlit(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialResizedBlit(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialResizedBlit()
	{}

	void setExternalParameters() override
	{
		uniform_TextureBlit = MaterialHelper::getUniform(m_glProgramId, "Texture");
		uniform_Resize = MaterialHelper::getUniform(m_glProgramId, "Resize");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformBlitTexture(int texUnitId)
	{
		GlHelper::pushParameterToGPU(uniform_TextureBlit, texUnitId);
	}

	void setUniformResize(const glm::vec2& resize)
	{
		GlHelper::pushParameterToGPU(uniform_Resize, resize);
	}
};


//Default materials :
class Material3DObject : public BatchableMaterial<Material3DObject, Material> //public Material
{
private:
	GLuint uniform_MVP;
	GLuint uniform_normalMatrix;
	GLuint uniform_ModelMatrix;
	GLuint uniform_ViewMatrix;
	GLuint uniform_ProjectionMatrix;
	std::vector<GLuint> uniform_bonesTransform;
	GLuint uniform_useSkeleton;

public:
	Material3DObject()
		: BatchableMaterial<Material3DObject, Material>()
	{}

	Material3DObject(const ShaderProgram& shaderProgram)
		: BatchableMaterial<Material3DObject, Material>(shaderProgram)
	{
		setExternalParameters();
	}

	Material3DObject(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: BatchableMaterial<Material3DObject, Material>(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~Material3DObject()
	{}

	void setExternalParameters() override
	{
		uniform_MVP = MaterialHelper::getUniform(m_glProgramId, "MVP");
		uniform_normalMatrix = MaterialHelper::getUniform(m_glProgramId, "NormalMatrix");
		uniform_ModelMatrix = MaterialHelper::getUniform(m_glProgramId, "ModelMatrix");
		uniform_ViewMatrix = MaterialHelper::getUniform(m_glProgramId, "ViewMatrix");
		uniform_ProjectionMatrix = MaterialHelper::getUniform(m_glProgramId, "ProjectionMatrix");
		uniform_bonesTransform = MaterialHelper::getUniforms(m_glProgramId, "BonesTransform", MAX_BONE_COUNT);
		uniform_useSkeleton = MaterialHelper::getUniform(m_glProgramId, "UseSkeleton");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniform_MVP(const glm::mat4& mvp) const
	{
		GlHelper::pushParameterToGPU(uniform_MVP, mvp);
	}
	void setUniform_normalMatrix(const glm::mat4& normalMatrix) const
	{
		GlHelper::pushParameterToGPU(uniform_normalMatrix, normalMatrix);
	}
	void setUniformModelMatrix(const glm::mat4& modelMatrix) const
	{
		GlHelper::pushParameterToGPU(uniform_ModelMatrix, modelMatrix);
	}
	void setUniformViewMatrix(const glm::mat4& viewMatrix) const
	{
		GlHelper::pushParameterToGPU(uniform_ViewMatrix, viewMatrix);
	}
	void setUniformProjectionMatrix(const glm::mat4& projectionMatrix) const
	{
		GlHelper::pushParameterToGPU(uniform_ProjectionMatrix, projectionMatrix);
	}
	void setUniformBonesTransform(unsigned int idx, const glm::mat4& boneTransform) const
	{
		GlHelper::pushParameterToGPU(uniform_bonesTransform[idx], boneTransform);
	}
	void setUniformUseSkeleton(bool useSkeleton) const
	{
		GlHelper::pushParameterToGPU(uniform_useSkeleton, useSkeleton);
	}
};

class MaterialLit final : public BatchableMaterial<MaterialLit, Material3DObject>
{
private:

public:
	MaterialLit()
		: BatchableMaterial<MaterialLit, Material3DObject>()
	{}

	MaterialLit(const ShaderProgram& shaderProgram)
		: BatchableMaterial<MaterialLit, Material3DObject>(shaderProgram)
	{

	}

	MaterialLit(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: BatchableMaterial<MaterialLit, Material3DObject>(shaderProgram, completePath)
	{

	}

};

class MaterialTransparent : public BatchableMaterial<MaterialTransparent, Material3DObject>
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
	
	//Internals
	// Material properties
	//GLuint uniform_Diffuse; //sampler2D
	//GLuint uniform_Specular; //sampler2D
	//GLuint uniform_Bump; //sampler2D
	//GLuint uniform_Emissive; //sampler2D
	//GLuint uniform_SpecularPower; //float
	//GLuint uniform_EmissiveMultiplier; //float
	//GLuint uniform_EmissiveColor; //vec3

public:
	MaterialTransparent()
		: BatchableMaterial<MaterialTransparent, Material3DObject>()
	{}

	MaterialTransparent(const ShaderProgram& shaderProgram)
		: BatchableMaterial<MaterialTransparent, Material3DObject>(shaderProgram)
	{
		setExternalParameters(/*shaderProgram.getExternalParameters()*/);
	}

	MaterialTransparent(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: BatchableMaterial<MaterialTransparent, Material3DObject>(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialTransparent()
	{}

	void setExternalParameters() override
	{

		for (int i = 0; i < MAX_POINT_LIGHT_COUNT; ++i)
		{
			uniform_PointLight_position[i] = MaterialHelper::getUniformStruct(m_glProgramId,  "PointLight", i, "position");
			uniform_PointLight_color[i] = MaterialHelper::getUniformStruct(m_glProgramId,     "PointLight", i, "color");
			uniform_PointLight_intensity[i] = MaterialHelper::getUniformStruct(m_glProgramId, "PointLight", i, "intensity");
		}

		for (int i = 0; i < MAX_SPOT_LIGHT_COUNT; ++i)
		{
			uniform_SpotLight_position[i] = MaterialHelper::getUniformStruct(m_glProgramId,  "SpotLight", i, "position");
			uniform_SpotLight_direction[i] = MaterialHelper::getUniformStruct(m_glProgramId, "SpotLight", i, "direction");
			uniform_SpotLight_color[i] = MaterialHelper::getUniformStruct(m_glProgramId,     "SpotLight", i, "color");
			uniform_SpotLight_intensity[i] = MaterialHelper::getUniformStruct(m_glProgramId, "SpotLight", i, "intensity");
			uniform_SpotLight_angle[i] = MaterialHelper::getUniformStruct(m_glProgramId,     "SpotLight", i, "angle");
		}

		for (int i = 0; i < MAX_DIRECTIONAL_LIGHT_COUNT; ++i)
		{
			uniform_DirectionalLight_direction[i] = MaterialHelper::getUniformStruct(m_glProgramId, "DirectionalLight", i, "direction");
			uniform_DirectionalLight_color[i] = MaterialHelper::getUniformStruct(m_glProgramId,     "DirectionalLight", i, "color");
			uniform_DirectionalLight_intensity[i] = MaterialHelper::getUniformStruct(m_glProgramId, "DirectionalLight", i, "intensity");
		}

		uniform_PointLightCount = MaterialHelper::getUniform(m_glProgramId, "PointLightCount");
		uniform_SpotLightCount = MaterialHelper::getUniform(m_glProgramId, "SpotLightCount");
		uniform_DirectionalLightCount = MaterialHelper::getUniform(m_glProgramId, "DirectionalLightCount");

		uniform_ScreenToView = MaterialHelper::getUniform(m_glProgramId, "ScreenToView");

		//uniform_Diffuse = MaterialHelper::getUniform(m_glProgramId, "Diffuse");
		//uniform_Specular = MaterialHelper::getUniform(m_glProgramId, "Specular");
		//uniform_Bump = MaterialHelper::getUniform(m_glProgramId, "Bump");
		//uniform_Emissive = MaterialHelper::getUniform(m_glProgramId, "ScreenToView");
		//uniform_SpecularPower = MaterialHelper::getUniform(m_glProgramId, "ScreenToView");
		//uniform_EmissiveMultiplier = MaterialHelper::getUniform(m_glProgramId, "ScreenToView");
		//uniform_EmissiveColor = MaterialHelper::getUniform(m_glProgramId, "ScreenToView");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformPointLight(int index, const glm::vec3& position, const glm::vec3& color, float intensity) const
	{
		assert(index < MAX_POINT_LIGHT_COUNT);
		GlHelper::pushParameterToGPU(uniform_PointLight_position[index], position);
		GlHelper::pushParameterToGPU(uniform_PointLight_color[index], color);
		GlHelper::pushParameterToGPU(uniform_PointLight_intensity[index], intensity);
	}
	void setUniformSpotLight(int index, const glm::vec3& position, const glm::vec3& direction, float angle, const glm::vec3& color, float intensity) const
	{
		assert(index < MAX_SPOT_LIGHT_COUNT);

		GlHelper::pushParameterToGPU(uniform_SpotLight_position[index], position);
		GlHelper::pushParameterToGPU(uniform_SpotLight_direction[index], direction);
		GlHelper::pushParameterToGPU(uniform_SpotLight_angle[index], angle);
		GlHelper::pushParameterToGPU(uniform_SpotLight_color[index], color);
		GlHelper::pushParameterToGPU(uniform_SpotLight_intensity[index], intensity);
	}
	void setUniformDirectionalLight(int index, const glm::vec3& direction, const glm::vec3& color, float intensity) const
	{
		assert(index < MAX_DIRECTIONAL_LIGHT_COUNT);

		GlHelper::pushParameterToGPU(uniform_DirectionalLight_direction[index], direction);
		GlHelper::pushParameterToGPU(uniform_DirectionalLight_color[index], color);
		GlHelper::pushParameterToGPU(uniform_DirectionalLight_intensity[index], intensity);
	}

	void setUniformPointLightCount(int count) const
	{
		GlHelper::pushParameterToGPU(uniform_PointLightCount, count);
	}
	void setUniformSpotLightCount(int count) const
	{
		GlHelper::pushParameterToGPU(uniform_SpotLightCount, count);
	}
	void setUniformDirectionalLightCount(int count) const
	{
		GlHelper::pushParameterToGPU(uniform_DirectionalLightCount, count);
	}

	void setUniformScreenToView(const glm::mat4& screenToView) const
	{
		GlHelper::pushParameterToGPU(uniform_ScreenToView, screenToView);
	}

};

class MaterialUnlit final : public BatchableMaterial<MaterialUnlit, Material3DObject>
{
	GLuint uniform_color;

public:
	MaterialUnlit()
		: BatchableMaterial<MaterialUnlit, Material3DObject>()
	{}

	MaterialUnlit(const ShaderProgram& shaderProgram)
		: BatchableMaterial<MaterialUnlit, Material3DObject>(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialUnlit(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: BatchableMaterial<MaterialUnlit, Material3DObject>(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters(/*const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters*/) override
	{
		uniform_color = MaterialHelper::getUniform(m_glProgramId, "Color");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniform_color(glm::vec3 color)
	{
		GlHelper::pushParameterToGPU<glm::vec3>(uniform_color, color);
	}
};

class MaterialInstancedUnlit final : public Material
{
private:
	GLuint uniform_VP;
	GLuint uniform_color;

public:
	MaterialInstancedUnlit()
		: Material()
	{}

	MaterialInstancedUnlit(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialInstancedUnlit(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_VP = MaterialHelper::getUniform(m_glProgramId, "VP");
		uniform_color = MaterialHelper::getUniform(m_glProgramId, "Color");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniform_color(const glm::vec3& color) const
	{
		GlHelper::pushParameterToGPU<glm::vec3>(uniform_color, color);

	}
	void setUniform_VP(const glm::mat4& VP) const
	{
		GlHelper::pushParameterToGPU<glm::mat4>(uniform_VP, VP);
	}
};

struct MaterialDebugDrawer final : public Material
{
private:
	GLuint uniform_MVP;

public:
	MaterialDebugDrawer()
		: Material()
	{}

	MaterialDebugDrawer(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialDebugDrawer(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_MVP = MaterialHelper::getUniform(m_glProgramId, "MVP");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniform_MVP(const glm::mat4& MVP) const
	{
		GlHelper::pushParameterToGPU<glm::mat4>(uniform_MVP, MVP);
	}
};


class MaterialSkybox final : public Material
{
private:
	GLuint uniform_VP;

public:
	MaterialSkybox()
		: Material()
	{}

	MaterialSkybox(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialSkybox(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_VP = MaterialHelper::getUniform(m_glProgramId, "VP");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniform_VP(const glm::mat4& VP)
	{
		GlHelper::pushParameterToGPU<glm::mat4>(uniform_VP, VP);
	}
};


class MaterialShadow : public Material
{

};


class MaterialBillboard final : public BatchableMaterial<MaterialBillboard, Material>
{
private:
	GLuint uniform_MVP;
	GLuint uniform_Scale;
	GLuint uniform_Translation;
	GLuint uniform_Texture;
	GLuint uniform_CameraRight;
	GLuint uniform_CameraUp;
	GLuint uniform_Color;

public:
	MaterialBillboard()
		: BatchableMaterial<MaterialBillboard, Material>()
	{}

	MaterialBillboard(const ShaderProgram& shaderProgram)
		: BatchableMaterial<MaterialBillboard, Material>(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialBillboard(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: BatchableMaterial<MaterialBillboard, Material>(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_MVP = MaterialHelper::getUniform(m_glProgramId, "MVP");
		uniform_Scale = MaterialHelper::getUniform(m_glProgramId, "Scale");
		uniform_Translation = MaterialHelper::getUniform(m_glProgramId, "Translation");
		uniform_Texture = MaterialHelper::getUniform(m_glProgramId, "Texture");
		uniform_CameraRight = MaterialHelper::getUniform(m_glProgramId, "CameraRight");
		uniform_CameraUp = MaterialHelper::getUniform(m_glProgramId, "CameraUp");
		uniform_Color = MaterialHelper::getUniform(m_glProgramId, "Color");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformMVP(const glm::mat4& VP) const
	{
		GlHelper::pushParameterToGPU<glm::mat4>(uniform_MVP, VP);
	}
	void setUniformScale(const glm::vec2& scale) const
	{
		GlHelper::pushParameterToGPU(uniform_Scale, scale);
	}
	void setUniformTranslation(const glm::vec3& translation) const
	{
		GlHelper::pushParameterToGPU(uniform_Translation, translation);
	}
	void setUniformTexture(int texId) const
	{
		GlHelper::pushParameterToGPU(uniform_Texture, texId);
	}
	void setUniformCameraRight(const glm::vec3& camRight) const
	{
		GlHelper::pushParameterToGPU(uniform_CameraRight, camRight);
	}
	void setUniformCameraUp(const glm::vec3& camUp) const
	{
		GlHelper::pushParameterToGPU(uniform_CameraUp, camUp);
	}
	void setUniformColor(const glm::vec4& color) const
	{
		GlHelper::pushParameterToGPU(uniform_Color, color);
	}
};


struct MaterialTerrain final : public Material3DObject
{
private:
	//internals

	GLuint uniform_SpecularPower;
	GLuint uniform_TextureRepetition;
	GLuint uniform_LayoutOffset;
	GLuint uniform_FilterTexture;

	GLuint uniform_DiffuseTexture;
	GLuint uniform_BumpTexture;
	GLuint uniform_SpecularTexture;


public:
	MaterialTerrain()
		: Material3DObject()
	{}

	MaterialTerrain(const ShaderProgram& shaderProgram)
		: Material3DObject(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialTerrain(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material3DObject(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_LayoutOffset = MaterialHelper::getUniform(m_glProgramId, "LayoutOffset");
		uniform_FilterTexture = MaterialHelper::getUniform(m_glProgramId, "FilterTexture");
		uniform_TextureRepetition = MaterialHelper::getUniform(m_glProgramId, "TextureRepetition");
		uniform_SpecularPower = MaterialHelper::getUniform(m_glProgramId, "SpecularPower");

		CHECK_GL_ERROR("error in material initialization.");

		//uniform_DiffuseTexture = MaterialHelper::getUniform(m_glProgramId, "DiffuseTexture");
		//uniform_BumpTexture = MaterialHelper::getUniform(m_glProgramId, "BumpTexture");
		//uniform_SpecularTexture = MaterialHelper::getUniform(m_glProgramId, "SpecularTexture");
	}

	void setUniformLayoutOffset(const glm::vec2& layoutOffset)
	{
		GlHelper::pushParameterToGPU(uniform_LayoutOffset, layoutOffset);
	}
	void setUniformFilterTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_FilterTexture, textureId);
	}
	void setUniformTextureRepetition(glm::vec2 repetition)
	{
		GlHelper::pushParameterToGPU(uniform_TextureRepetition, repetition);
	}
	void setUniformDiffuseTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_DiffuseTexture, textureId);
	}
	void setUniformBumpTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_BumpTexture, textureId);
	}
	void setUniformSpecularTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_SpecularTexture, textureId);
	}
	void setUniformSpecularPower(float specularPower)
	{
		GlHelper::pushParameterToGPU(uniform_SpecularPower, specularPower);
	}

};


class MaterialTerrainEdition final : public Material
{
private:

	GLuint uniform_textureDiffuse;
	GLuint uniform_textureSpecular;
	GLuint uniform_textureBump;

	GLuint uniform_textureRepetition;
	GLuint uniform_textureFilter;
	GLuint uniform_filterValues;

public:
	MaterialTerrainEdition()
		: Material()
	{}

	MaterialTerrainEdition(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialTerrainEdition(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_filterValues = MaterialHelper::getUniform(m_glProgramId, "FilterValues");
		uniform_textureFilter = MaterialHelper::getUniform(m_glProgramId, "FilterTexture");
		uniform_textureRepetition = MaterialHelper::getUniform(m_glProgramId, "TextureRepetition");

		uniform_textureDiffuse = MaterialHelper::getUniform(m_glProgramId, "Diffuse");
		uniform_textureSpecular = MaterialHelper::getUniform(m_glProgramId, "Specular");
		uniform_textureBump = MaterialHelper::getUniform(m_glProgramId, "Bump");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformLayoutOffset(const glm::vec2& layoutOffset)
	{
		GlHelper::pushParameterToGPU(uniform_filterValues, layoutOffset);
	}
	void setUniformFilterTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_textureFilter, textureId);
	}
	void setUniformTextureRepetition(glm::vec2 repetition)
	{
		GlHelper::pushParameterToGPU(uniform_textureRepetition, repetition);
	}
	void setUniformDiffuseTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_textureDiffuse, textureId);
	}
	void setUniformBumpTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_textureBump, textureId);
	}
	void setUniformSpecularTexture(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_textureSpecular, textureId);
	}

};

class MaterialDrawOnTexture final : public Material
{
private:
	GLuint uniform_drawPosition;
	GLuint uniform_colorToDraw;
	GLuint uniform_drawRadius;
	GLuint uniform_textureToDrawOn;

public:
	MaterialDrawOnTexture()
		: Material()
	{}

	MaterialDrawOnTexture(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialDrawOnTexture(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_drawPosition = MaterialHelper::getUniform(m_glProgramId, "DrawPosition");
		uniform_colorToDraw = MaterialHelper::getUniform(m_glProgramId, "DrawColor");
		uniform_drawRadius = MaterialHelper::getUniform(m_glProgramId, "DrawRadius");
		uniform_textureToDrawOn = MaterialHelper::getUniform(m_glProgramId, "Texture");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformDrawPosition(const glm::vec2& position)
	{
		GlHelper::pushParameterToGPU(uniform_drawPosition, position);
	}
	void setUniformColorToDraw(const glm::vec4& color)
	{
		GlHelper::pushParameterToGPU(uniform_colorToDraw, color);
	}
	void setUniformDrawRadius(float radius)
	{
		GlHelper::pushParameterToGPU(uniform_drawRadius, radius);
	}
	void setUniformTextureToDrawOn(int textureId)
	{
		GlHelper::pushParameterToGPU(uniform_textureToDrawOn, textureId);
	}
};

class MaterialGrassField final : public Material
{
	GLuint uniform_time;
	GLuint uniform_Texture;
	GLuint uniform_VP;


public:
	MaterialGrassField()
		: Material()
	{}

	MaterialGrassField(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialGrassField(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		uniform_time = MaterialHelper::getUniform(m_glProgramId, "Time");
		uniform_Texture = MaterialHelper::getUniform(m_glProgramId, "Texture");
		uniform_VP = MaterialHelper::getUniform(m_glProgramId, "VP");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void setUniformTime(float time)
	{
		GlHelper::pushParameterToGPU(uniform_time, time);
	}
	void setUniformTexture(int texId)
	{
		GlHelper::pushParameterToGPU(uniform_Texture, texId);
	}
	void setUniformVP(const glm::mat4& VP)
	{
		GlHelper::pushParameterToGPU(uniform_VP, VP);
	}
};

class MaterialParticlesCPU final : public BatchableMaterial<MaterialParticlesCPU, Material>
{
private:
	GLuint m_uniformVP;
	GLuint m_uniformTexture;
	GLuint m_uniformCameraRight;
	GLuint m_uniformCameraUp;

public:
	MaterialParticlesCPU()
		: BatchableMaterial<MaterialParticlesCPU, Material>()
	{}

	MaterialParticlesCPU(const ShaderProgram& shaderProgram)
		: BatchableMaterial<MaterialParticlesCPU, Material>(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialParticlesCPU(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: BatchableMaterial<MaterialParticlesCPU, Material>(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		m_uniformVP = MaterialHelper::getUniform(m_glProgramId, "VP");
		m_uniformTexture = MaterialHelper::getUniform(m_glProgramId, "Texture");
		m_uniformCameraRight = MaterialHelper::getUniform(m_glProgramId, "CameraRight");
		m_uniformCameraUp = MaterialHelper::getUniform(m_glProgramId, "CameraUp");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void glUniform_VP(const glm::mat4& VP) const
	{
		GlHelper::pushParameterToGPU(m_uniformVP, VP);
	}
	void setUniformTexture(int texId) const
	{
		GlHelper::pushParameterToGPU(m_uniformTexture, texId);
	}
	void setUniformCameraRight(const glm::vec3& camRight) const
	{
		GlHelper::pushParameterToGPU(m_uniformCameraRight, camRight);
	}
	void setUniformCameraUp(const glm::vec3& camUp) const
	{
		GlHelper::pushParameterToGPU(m_uniformCameraUp, camUp);
	}
};


class MaterialParticles final : public Material
{
private:
	GLuint m_uniformVP;
	GLuint m_uniformTexture;
	GLuint m_uniformCameraRight;
	GLuint m_uniformCameraUp;

public:
	MaterialParticles()
		: Material()
	{}

	MaterialParticles(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialParticles(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		m_uniformVP = MaterialHelper::getUniform(m_glProgramId, "VP");
		m_uniformTexture = MaterialHelper::getUniform(m_glProgramId, "Texture");
		m_uniformCameraRight = MaterialHelper::getUniform(m_glProgramId, "CameraRight");
		m_uniformCameraUp = MaterialHelper::getUniform(m_glProgramId, "CameraUp");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void glUniform_VP(const glm::mat4& VP)
	{
		GlHelper::pushParameterToGPU(m_uniformVP, VP);
	}
	void setUniformTexture(int texId)
	{
		GlHelper::pushParameterToGPU(m_uniformTexture, texId);
	}
	void setUniformCameraRight(const glm::vec3& camRight)
	{
		GlHelper::pushParameterToGPU(m_uniformCameraRight, camRight);
	}
	void setUniformCameraUp(const glm::vec3& camUp)
	{
		GlHelper::pushParameterToGPU(m_uniformCameraUp, camUp);
	}
};

class MaterialParticleSimulation final : public Material
{
private:
	GLuint m_uniformDeltaTime;

public:
	MaterialParticleSimulation()
		: Material()
	{}

	MaterialParticleSimulation(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialParticleSimulation(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		m_uniformDeltaTime = MaterialHelper::getUniform(m_glProgramId, "DeltaTime");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void glUniform_deltaTime(float deltaTime)
	{
		GlHelper::pushParameterToGPU(m_uniformDeltaTime, deltaTime);
	}
};

class MaterialSimple3DDraw final : public Material
{
private:
	GLuint m_uniformMVP;

public:
	MaterialSimple3DDraw()
		: Material()
	{}

	MaterialSimple3DDraw(const ShaderProgram& shaderProgram)
		: Material(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialSimple3DDraw(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: Material(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		m_uniformMVP = MaterialHelper::getUniform(m_glProgramId, "MVP");

		CHECK_GL_ERROR("error in material initialization.");
	}

	void glUniform_MVP(const glm::mat4& mvp)
	{
		GlHelper::pushParameterToGPU(m_uniformMVP, mvp);
	}
};

class MaterialReflection final : public BatchableMaterial<MaterialReflection, Material>
{
private:
	GLuint m_uniformMVP;
	GLuint m_uniformReflectionTexture;

	//std::unordered_map<std::string, GLuint> m_externals;

public:
	MaterialReflection()
		: BatchableMaterial<MaterialReflection, Material>()
	{}

	MaterialReflection(const ShaderProgram& shaderProgram)
		: BatchableMaterial<MaterialReflection, Material>(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialReflection(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath)
		: BatchableMaterial<MaterialReflection, Material>(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	void setExternalParameters() override
	{
		//m_externals["MVP"] = MaterialHelper::getUniform(m_glProgramId, "MVP");

		m_uniformMVP = MaterialHelper::getUniform(m_glProgramId, "MVP");
		m_uniformReflectionTexture = MaterialHelper::getUniform(m_glProgramId, "ReflectionTexture");

		CHECK_GL_ERROR("error in material initialization.");
	}

	//template<typename T>
	//void pushExternalUniform(std::string name, const T& value)
	//{
	//	GlHelper::pushParameterToGPU(m_externals[name], value);
	//}

	void glUniform_MVP(const glm::mat4& mvp) const
	{
		GlHelper::pushParameterToGPU(m_uniformMVP, mvp);
	}

	void glUniform_ReflectionTexture(int texId) const
	{
		GlHelper::pushParameterToGPU(m_uniformReflectionTexture, texId);
	}
};