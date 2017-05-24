#include "MaterialAggregation.h"
#include "MaterialHelper.h"
#include "Skeleton.h"
#include "RenderDatas.h"
#include "Billboard.h"
#include "ParticleEmitter.h"
#include "ReflectivePlane.h"
#include "Camera.h"

void MaterialAggregationWithSkeleton::initParameters(GLuint programID)
{
	uniform_bonesTransform = MaterialHelper::getUniforms(programID, "BonesTransform", MAX_BONE_COUNT);
	uniform_useSkeleton = MaterialHelper::getUniform(programID, "UseSkeleton");
}

void MaterialAggregationWithSkeleton::pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const
{
	Skeleton* skeleton = drawable.getSkeletonIfPossible();
	if (skeleton == nullptr)
	{
		setUniformUseSkeleton(false);
	}
	else
	{
		setUniformUseSkeleton(true);

		for (int boneIdx = 0; boneIdx < skeleton->getBoneCount(); boneIdx++)
			setUniformBonesTransform(boneIdx, skeleton->getBoneTransform(boneIdx));
	}
}

void MaterialAggregationWithSkeleton::setUniformBonesTransform(unsigned int idx, const glm::mat4& boneTransform) const
{
	GlHelper::pushParameterToGPU(uniform_bonesTransform[idx], boneTransform);
}
void MaterialAggregationWithSkeleton::setUniformUseSkeleton(bool useSkeleton) const
{
	GlHelper::pushParameterToGPU(uniform_useSkeleton, useSkeleton);
}

///////////////////////////////////

void MaterialAggregationMesh::initParameters(GLuint programID)
{
	uniform_ModelMatrix = MaterialHelper::getUniform(programID, "ModelMatrix");
	uniform_ViewMatrix = MaterialHelper::getUniform(programID, "ViewMatrix");
	uniform_ProjectionMatrix = MaterialHelper::getUniform(programID, "ProjectionMatrix");
	uniform_ClipPlane = MaterialHelper::getUniform(programID, "ClipPlane");

	CHECK_GL_ERROR("error in material initialization.");
}
void MaterialAggregationMesh::pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const
{
	// Transform matrices
	setUniformModelMatrix(drawable.getModelMatrix());
	setUniformViewMatrix(*renderDatas.View);
	setUniformProjectionMatrix(*renderDatas.Projection);
	setUniformClipPlane(renderDatas.clipPlane);
}

void MaterialAggregationMesh::setUniformModelMatrix(const glm::mat4& modelMatrix) const
{
	GlHelper::pushParameterToGPU(uniform_ModelMatrix, modelMatrix);
}

void MaterialAggregationMesh::setUniformViewMatrix(const glm::mat4& viewMatrix) const
{
	GlHelper::pushParameterToGPU(uniform_ViewMatrix, viewMatrix);
}

void MaterialAggregationMesh::setUniformProjectionMatrix(const glm::mat4& projectionMatrix) const
{
	GlHelper::pushParameterToGPU(uniform_ProjectionMatrix, projectionMatrix);
}

void MaterialAggregationMesh::setUniformClipPlane(const glm::vec4& clipPlane) const
{
	GlHelper::pushParameterToGPU(uniform_ClipPlane, clipPlane);
}

///////////////////////////////////

void MaterialAggregationBillboard::initParameters(GLuint programID)
{
	uniform_MVP = MaterialHelper::getUniform(programID, "MVP");
	uniform_Scale = MaterialHelper::getUniform(programID, "Scale");
	uniform_Translation = MaterialHelper::getUniform(programID, "Translation");
	uniform_Texture = MaterialHelper::getUniform(programID, "Texture");
	uniform_CameraRight = MaterialHelper::getUniform(programID, "CameraRight");
	uniform_CameraUp = MaterialHelper::getUniform(programID, "CameraUp");
	uniform_Color = MaterialHelper::getUniform(programID, "Color");
	uniform_ClipPlane = MaterialHelper::getUniform(programID, "ClipPlane");

	CHECK_GL_ERROR("error in material initialization.");
}

void MaterialAggregationBillboard::pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const
{
	const Billboard* drawableAsBillboard = drawable.getAsBillboardIfPossible();
	if (drawableAsBillboard != nullptr)
		return;

	const glm::mat4& view(*renderDatas.View);
	glm::vec3 CameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
	glm::vec3 CameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, drawableAsBillboard->getTexture().glId);

	setUniformMVP(renderDatas.VP);
	setUniformCameraRight(CameraRight);
	setUniformCameraUp(CameraUp);
	setUniformScale(drawableAsBillboard->getScale());
	setUniformTexture(0);
	setUniformTranslation(drawableAsBillboard->getTranslation());
	setUniformColor(drawableAsBillboard->getColor());
	setUniformClipPlane(renderDatas.clipPlane);
}

void MaterialAggregationBillboard::setUniformMVP(const glm::mat4& VP) const
{
	GlHelper::pushParameterToGPU<glm::mat4>(uniform_MVP, VP);
}

void MaterialAggregationBillboard::setUniformScale(const glm::vec2& scale) const
{
	GlHelper::pushParameterToGPU(uniform_MVP, scale);
}

void MaterialAggregationBillboard::setUniformTranslation(const glm::vec3& translation) const
{
	GlHelper::pushParameterToGPU(uniform_MVP, translation);
}

void MaterialAggregationBillboard::setUniformTexture(int texId) const
{
	GlHelper::pushParameterToGPU(uniform_MVP, texId);
}

void MaterialAggregationBillboard::setUniformCameraRight(const glm::vec3& camRight) const
{
	GlHelper::pushParameterToGPU(uniform_MVP, camRight);
}

void MaterialAggregationBillboard::setUniformCameraUp(const glm::vec3& camUp) const
{
	GlHelper::pushParameterToGPU(uniform_MVP, camUp);
}

void MaterialAggregationBillboard::setUniformColor(const glm::vec4& color) const
{
	GlHelper::pushParameterToGPU(uniform_MVP, color);
}

void MaterialAggregationBillboard::setUniformClipPlane(const glm::vec4 & clipPlane) const
{
	GlHelper::pushParameterToGPU(uniform_ClipPlane, clipPlane);
}

////////////////////////////////////

void MaterialAggregationParticles::initParameters(GLuint programID)
{
	m_uniformVP = MaterialHelper::getUniform(programID, "VP");
	m_uniformTexture = MaterialHelper::getUniform(programID, "Texture");
	m_uniformCameraRight = MaterialHelper::getUniform(programID, "CameraRight");
	m_uniformCameraUp = MaterialHelper::getUniform(programID, "CameraUp");
	m_uniformClipPlane = MaterialHelper::getUniform(programID, "ClipPlane");

	CHECK_GL_ERROR("error in material initialization.");
}

void MaterialAggregationParticles::pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const
{
	const Physic::ParticleEmitter* drawableAsParticleEmitter = drawable.getAsParticleEmiterIfPossible();

	const glm::mat4& view(*renderDatas.View);
	glm::vec3 CameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
	glm::vec3 CameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
	glm::vec3 CameraPos = glm::vec3(view[0][3], view[1][3], view[2][3]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, drawableAsParticleEmitter->getParticleTexture().glId);

	glUniform_VP(renderDatas.VP);
	setUniformCameraRight(CameraRight);
	setUniformCameraUp(CameraUp);
	setUniformTexture(0);
	setUniformClipPlane(renderDatas.clipPlane);
}

void MaterialAggregationParticles::glUniform_VP(const glm::mat4& VP) const
{
	GlHelper::pushParameterToGPU(m_uniformVP, VP);
}

void MaterialAggregationParticles::setUniformTexture(int texId) const
{
	GlHelper::pushParameterToGPU(m_uniformTexture, texId);
}

void MaterialAggregationParticles::setUniformCameraRight(const glm::vec3& camRight) const
{
	GlHelper::pushParameterToGPU(m_uniformCameraRight, camRight);
}

void MaterialAggregationParticles::setUniformCameraUp(const glm::vec3& camUp) const
{
	GlHelper::pushParameterToGPU(m_uniformCameraUp, camUp);
}

void MaterialAggregationParticles::setUniformClipPlane(const glm::vec4 & clipPlane) const
{
	GlHelper::pushParameterToGPU(m_uniformClipPlane, clipPlane);
}

////////////////////////////////////

void MaterialAggregationReflectivePlane::initParameters(GLuint programID)
{
	uniform_reflectionTexture = MaterialHelper::getUniform(programID, "ReflectionTexture");
	uniform_ModelMatrix = MaterialHelper::getUniform(programID, "ModelMatrix");
	uniform_ViewMatrix = MaterialHelper::getUniform(programID, "ViewMatrix");
	uniform_ProjectionMatrix = MaterialHelper::getUniform(programID, "ProjectionMatrix");

	CHECK_GL_ERROR("error in material initialization.");
}

void MaterialAggregationReflectivePlane::pushParametersToGPU(const IDrawable& drawable, const RenderDatas& renderDatas, int& boundTextureCount) const
{
	if (!renderDatas.currentCameraID.isValid())
		return;

	const ReflectivePlane* drawableAsReflectivePlane = drawable.getAsReflectivePlaneIfPossible();

	if (drawableAsReflectivePlane == nullptr)
		return;

	setUniformModelMatrix(drawableAsReflectivePlane->getModelMatrix());
	setUniformProjectionMatrix(*renderDatas.Projection);
	setUniformViewMatrix(*renderDatas.View);

	// Reflective texture
	glActiveTexture(GL_TEXTURE0 + boundTextureCount);
	glBindTexture(GL_TEXTURE_2D, renderDatas.currentReflectionLayer->getFinalFrame());
	setUniformReflectionTexture(boundTextureCount);
	boundTextureCount++;
}

void MaterialAggregationReflectivePlane::setUniformReflectionTexture(int texUnitId) const
{
	GlHelper::pushParameterToGPU(uniform_reflectionTexture, texUnitId);
}

void MaterialAggregationReflectivePlane::setUniformModelMatrix(const glm::mat4& modelMatrix) const
{
	GlHelper::pushParameterToGPU(uniform_ModelMatrix, modelMatrix);
}

void MaterialAggregationReflectivePlane::setUniformViewMatrix(const glm::mat4& viewMatrix) const
{
	GlHelper::pushParameterToGPU(uniform_ViewMatrix, viewMatrix);
}

void MaterialAggregationReflectivePlane::setUniformProjectionMatrix(const glm::mat4& projectionMatrix) const
{
	GlHelper::pushParameterToGPU(uniform_ProjectionMatrix, projectionMatrix);
}