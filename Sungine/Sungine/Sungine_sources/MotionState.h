#pragma once

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "TransformNode.h"

class MotionState : public btMotionState
{
private:
	btTransform m_physicTransform;
	TransformNode* m_entityTransform;

public:
	MotionState(TransformNode* entityTransform);
	virtual ~MotionState();


	//bullet motion state overides :
	virtual void getWorldTransform(btTransform& worldTransform) const override;
	virtual void setWorldTransform(const btTransform& worldTransform) override;
};

