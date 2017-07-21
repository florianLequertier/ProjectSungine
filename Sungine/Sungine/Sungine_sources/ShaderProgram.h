#pragma once

class ShaderProgram
{

};


class MaterialLight : public MaterialInstance
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
		: MaterialInstance()
	{}

	MaterialLight(const AssetHandle<Material>& shaderProgram)
		: MaterialInstance(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialLight(const AssetHandle<Material>& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialInstance(shaderProgram, completePath)
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

	MaterialPointLight(const Material& shaderProgram)
		: MaterialLight(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialPointLight(const Material& shaderProgram, const FileHandler::CompletePath& completePath)
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

	MaterialDirectionalLight(const Material& shaderProgram)
		: MaterialLight(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialDirectionalLight(const Material& shaderProgram, const FileHandler::CompletePath& completePath)
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

	MaterialSpotLight(const Material& shaderProgram)
		: MaterialLight(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialSpotLight(const Material& shaderProgram, const FileHandler::CompletePath& completePath)
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
class MaterialShadowPass final : public MaterialInstance
{
private:

	GLuint uniform_MVP;

public:
	MaterialShadowPass()
		: MaterialInstance()
	{}

	MaterialShadowPass(const AssetHandle<Material>& shaderProgram)
		: MaterialInstance(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialShadowPass(const AssetHandle<Material>& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialInstance(shaderProgram, completePath)
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


class MaterialShadowPassOmni final : public MaterialInstance
{
private:

	GLuint uniform_ModelMatrix;
	GLuint uniform_VPLight[6];
	GLuint uniform_LightPos;
	GLuint uniform_FarPlane;

public:
	MaterialShadowPassOmni()
		: MaterialInstance()
	{}

	MaterialShadowPassOmni(const Material& shaderProgram)
		: MaterialInstance(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialShadowPassOmni(const Material& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialInstance(shaderProgram, completePath)
	{
		setExternalParameters();
	}

	virtual ~MaterialShadowPassOmni()
	{}

	void setExternalParameters() override
	{
		uniform_ModelMatrix = MaterialHelper::getUniform(m_glProgramId, "ModelMatrix");
		for (int i = 0; i < 6; i++)
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
class MaterialBlit final : public MaterialInstance
{
private:
	GLuint uniform_TextureBlit;

public:
	MaterialBlit()
		: MaterialInstance()
	{}

	MaterialBlit(const Material& shaderProgram)
		: MaterialInstance(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialBlit(const Material& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialInstance(shaderProgram, completePath)
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


class MaterialResizedBlit final : public MaterialInstance
{
private:
	GLuint uniform_TextureBlit;
	GLuint uniform_Resize;

public:
	MaterialResizedBlit()
		: MaterialInstance()
	{}

	MaterialResizedBlit(const Material& shaderProgram)
		: MaterialInstance(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialResizedBlit(const Material& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialInstance(shaderProgram, completePath)
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


struct MaterialDebugDrawer final : public MaterialInstance
{
private:
	GLuint uniform_MVP;

public:
	MaterialDebugDrawer()
		: MaterialInstance()
	{}

	MaterialDebugDrawer(const Material& shaderProgram)
		: MaterialInstance(shaderProgram)
	{
		setExternalParameters();
	}

	MaterialDebugDrawer(const Material& shaderProgram, const FileHandler::CompletePath& completePath)
		: MaterialInstance(shaderProgram, completePath)
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




