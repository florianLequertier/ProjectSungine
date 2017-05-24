#include "PostProcessMaterials.h"

MaterialBlur::MaterialBlur()
	: Material()
{}

MaterialBlur::MaterialBlur(const ShaderProgram& shaderProgram)
	: Material(shaderProgram)
{
	setExternalParameters();
}

MaterialBlur::MaterialBlur(const ShaderProgram & shaderProgram, const FileHandler::CompletePath & completePath)
	: Material(shaderProgram, completePath)
{
	setExternalParameters();
}

void MaterialBlur::setExternalParameters()
{
	m_uniformResize = MaterialHelper::getUniform(m_glProgramId, "Resize");
	m_uniformPassId = MaterialHelper::getUniform(m_glProgramId, "PassId");
	m_uniformTexture = MaterialHelper::getUniform(m_glProgramId, "Texture");

	CHECK_GL_ERROR("error in texture initialization.");
}

void MaterialBlur::glUniform_passId(int passId)
{
	GlHelper::pushParameterToGPU(m_uniformPassId, passId);
}

void MaterialBlur::glUniform_Texture(int textureId)
{
	GlHelper::pushParameterToGPU(m_uniformTexture, textureId);
}

void MaterialBlur::glUniform_Resize(const glm::vec2 & resize)
{
	GlHelper::pushParameterToGPU(m_uniformResize, resize);
}

///////////////////////////////

MaterialBloom::MaterialBloom()
	: Material()
{

}

MaterialBloom::MaterialBloom(const ShaderProgram& shaderProgram)
	: Material(shaderProgram)
{
	setExternalParameters();
}

MaterialBloom::MaterialBloom(const ShaderProgram & shaderProgram, const FileHandler::CompletePath & completePath)
	: Material(shaderProgram, completePath)
{
	setExternalParameters();
}

void MaterialBloom::setExternalParameters()
{
	m_uniformResize = MaterialHelper::getUniform(m_glProgramId, "Resize");
	m_uniformExposure = MaterialHelper::getUniform(m_glProgramId, "Exposure");
	m_uniformGamma = MaterialHelper::getUniform(m_glProgramId, "Gamma");
	m_uniformTexture = MaterialHelper::getUniform(m_glProgramId, "Texture");
	m_uniformTextureBlur = MaterialHelper::getUniform(m_glProgramId, "TextureBlur");

	CHECK_GL_ERROR("error in texture initialization.");
}

void MaterialBloom::glUniform_Texture(int textureId)
{
	GlHelper::pushParameterToGPU(m_uniformTexture, textureId);
}

void MaterialBloom::glUniform_TextureBlur(int textureId)
{
	GlHelper::pushParameterToGPU(m_uniformTextureBlur, textureId);
}

void MaterialBloom::glUniform_Exposure(float exposure)
{
	GlHelper::pushParameterToGPU(m_uniformExposure, exposure);
}

void MaterialBloom::glUniform_Gamma(float gamma)
{
	GlHelper::pushParameterToGPU(m_uniformGamma, gamma);
}

void MaterialBloom::glUniform_Resize(const glm::vec2 & resize)
{
	GlHelper::pushParameterToGPU(m_uniformResize, resize);
}

/////////////////////////////

MaterialAdd::MaterialAdd()
	: Material()
{
}

MaterialAdd::MaterialAdd(const ShaderProgram & shaderProgram)
	: Material(shaderProgram)
{
	setExternalParameters();
}

MaterialAdd::MaterialAdd(const ShaderProgram & shaderProgram, const FileHandler::CompletePath & completePath)
	: Material(shaderProgram, completePath)
{
	setExternalParameters();
}

void MaterialAdd::setExternalParameters()
{
	m_uniformResize = MaterialHelper::getUniform(m_glProgramId, "Resize");
	m_uniformTexture01 = MaterialHelper::getUniform(m_glProgramId, "Texture01");
	m_uniformTexture02 = MaterialHelper::getUniform(m_glProgramId, "Texture02");

	CHECK_GL_ERROR("error in texture initialization.");
}

void MaterialAdd::glUniform_Texture01(int textureId)
{
	GlHelper::pushParameterToGPU(m_uniformTexture01, textureId);
}

void MaterialAdd::glUniform_Texture02(int textureId)
{
	GlHelper::pushParameterToGPU(m_uniformTexture02, textureId);
}

void MaterialAdd::glUniform_Resize(const glm::vec2 & resize)
{
	GlHelper::pushParameterToGPU(m_uniformResize, resize);
}

///////////////////////////

MaterialFlares::MaterialFlares()
	: Material()
{
}

MaterialFlares::MaterialFlares(const ShaderProgram & shaderProgram)
	: Material(shaderProgram)
{
	setExternalParameters();
}

MaterialFlares::MaterialFlares(const ShaderProgram & shaderProgram, const FileHandler::CompletePath & completePath)
	: Material(shaderProgram, completePath)
{
	setExternalParameters();
}

void MaterialFlares::setExternalParameters()
{
	m_uniformVP = MaterialHelper::getUniform(m_glProgramId, "VP");
	m_uniformDepth = MaterialHelper::getUniform(m_glProgramId, "Depth");

	CHECK_GL_ERROR("error in texture initialization.");
}

void MaterialFlares::glUniform_VP(const glm::mat4 & VP) const
{
	GlHelper::pushParameterToGPU(m_uniformVP, VP);
}

void MaterialFlares::glUniform_Depth(int textureId) const
{
	GlHelper::pushParameterToGPU(m_uniformDepth, textureId);
}

MaterialSSAO::MaterialSSAO()
	: Material()
{
}

MaterialSSAO::MaterialSSAO(const ShaderProgram & shaderProgram)
	: Material(shaderProgram)
{
	setExternalParameters();
}

MaterialSSAO::MaterialSSAO(const ShaderProgram & shaderProgram, const FileHandler::CompletePath & completePath)
	: Material(shaderProgram, completePath)
{
	setExternalParameters();
}

void MaterialSSAO::setExternalParameters()
{
	m_uniformResize = MaterialHelper::getUniform(m_glProgramId, "Resize");
	m_uniformNormals = MaterialHelper::getUniform(m_glProgramId, "Normals");
	m_uniformNoiseTexture = MaterialHelper::getUniform(m_glProgramId, "NoiseTexture");
	m_uniformKernel = MaterialHelper::getUniform(m_glProgramId, "Kernel");
	m_uniformDepth = MaterialHelper::getUniform(m_glProgramId, "Depth");
	m_uniformProjection = MaterialHelper::getUniform(m_glProgramId, "Projection");
	m_uniformScreenToView = MaterialHelper::getUniform(m_glProgramId, "ScreenToView");

	CHECK_GL_ERROR("error in texture initialization.");
}

void MaterialSSAO::glUniform_Normals(int texId) const
{
	GlHelper::pushParameterToGPU(m_uniformNormals, texId);
}

void MaterialSSAO::glUniform_NoiseTexture(int texId) const
{
	GlHelper::pushParameterToGPU(m_uniformNoiseTexture, texId);
}

void MaterialSSAO::glUniform_Kernel(const std::vector<glm::vec3>& kernel) const
{
	GlHelper::pushParametersToGPU(m_uniformKernel, 64, kernel);
}

void MaterialSSAO::glUniform_Depth(int texId) const
{
	GlHelper::pushParameterToGPU(m_uniformDepth, texId);
}

void MaterialSSAO::glUniform_Projection(const glm::mat4 & projection) const
{
	GlHelper::pushParameterToGPU(m_uniformProjection, projection);
}

void MaterialSSAO::glUniform_ScreenToView(const glm::mat4 & screenToView) const
{
	GlHelper::pushParameterToGPU(m_uniformScreenToView, screenToView);
}

void MaterialSSAO::glUniform_Resize(const glm::vec2 & resize) const
{
	GlHelper::pushParameterToGPU(m_uniformResize, resize);
}

MaterialSSAOBlur::MaterialSSAOBlur()
	: Material()
{
}

MaterialSSAOBlur::MaterialSSAOBlur(const ShaderProgram & shaderProgram)
	: Material(shaderProgram)
{
	setExternalParameters();
}

MaterialSSAOBlur::MaterialSSAOBlur(const ShaderProgram & shaderProgram, const FileHandler::CompletePath & completePath)
	: Material(shaderProgram, completePath)
{
	setExternalParameters();
}

void MaterialSSAOBlur::setExternalParameters()
{
	m_uniformResize = MaterialHelper::getUniform(m_glProgramId, "Resize");
	m_uniformTexture = MaterialHelper::getUniform(m_glProgramId, "Texture");

	CHECK_GL_ERROR("error in texture initialization.");
}

void MaterialSSAOBlur::glUniform_Texture(int texId) const
{
	GlHelper::pushParameterToGPU(m_uniformTexture, texId);

}

void MaterialSSAOBlur::glUniform_Resize(const glm::vec2 & resize) const
{
	GlHelper::pushParameterToGPU(m_uniformResize, resize);
}
