#pragma once

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "MotionState.h"
#include "Component.h"
#include "Collider.h"

class Rigidbody : public Component
{
	COMPONENT_IMPLEMENTATION_HEADER(Rigidbody)

	CLASS((Rigidbody, Component),
	((PRIVATE)
		(glm::vec3	, m_translation		)
		(glm::quat	, m_rotation		)
		(glm::vec3	, m_scale			)
					
		(btScalar	, m_mass			,"", onMassChanged)
		(btVector3	, m_inertia			,"", onPhysicPropChanged)
					 
		(bool		, m_isTrigger		,"", onPhysicPropChanged)

		(bool		, m_useGravity		,"", onPhysicPropChanged)

		(bool		, m_frozenAxis		,"", onPhysicPropChanged) // ???
		(bool		, m_frozenAngles	,"", onPhysicPropChanged) // ???
	)
	)

private:

	btRigidBody* m_bulletRigidbody; // nosave, hide

	MotionState* m_motionState; // nosave, hide
	btCompoundShape* m_shape; // nosave, hide
	std::vector<Collider*> m_colliders; // nosave, hide

	btDiscreteDynamicsWorld* m_ptrToPhysicWorld; // nosave, hide
	
public:
	Rigidbody();
	Rigidbody(const Rigidbody& other);
	Rigidbody& operator=(const Rigidbody& other);
	~Rigidbody();

    /**
     * build a bullet rigidbody and send it to the btDynamicWorld
     * if m_target is in wolrd do nothing
     * to update a rigidbody, call popFromSimulation() and then pushToSimulation();
     */
    void pushToSimulation();

    /**
     * remove the target btRigidbody (if any) from the btDynamicWorld
     */
    void popFromSimulation();

    /**
     * delete the previous shape (if any), then make the multi shape, base on all the collider attached to this entity
     */
    void makeShape();

    /**
	 * Must be called after the rigidbody has been attached to an entity.
     * calculate mass, inertia, and motion state, then create appropriate btRigidBody. and place it in m_bulletRigidbody parameter
	 * also set the m_ptrToPhysicWorld with physicSimulation.
     */
    void init(btDiscreteDynamicsWorld* physicSimulation);

	void onMassChanged();
	void onPhysicPropChanged();

    float getMass() const;
    void setMass(float mass);
    btVector3 getInertia() const;
    void setInertia(const glm::vec3 &inertia);
	void setPtrToPhysicWorld(btDiscreteDynamicsWorld* ptrToPhysicWorld);
	void addColliders(std::vector<Collider*> colliders);
	void addCollider(Collider* collider);
	//only remove a collider from the rigidbody, don't destroy them.
	void removeCollider(Collider* collider);
	//only remove all colliders from the rigidbody, don't destroy them.
	void removeAllColliders();
	void freezeAxis(bool x, bool y, bool z);
	void freezeAngles(bool x, bool y, bool z);
	void setUseGravity(bool useGravity);

	void setIsTrigger(bool state);

	//virtual void drawInInspector(Scene& scene) override;
	//virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat());
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat());
	//void applyTransformFromPhysicSimulation();

	virtual void save(Json::Value& componentRoot) const override;
	virtual void load(const Json::Value& componentRoot) override;

	virtual void onAfterAddedToScene(Scene& scene) override;
	virtual void onAfterAddedToEntity(Entity& entity) override;
};

REGISTER_CLASS(Rigidbody)

