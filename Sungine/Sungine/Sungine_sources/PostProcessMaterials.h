#pragma once

#include "Material.h"
#include "MaterialInstance.h"

#include "Project.h"

// Register both operation and operation data into there factories : 
#define REGISTER_POST_PROCESS_MATERIAL(MaterialType, MaterialName, MaterialPath)\
	static bool isRegistered##MaterialType = PostProcessMaterialsFactory::instance().registerMaterialTemplate<MaterialType>(MaterialName, MaterialPath);


// The factory which stores the templates of all internal post process materials
class PostProcessMaterialsFactory : public ISingleton<PostProcessMaterialsFactory>
{
	SINGLETON_IMPL(PostProcessMaterialsFactory)

private:
	std::unordered_map<std::string, std::shared_ptr<Material>> m_templates;

public:

	PostProcessMaterialsFactory()
	{}

	template<typename MaterialType>
	bool registerMaterialTemplate(const std::string& materialName, const FileHandler::CompletePath& materialPath)
	{
		m_templates[materialName] = std::make_shared<MaterialType>(materialPath, Rendering::MaterialType::INTERNAL);
		return true;
	}

	std::shared_ptr<Material> instantiateMaterial(const std::string& materialName)
	{
		auto found = m_templates.find(materialName);
		if (found != m_templates.end())
		{
			return found->second->cloneShared();
		}
		else
		{
			std::cout << "you are trying to instantiate a material with the wrong name. Name : " << materialName << std::endl;
		}

		return m_templates[materialName]->cloneShared();
	}

	std::shared_ptr<Material> getRef(const std::string& materialName)
	{
		auto found = m_templates.find(materialName);
		if (found != m_templates.end())
		{
			return found->second->cloneShared();
		}
		else
		{
			std::cout << "you are trying to get a ref to a material with the wrong name. Name : " << materialName << std::endl;
		}

		return m_templates[materialName];
	}

	int getTemplateCount() const
	{
		return m_templates.size();
	}

	typename std::unordered_map<std::string, std::shared_ptr<Material>>::const_iterator getTemplateIteratorBegin() const
	{
		return m_templates.begin();
	}

	typename std::unordered_map<std::string, std::shared_ptr<Material>>::const_iterator getTemplateIteratorEnd() const
	{
		return m_templates.end();
	}
};

class MaterialBlur : public MaterialInstance
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformPassId;
	GLuint m_uniformTexture;

public:
	MaterialBlur();
	MaterialBlur(const Material& shaderProgram);
	MaterialBlur(const Material& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_passId(int passId);
	void glUniform_Texture(int textureId);
	void glUniform_Resize(const glm::vec2& resize);
};

REGISTER_POST_PROCESS_MATERIAL(MaterialBlur, "blur", FileHandler::CompletePath(Project::getShaderFolderPath().toString() + "blur/blur.glProg"))

class MaterialBloom : public MaterialInstance
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformExposure;
	GLuint m_uniformGamma;
	GLuint m_uniformTexture;
	GLuint m_uniformTextureBlur;

public:
	MaterialBloom();
	MaterialBloom(const Material& shaderProgram);
	MaterialBloom(const Material& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Texture(int textureId);
	void glUniform_TextureBlur(int textureId);
	void glUniform_Exposure(float exposure);
	void glUniform_Gamma(float gamma);
	void glUniform_Resize(const glm::vec2& resize);
};

REGISTER_POST_PROCESS_MATERIAL(MaterialBloom, "bloom", FileHandler::CompletePath(Project::getShaderFolderPath().toString() + "bloom/bloom.glProg"))

class MaterialAdd : public MaterialInstance
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformTexture01;
	GLuint m_uniformTexture02;

public:
	MaterialAdd();
	MaterialAdd(const Material& shaderProgram);
	MaterialAdd(const Material& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Texture01(int textureId);
	void glUniform_Texture02(int textureId);
	void glUniform_Resize(const glm::vec2& resize);
};

REGISTER_POST_PROCESS_MATERIAL(MaterialAdd, "add", FileHandler::CompletePath(Project::getShaderFolderPath().toString() + "add/add.glProg"))

class MaterialFlares : public MaterialInstance
{
private:
	GLuint m_uniformVP;
	GLuint m_uniformDepth;

public:
	MaterialFlares();
	MaterialFlares(const Material& shaderProgram);
	MaterialFlares(const Material& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_VP(const glm::mat4& VP) const;
	void glUniform_Depth(int textureId) const;
};

REGISTER_POST_PROCESS_MATERIAL(MaterialFlares, "flares", FileHandler::CompletePath(Project::getShaderFolderPath().toString() + "flares/flares.glProg"))

class MaterialSSAO : public MaterialInstance
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformNormals;
	GLuint m_uniformNoiseTexture;
	GLuint m_uniformKernel;
	GLuint m_uniformDepth;
	GLuint m_uniformProjection;
	GLuint m_uniformScreenToView;


public:
	MaterialSSAO();
	MaterialSSAO(const Material& shaderProgram);
	MaterialSSAO(const Material& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Normals(int texId) const;
	void glUniform_NoiseTexture(int texId) const;
	void glUniform_Kernel(const std::vector<glm::vec3>& kernel) const;
	void glUniform_Depth(int texId) const;
	void glUniform_Projection(const glm::mat4& projection) const;
	void glUniform_ScreenToView(const glm::mat4& screenToView) const;
	void glUniform_Resize(const glm::vec2& resize) const;
};

REGISTER_POST_PROCESS_MATERIAL(MaterialSSAO, "ssao", FileHandler::CompletePath(Project::getShaderFolderPath().toString() + "ssao/ssao.glProg"))

class MaterialSSAOBlur : public MaterialInstance
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformTexture;

public:
	MaterialSSAOBlur();
	MaterialSSAOBlur(const Material& shaderProgram);
	MaterialSSAOBlur(const Material& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Texture(int texId) const;
	void glUniform_Resize(const glm::vec2& resize) const;
};

REGISTER_POST_PROCESS_MATERIAL(MaterialSSAOBlur, "ssaoBlur", FileHandler::CompletePath(Project::getShaderFolderPath().toString() + "ssaoBlur/ssaoBlur.glProg"))
