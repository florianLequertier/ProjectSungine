#pragma once
#include "Behavior.h"



class TestBehavior : public Behavior
{
public:
	//BEHAVIOR(TestBehavior)
	COMPONENT_IMPLEMENTATION_HEADER(TestBehavior)

	TestBehavior();
	virtual ~TestBehavior();

	virtual void start(Scene& scene) override;
	virtual void update(Scene& scene) override;
	virtual void onCollisionEnter(Scene& scene, const CollisionInfo& collisionInfo) override;
	virtual void onCollisionStay(Scene& scene, const CollisionInfo& collisionInfo) override;
	virtual void onCollisionEnd(Scene& scene, const CollisionInfo& collisionInfo) override;

	// Hérité via Behavior
	virtual void save(Json::Value & entityRoot) const override;
	virtual void load(const Json::Value & entityRoot) override;
};


