#pragma once

#include "BasicColliders.h"

class Skeleton;
class Billboard;
class ReflectivePlane;
namespace Physic {
	class ParticleEmitter;
}

class IDrawable
{
public:
	virtual const AABB& getVisualBoundingBox() const = 0;
	virtual void draw() const = 0;
	virtual const glm::mat4& getModelMatrix() const = 0;
	virtual bool castShadows() const = 0;
	virtual Skeleton* getSkeletonIfPossible() const { return nullptr; }
	virtual const Billboard* getAsBillboardIfPossible() const { return nullptr; }
	virtual const Physic::ParticleEmitter* getAsParticleEmiterIfPossible() const { return nullptr; }
	virtual const ReflectivePlane* getAsReflectivePlaneIfPossible() const { return nullptr; }
};