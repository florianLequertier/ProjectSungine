#pragma once

//#include "RenderBatch.h"
//#include "Materials.h"
//#include "Mesh.h"
//#include "ParticleEmitter.h"
//#include "Flag.h"
//#include "Billboard.h"

#include "IDrawable.h"

template<typename MaterialType>
class IBatchableWith : public IDrawable
{
public:
	virtual void setExternalsOf(const MaterialType& material, const glm::mat4& projection, const glm::mat4& view, int* texId = nullptr) const = 0;
};

//class SubMeshLitRenderBatch : public RenderBatch<SubMesh, MaterialLit>
//{
//public:
//	virtual void pushGlobalsToGPU(const MaterialLit& material) const override;
//	virtual void pushExternalsToPGU(const MaterialLit& materialInstance, const SubMesh& drawable, const glm::mat4& projection, const glm::mat4& view) const override;
//};
//
//class ParticlesRenderBatch : public RenderBatch<Physic::ParticleEmitter, MaterialParticlesCPU>
//{
//public:
//	virtual void pushGlobalsToGPU(const MaterialParticlesCPU& material) const override;
//	virtual void pushExternalsToPGU(const MaterialParticlesCPU& materialInstance, const Physic::ParticleEmitter& drawable, const glm::mat4& projection, const glm::mat4& view) const override;
//};
//
//class FlagLitRenderBatch : public RenderBatch<Physic::Flag, MaterialLit>
//{
//public:
//	virtual void pushGlobalsToGPU(const MaterialLit& material) const override;
//	virtual void pushExternalsToPGU(const MaterialLit& materialInstance, const Physic::Flag& drawable, const glm::mat4& projection, const glm::mat4& view) const override;
//};
//
//class BillboardLitRenderBatch : public RenderBatch<Billboard, MaterialBillboard>
//{
//public:
//	virtual void pushGlobalsToGPU(const MaterialBillboard& material) const override;
//	virtual void pushExternalsToPGU(const MaterialBillboard& materialInstance, const Billboard& drawable, const glm::mat4& projection, const glm::mat4& view) const override;
//};