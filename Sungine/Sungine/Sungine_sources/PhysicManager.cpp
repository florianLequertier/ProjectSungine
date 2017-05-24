

#include "PhysicManager.h"
//forwards :
#include "Application.h"
#include "Entity.h"
#include "DebugDrawer.h"

namespace Physic {

	bool CollisionPair:: operator==(const CollisionPair& other) const {
		return ( ((collisionInfoA.receiver == other.collisionInfoA.receiver) && (collisionInfoA.rigidbody == other.collisionInfoA.rigidbody))
			|| ((collisionInfoA.receiver == other.collisionInfoA.rigidbody) && (collisionInfoA.rigidbody == other.collisionInfoA.receiver)) );
	}
	
	/////////////////////////////////////

	void CollisionPair::onCollisionBegin() {
		if (collisionInfoA.receiver != nullptr && collisionInfoA.receiver->entity() != nullptr)
			collisionInfoA.receiver->entity()->onCollisionBegin(collisionInfoA);
		if (collisionInfoB.receiver != nullptr && collisionInfoB.receiver->entity() != nullptr)
			collisionInfoB.receiver->entity()->onCollisionBegin(collisionInfoB);
	}

	void CollisionPair::onCollisionStay() {
		if (collisionInfoA.receiver != nullptr && collisionInfoA.receiver->entity() != nullptr)
			collisionInfoA.receiver->entity()->onCollisionStay(collisionInfoA);
		if (collisionInfoB.receiver != nullptr && collisionInfoB.receiver->entity() != nullptr)
			collisionInfoB.receiver->entity()->onCollisionStay(collisionInfoB);
	}

	void CollisionPair::onCollisionEnd() {
		if (collisionInfoA.receiver != nullptr && collisionInfoA.receiver->entity() != nullptr)
			collisionInfoA.receiver->entity()->onCollisionEnd(collisionInfoA);
		if (collisionInfoB.receiver != nullptr && collisionInfoB.receiver->entity() != nullptr)
			collisionInfoB.receiver->entity()->onCollisionEnd(collisionInfoB);
	}

	////////////////////////////////

	DebugDrawerPhysicWorld::DebugDrawerPhysicWorld() : m_debugMode(DBG_DrawWireframe | DBG_DrawAabb)
	{
	}

	DebugDrawerPhysicWorld::~DebugDrawerPhysicWorld()
	{

	}

	void DebugDrawerPhysicWorld::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
	{
		DebugDrawer::drawLine(glm::vec3(from.x(), from.y(), from.z()), glm::vec3(to.x(), to.y(), to.z()), glm::vec3(color.x(), color.y(), color.z()));
	}

	void DebugDrawerPhysicWorld::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
	{
		//nothing
	}

	void DebugDrawerPhysicWorld::reportErrorWarning(const char *warningString)
	{
		std::cerr << warningString << std::endl;
	}

	void DebugDrawerPhysicWorld::draw3dText(const btVector3 &location, const char *textString)
	{
		//nothing
	}

	void DebugDrawerPhysicWorld::setDebugMode(int debugMode)
	{
		m_debugMode = debugMode;
	}

	int DebugDrawerPhysicWorld::getDebugMode() const
	{
		return m_debugMode;
	}

	//////////////////////////////////////////////////
	bool _onCollisionBegin(btManifoldPoint& pt, const btCollisionObjectWrapper* obA, int partId0, int index0, const btCollisionObjectWrapper* obB, int partId1, int index1)
	{
		CollisionInfo collisionInfo;
		if (pt.getDistance() < 0.f)
		{
			//const btVector3& ptA = pt.getPositionWorldOnA();
			const btVector3& ptB = pt.getPositionWorldOnB();
			const btVector3& normalOnB = pt.m_normalWorldOnB;

			collisionInfo.normal = glm::vec3(normalOnB.x(), normalOnB.y(), normalOnB.z());
			collisionInfo.point = glm::vec3(ptB.x(), ptB.y(), ptB.z());
		}
		//if (obB->getCollisionObject()->getUserPointer() != nullptr) {
			collisionInfo.rigidbody = static_cast<Rigidbody*>(obB->getCollisionObject()->getUserPointer());
			Entity* entity = collisionInfo.rigidbody->entity();
			if(entity != nullptr)
				entity->onCollisionBegin(collisionInfo);
		//}

		return false;
	}

	PhysicManager::PhysicManager(const glm::vec3& _gravity) : m_gravity(_gravity), m_physicWorld(nullptr)
	{
		//build the bullet physic world : 
		m_collisionConfiguration = new btDefaultCollisionConfiguration();
		m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
		m_broadPhase = new btDbvtBroadphase();
		m_sequentialImpulseConstraintSolver = new btSequentialImpulseConstraintSolver();

		m_physicWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadPhase, m_sequentialImpulseConstraintSolver, m_collisionConfiguration);

		//ghost callback :
		btGhostPairCallback* ghostPairCallback = new btGhostPairCallback();
		m_physicWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);

		// debug :
		m_debugDrawerPhysicWorld = new DebugDrawerPhysicWorld();

		btIDebugDraw::DefaultColors debugColors;
		debugColors.m_aabb = btVector3(1, 0, 0);
		debugColors.m_activeObject = btVector3(0, 1, 0);
		debugColors.m_deactivatedObject = btVector3(0, 0, 1);

		m_debugDrawerPhysicWorld->setDefaultColors(debugColors);
		m_debugDrawerPhysicWorld->setDebugMode(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawWireframe);
		m_physicWorld->setDebugDrawer(m_debugDrawerPhysicWorld);

		//gContactAddedCallback = _onCollisionBegin;
	}


	PhysicManager::~PhysicManager()
	{
		clear();
	}

	void PhysicManager::clear()
	{
		if (m_physicWorld != nullptr) {
			delete m_physicWorld;
			m_physicWorld = nullptr;
		}
		if (m_sequentialImpulseConstraintSolver != nullptr) {
			delete m_sequentialImpulseConstraintSolver;
			m_sequentialImpulseConstraintSolver = nullptr;
		}
		if (m_broadPhase != nullptr) {
			delete m_broadPhase;
			m_broadPhase = nullptr;
		}
		if (m_dispatcher != nullptr) {
			delete m_dispatcher;
			m_dispatcher = nullptr;
		}
		if (m_collisionConfiguration != nullptr) {
			delete m_collisionConfiguration;
			m_collisionConfiguration = nullptr;
		}

		// debug :
		if (m_debugDrawerPhysicWorld != nullptr) {
			delete m_debugDrawerPhysicWorld;
			m_debugDrawerPhysicWorld = nullptr;
		}
	}

	void PhysicManager::setGravity(const glm::vec3&  g)
	{
		m_gravity = g;
		m_physicWorld->setGravity(btVector3(m_gravity.x, m_gravity.y, m_gravity.z));
	}

	glm::vec3  PhysicManager::getGravity() const
	{
		return m_gravity;
	}

	btDiscreteDynamicsWorld * PhysicManager::getBulletDynamicSimulation() const
	{
		return m_physicWorld;
	}

	void PhysicManager::update(float deltaTime, const BaseCamera& camera, std::vector<Flag*>& flags, /*Terrain& terrain,*/ std::vector<WindZone*>& windZones, std::vector<ParticleEmitter*>& particleEmitters)
	{
		//update bullet internal physic :
		m_physicWorld->stepSimulation(Application::get().getFixedDeltaTime(), 10);
		physicLateUpdate();

		//update the reste of physic : 

		//update flags :
		for (int i = 0; i < flags.size(); i++)
		{
			for (int j = 0; j < windZones.size(); j++)
			{
				// TODO replace by flags[i]->getTransform().position...
				flags[i]->applyForce(windZones[j]->getForce(Application::get().getTime(), flags[i]->entity()->getTranslation() ));
			}

			flags[i]->applyGravity(m_gravity);
			flags[i]->update(deltaTime);
		}

		//update terrain : 
		//terrain.updatePhysic(deltaTime, windZones);

		//update particles : 
		for (int i = 0; i < particleEmitters.size(); i++)
		{
			particleEmitters[i]->update(deltaTime, camera.getCameraPosition());
		}
	}


	void PhysicManager::update(float deltaTime, const BaseCamera& camera, std::vector<Flag*>& flags,/* Terrain& terrain, */std::vector<WindZone*>& windZones, std::vector<ParticleEmitter*>& particleEmitters, bool updateRigidbodies)
	{
		if (updateRigidbodies) {
			//update bullet internal physic :
			m_physicWorld->stepSimulation(Application::get().getFixedDeltaTime(), 10);
			physicLateUpdate();
		}

		//update flags :
		for (int i = 0; i < flags.size(); i++)
		{
			for (int j = 0; j < windZones.size(); j++)
			{
				// TODO replace by flags[i]->getTransform().position...
				flags[i]->applyForce(windZones[j]->getForce(Application::get().getTime(), flags[i]->entity()->getTranslation()));
			}

			flags[i]->applyGravity(m_gravity);
			flags[i]->update(deltaTime);
		}

		//update terrain : 
		//terrain.updatePhysic(deltaTime, windZones);

		//update particles : 
		for (int i = 0; i < particleEmitters.size(); i++)
		{
			particleEmitters[i]->update(deltaTime, camera.getCameraPosition());
		}
	}

	void PhysicManager::physicLateUpdate()
	{
		int numManifolds = m_physicWorld->getDispatcher()->getNumManifolds();
		for (int i = 0; i < numManifolds; i++)
		{
			btPersistentManifold* contactManifold = m_physicWorld->getDispatcher()->getManifoldByIndexInternal(i);
			const btCollisionObject* obA = contactManifold->getBody0();
			const btCollisionObject* obB = contactManifold->getBody1();

			CollisionInfo collisionInfo01;
			CollisionInfo collisionInfo02;

			int numContacts = contactManifold->getNumContacts();
			for (int j = 0; j < numContacts; j++)
			{
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				if (pt.getDistance() < 0.f)
				{
					const btVector3& ptA = pt.getPositionWorldOnA();
					const btVector3& ptB = pt.getPositionWorldOnB();
					const btVector3& normalOnB = pt.m_normalWorldOnB;

					collisionInfo01.normal = glm::vec3(normalOnB.x(), normalOnB.y(), normalOnB.z());
					collisionInfo01.point = glm::vec3(ptB.x(), ptB.y(), ptB.z());

					collisionInfo02.normal = glm::vec3(-normalOnB.x(), -normalOnB.y(), -normalOnB.z());
					collisionInfo02.point = glm::vec3(ptA.x(), ptA.y(), ptA.z());

					break;
				}
			}

			collisionInfo01.receiver = (obA->getUserIndex() == 1) ? static_cast<Rigidbody*>(obA->getUserPointer()) : nullptr; // rigidbody A ...
			collisionInfo01.rigidbody = (obB->getUserIndex() == 1) ? static_cast<Rigidbody*>(obB->getUserPointer()) : nullptr; // ... collide with rigidbody B.
			//collisionInfo01.receiver->entity()->onCollisionBegin(collisionInfo01); 

			//if two collider are on the same rigidbody, don't send callback :
			if (collisionInfo01.receiver == collisionInfo01.rigidbody)
				continue;

			collisionInfo02.receiver = (obB->getUserIndex() == 1) ? static_cast<Rigidbody*>(obB->getUserPointer()) : nullptr; // rigidbody B ...
			collisionInfo02.rigidbody = (obA->getUserIndex() == 1) ? static_cast<Rigidbody*>(obA->getUserPointer()) : nullptr; // ... collide with rigidbody A.
			//collisionInfo02.receiver->entity()->onCollisionBegin(collisionInfo02);

			CollisionPair collisionPair(collisionInfo01, collisionInfo02);
			CollisionKey collisionKey(obA, obB);

			//the pair already exists, the collision continues :
			auto findIt = m_collisionPairUpdateState.find(collisionKey);
			if(findIt != m_collisionPairUpdateState.end()) {
				findIt->second.collisionState = CollisionPair::CollisionState::STAY;
				std::cout << "collision stay !" << std::endl;
				//we update the collisions infos :
				findIt->second.collisionInfoA = collisionInfo01;
				findIt->second.collisionInfoB = collisionInfo02;
				//trigger collision stays :
				collisionPair.onCollisionStay();
			}
			//the pair doesn't exists, the collision begins :
			else {
				//we add the new collision pair to the map :
				m_collisionPairUpdateState[collisionKey] = collisionPair;
				std::cout << "collision begin !" << std::endl;
				//trigger collision begins :
				collisionPair.onCollisionBegin();
			}
		}

		for (auto it = m_collisionPairUpdateState.begin(); it != m_collisionPairUpdateState.end(); ){
			//collision state already marked as ended, we need to remove this collision pair :
			if (it->second.collisionState == CollisionPair::CollisionState::END) {
				std::cout << "collision end !" << std::endl;
				//trigger collision ends :
				it->second.onCollisionEnd();
				//delete it->first;
				/*it = */m_collisionPairUpdateState.erase(it++);
			}
			else {
				it->second.collisionState = CollisionPair::CollisionState::END;
				++it;
			}
		}
	}

	bool PhysicManager::onCollisionBegin(btManifoldPoint& pt, const btCollisionObjectWrapper* obA, int partId0, int index0, const btCollisionObjectWrapper* obB, int partId1, int index1)
	{
		CollisionInfo collisionInfo;
		if (pt.getDistance() < 0.f)
		{
			//const btVector3& ptA = pt.getPositionWorldOnA();
			const btVector3& ptB = pt.getPositionWorldOnB();
			const btVector3& normalOnB = pt.m_normalWorldOnB;

			collisionInfo.normal = glm::vec3(normalOnB.x(), normalOnB.y(), normalOnB.z());
			collisionInfo.point = glm::vec3(ptB.x(), ptB.y(), ptB.z());
		}
		collisionInfo.rigidbody = static_cast<Rigidbody*>(obB->getCollisionObject()->getUserPointer());
		collisionInfo.rigidbody->entity()->onCollisionBegin(collisionInfo);

		return false;
	}

	void PhysicManager::debugDraw(const glm::mat4 & projection, const glm::mat4 & view) const
	{
		if (m_debugDrawerPhysicWorld != nullptr) {
			m_physicWorld->debugDrawWorld();
			CHECK_GL_ERROR("error when rendering debug physic");
		}
	}

}
