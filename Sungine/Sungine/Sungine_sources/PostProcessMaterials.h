#pragma once

#include "Materials.h"

class MaterialBlur : public Material
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformPassId;
	GLuint m_uniformTexture;

public:
	MaterialBlur();
	MaterialBlur(const ShaderProgram& shaderProgram);
	MaterialBlur(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_passId(int passId);
	void glUniform_Texture(int textureId);
	void glUniform_Resize(const glm::vec2& resize);
};

class MaterialBloom : public Material
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformExposure;
	GLuint m_uniformGamma;
	GLuint m_uniformTexture;
	GLuint m_uniformTextureBlur;

public:
	MaterialBloom();
	MaterialBloom(const ShaderProgram& shaderProgram);
	MaterialBloom(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Texture(int textureId);
	void glUniform_TextureBlur(int textureId);
	void glUniform_Exposure(float exposure);
	void glUniform_Gamma(float gamma);
	void glUniform_Resize(const glm::vec2& resize);
};

class MaterialAdd : public Material
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformTexture01;
	GLuint m_uniformTexture02;

public:
	MaterialAdd();
	MaterialAdd(const ShaderProgram& shaderProgram);
	MaterialAdd(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Texture01(int textureId);
	void glUniform_Texture02(int textureId);
	void glUniform_Resize(const glm::vec2& resize);
};

class MaterialFlares : public Material
{
private:
	GLuint m_uniformVP;
	GLuint m_uniformDepth;

public:
	MaterialFlares();
	MaterialFlares(const ShaderProgram& shaderProgram);
	MaterialFlares(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_VP(const glm::mat4& VP) const;
	void glUniform_Depth(int textureId) const;
};

class MaterialSSAO : public Material
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
	MaterialSSAO(const ShaderProgram& shaderProgram);
	MaterialSSAO(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Normals(int texId) const;
	void glUniform_NoiseTexture(int texId) const;
	void glUniform_Kernel(const std::vector<glm::vec3>& kernel) const;
	void glUniform_Depth(int texId) const;
	void glUniform_Projection(const glm::mat4& projection) const;
	void glUniform_ScreenToView(const glm::mat4& screenToView) const;
	void glUniform_Resize(const glm::vec2& resize) const;
};

class MaterialSSAOBlur : public Material
{
private:
	GLuint m_uniformResize;
	GLuint m_uniformTexture;

public:
	MaterialSSAOBlur();
	MaterialSSAOBlur(const ShaderProgram& shaderProgram);
	MaterialSSAOBlur(const ShaderProgram& shaderProgram, const FileHandler::CompletePath& completePath);
	void setExternalParameters() override;
	void glUniform_Texture(int texId) const;
	void glUniform_Resize(const glm::vec2& resize) const;
};