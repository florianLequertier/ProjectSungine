#pragma once

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics\Character\btCharacterControllerInterface.h"
#include "BulletDynamics\Character\btKinematicCharacterController.h"

#include "Component.h"

class btKinematicClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
	btKinematicClosestNotMeRayResultCallback(btCollisionObject* me) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0)) {
		m_me = me;
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) {
		if (rayResult.m_collisionObject == m_me)
			return 1.0;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}
protected:
	btCollisionObject* m_me;
};
//---------------------------------------------------------------------------------------
class btKinematicClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
	btKinematicClosestNotMeConvexResultCallback(btCollisionObject* me, const btVector3& up, btScalar minSlopeDot)
		: btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
		, m_me(me)
		, m_up(up)
		, m_minSlopeDot(minSlopeDot)
	{
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) {
		if (convexResult.m_hitCollisionObject == m_me)
			return btScalar(1.0);

		//for trigger filtering
		if (!convexResult.m_hitCollisionObject->hasContactResponse())
			return btScalar(1.0);

		btVector3 hitNormalWorld;
		if (normalInWorldSpace)
			hitNormalWorld = convexResult.m_hitNormalLocal;
		else
			hitNormalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal;

		btScalar dotUp = m_up.dot(hitNormalWorld);
		if (dotUp < m_minSlopeDot)
			return btScalar(1.0);

		return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}
protected:
	btCollisionObject* m_me;
	const btVector3 m_up;
	btScalar m_minSlopeDot;
};

class CharacterController: public Component
{
	OBJECT_CLASS(CharacterController,
		ObjectDescriptor<CharacterController>::registerParentClass<Component>();

		ObjectDescriptor<CharacterController>::registerProperty<glm::vec3>(&CharacterController::m_translation);
		ObjectDescriptor<CharacterController>::registerProperty<glm::quat>(&CharacterController::m_rotation);
		ObjectDescriptor<CharacterController>::registerProperty<glm::vec3>(&CharacterController::m_scale);

		ObjectDescriptor<CharacterController>::registerProperty<glm::vec3>(&CharacterController::m_speed);
		ObjectDescriptor<CharacterController>::registerProperty<glm::vec3>(&CharacterController::m_force);
		
		ObjectDescriptor<CharacterController>::registerProperty<float>(&CharacterController::m_height);
		ObjectDescriptor<CharacterController>::registerProperty<float>(&CharacterController::m_radius);

		ObjectDescriptor<CharacterController>::registerProperty<float>(&CharacterController::m_gravityFactor);
		ObjectDescriptor<CharacterController>::registerProperty<float>(&CharacterController::m_jumpFactor);
		ObjectDescriptor<CharacterController>::registerProperty<float[4]>(&CharacterController::m_speedFactor)
	)

	COMPONENT_IMPLEMENTATION_HEADER(CharacterController)

	enum Direction {FORWARD = 0, RIGHT = 1, LEFT = 2, BACKWARD = 3};
private:
	glm::vec3 m_translation;
	glm::quat m_rotation;
	glm::vec3 m_scale;

	glm::vec3 m_speed;
	glm::vec3 m_force;

	float m_height;
	float m_radius;

	float m_gravityFactor;
	float m_jumpFactor;
	float m_speedFactor[4]; //one speed per direction

	bool m_isOnGround;
	bool m_isJumping;

	btGhostObject* m_bulletGhostbody;
	bool m_isGhostInWorld;
	btCapsuleShape* m_shape;
	btDiscreteDynamicsWorld* m_ptrToPhysicWorld;

public:
	CharacterController();
	CharacterController(const CharacterController& other);
	CharacterController& operator=(const CharacterController& other);
	~CharacterController();

	void pushToSimulation();
	void popFromSimulation();
	void makeShape();
	void init(btDiscreteDynamicsWorld* physicSimulation);
	void setPtrToPhysicWorld(btDiscreteDynamicsWorld* ptrToPhysicWorld);
	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat());
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat());
	void checkBlockingByCollision(glm::vec3& nextPosition);
	void update(float deltaTime);

	void setGravity(float gravityFactor);
	float getGravity() const;
	void setSpeed(Direction direction, float speedFactor);
	float getSpeed(Direction direction) const;
	void setSpeed(float speedFactor);
	void setJumpFactor(float jumpFactor);
	float getJumpFactor() const;
	void setHeight(float height);
	float getHeight() const;
	void setRadius(float radius);
	float getRadius() const;

	void jump();
	void move(glm::vec3 direction);

	virtual void drawInInspector(Scene & scene) override;
	virtual void drawInInspector(Scene & scene, const std::vector<Component*>& components) override;

	virtual void save(Json::Value& componentRoot) const override;
	virtual void load(const Json::Value& componentRoot) override;

	virtual void onAfterComponentAddedToScene(Scene& scene) override;
	virtual void onAfterComponentAddedToEntity(Entity& entity) override;
};

REGISTER_CLASS(CharacterController)