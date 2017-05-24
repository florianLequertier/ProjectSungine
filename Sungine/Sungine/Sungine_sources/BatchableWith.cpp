//#include "RenderBatches.h"
//
//#include "Skeleton.h"




//void SubMeshLitRenderBatch::pushGlobalsToGPU(const MaterialLit& material) const
//{
//	// Nothing
//}
//
//void SubMeshLitRenderBatch::pushExternalsToPGU(const MaterialLit& materialInstance, const SubMesh& drawable, const glm::mat4& projection, const glm::mat4& view) const
//{
//	glm::mat4 modelMatrix = drawable.getModelMatrix();
//	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
//	glm::mat4 mvp = projection * view * modelMatrix;
//
//	// Uniform MVP :
//	materialInstance.setUniform_MVP(mvp);
//	// Uniform NormalMatrix
//	materialInstance.setUniform_normalMatrix(normalMatrix);
//	// Uniform BonesTransform
//	if (drawable.getMeshPtr()->getIsSkeletalMesh()) 
//	{
//		for (int boneIdx = 0; boneIdx < drawable.getMeshPtr()->getSkeleton()->getBoneCount(); boneIdx++)
//			materialInstance.setUniformBonesTransform(boneIdx, drawable.getMeshPtr()->getSkeleton()->getBoneTransform(boneIdx));
//	}
//	// Uniform UseSkeleton
//	materialInstance.setUniformUseSkeleton(drawable.getMeshPtr()->getIsSkeletalMesh());
//}
//
//void ParticlesRenderBatch::pushGlobalsToGPU(const MaterialParticlesCPU & material) const
//{
//	// Nothing
//}
//
//void ParticlesRenderBatch::pushExternalsToPGU(const MaterialParticlesCPU & materialInstance, const Physic::ParticleEmitter & drawable, const glm::mat4& projection, const glm::mat4& view) const
//{
//	glm::mat4 VP = projection * view;
//	glm::vec3 CameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
//	glm::vec3 CameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
//	glm::vec3 CameraPos = glm::vec3(view[0][3], view[1][3], view[2][3]);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, drawable.getParticleTexture().glId);
//
//	materialInstance.glUniform_VP(VP);
//	materialInstance.setUniformCameraRight(CameraRight);
//	materialInstance.setUniformCameraUp(CameraUp);
//	materialInstance.setUniformTexture(0);
//}
//
//void FlagLitRenderBatch::pushGlobalsToGPU(const MaterialLit & material) const
//{
//	// Nothing
//}
//
//void FlagLitRenderBatch::pushExternalsToPGU(const MaterialLit & materialInstance, const Physic::Flag & drawable, const glm::mat4& projection, const glm::mat4& view) const
//{
//	const glm::mat4& modelMatrix = drawable.getModelMatrix();
//	const glm::mat4 mvp = projection * view * modelMatrix;
//	const glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
//
//	materialInstance.setUniform_MVP(mvp);
//	materialInstance.setUniform_normalMatrix(normalMatrix);
//}
//
//void BillboardLitRenderBatch::pushGlobalsToGPU(const MaterialBillboard & material) const
//{
//	// Nothing
//}
//
//void BillboardLitRenderBatch::pushExternalsToPGU(const MaterialBillboard & materialInstance, const Billboard & drawable, const glm::mat4& projection, const glm::mat4& view) const
//{
//	glm::mat4 MVP = projection * view;
//	glm::vec3 CameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
//	glm::vec3 CameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, drawable.getTexture().glId);
//
//	materialInstance.setUniformMVP(MVP);
//	materialInstance.setUniformCameraRight(CameraRight);
//	materialInstance.setUniformCameraUp(CameraUp);
//	materialInstance.setUniformScale(drawable.getScale());
//	materialInstance.setUniformTexture(0);
//	materialInstance.setUniformTranslation(drawable.getTranslation());
//	materialInstance.setUniformColor(drawable.getColor());
//}
