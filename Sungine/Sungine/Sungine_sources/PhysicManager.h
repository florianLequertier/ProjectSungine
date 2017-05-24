#pragma once

#include <map>
#include <unordered_map>

#include "Flag.h"
#include "WindZone.h"
#include "Terrain.h"
#include "ParticleEmitter.h"
#include "Camera.h"
#include "Rigidbody.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "linearMath/btIDebugDraw.h"



namespace Physic {

	enum CollisionMasks {
		STATIC = 1 << 0,
		DYNAMIC = 1 << 1,
		TRIGGER = 1 << 2,
		ACTOR = 1 << 3,

		STATIC_GROUP = STATIC | ACTOR,
		DYNAMIC_GROUP = STATIC | DYNAMIC | TRIGGER | ACTOR,
		TRIGGER_GROUP = DYNAMIC | ACTOR,
		ACTOR_GROUP = STATIC | DYNAMIC | TRIGGER | ACTOR,
	};

	struct CollisionKey {
		const btCollisionObject* objA;
		const btCollisionObject* objB;

		CollisionKey(const btCollisionObject* A, const btCollisionObject* B): objA(A), objB(B)
		{ }

		inline bool operator==(const CollisionKey& other) const {
			return (objA == other.objA && objB == other.objB) || (objA == other.objB && objB == other.objA);
		}
	};

	struct CollisionPair
	{
		enum CollisionState { BEGIN, STAY, END };

		//Rigidbody* rigidbodyA;
		//Rigidbody* rigidbodyB;
		CollisionInfo collisionInfoA;
		CollisionInfo collisionInfoB;
		CollisionState collisionState;

		CollisionPair(CollisionInfo _collisionInfoA = CollisionInfo(), CollisionInfo _collisionInfoB = CollisionInfo()) : collisionInfoA(_collisionInfoA), collisionInfoB(_collisionInfoB), collisionState(BEGIN)
		{ }

		void onCollisionBegin();
		void onCollisionStay();
		void onCollisionEnd();

		//friend bool operator<(const CollisionPair& A, const CollisionPair& B);
		inline bool operator<(const CollisionPair& other) const {
			return collisionInfoA.point.x < other.collisionInfoA.point.x;
		}
		//inline bool operator==(const CollisionPair& other) const {
		//	return ((collisionInfoA.receiver == other.collisionInfoA.receiver) && (collisionInfoB.receiver == other.collisionInfoB.receiver))
		//		||((collisionInfoA.receiver == other.collisionInfoA.rigidbody) && (collisionInfoB.receiver == other.collisionInfoB.rigidbody))
		//		|| ((collisionInfoA.receiver == other.collisionInfoA.rigidbody) && (collisionInfoB.receiver == other.collisionInfoB.receiver))
		//		|| ((collisionInfoA.receiver == other.collisionInfoA.receiver) && (collisionInfoB.receiver == other.collisionInfoB.rigidbody));
		//}
		bool operator==(const CollisionPair& other) const;
	};

	struct MapCollisionPairEqual_to {

		inline bool operator()(const Physic::CollisionPair* lhs, const Physic::CollisionPair* rhs) const {
			return *lhs == *rhs;
		}
	};


	struct MapCollisionPairHasher
	{
		std::size_t operator()(const Physic::CollisionKey& k) const
		{
			using std::size_t;
			using std::hash;

			return hash<int>()((int)k.objA);
		}
	};



	class DebugDrawerPhysicWorld : public btIDebugDraw
	{
	private:
		int m_debugMode;

	public:
		DebugDrawerPhysicWorld();
		virtual ~DebugDrawerPhysicWorld();

		void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
		void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color);
		void reportErrorWarning(const char *warningString);
		void draw3dText(const btVector3 &location, const char *textString);
		void setDebugMode(int debugMode);
		int getDebugMode() const;
	};


	class PhysicManager
	{
		
	private:
		glm::vec3 m_gravity;

		std::unordered_map<CollisionKey, CollisionPair, MapCollisionPairHasher> m_collisionPairUpdateState;

		btDefaultCollisionConfiguration* m_collisionConfiguration;
		btCollisionDispatcher* m_dispatcher;
		btDbvtBroadphase* m_broadPhase;
		btSequentialImpulseConstraintSolver* m_sequentialImpulseConstraintSolver;

		btDiscreteDynamicsWorld* m_physicWorld;

		DebugDrawerPhysicWorld* m_debugDrawerPhysicWorld;

	public:
		PhysicManager(const glm::vec3& _gravity = glm::vec3(0.f,-9.8f,0.f));
		~PhysicManager();
		void clear();

		void setGravity(const glm::vec3& g);
		glm::vec3 getGravity() const;

		btDiscreteDynamicsWorld* getBulletDynamicSimulation() const;

		void update(float deltaTime, const BaseCamera& camera, std::vector<Flag*>& flags, /*Terrain& terrain,*/ std::vector<WindZone*>& windZones, std::vector<ParticleEmitter*>& particleEmitters);
		void update(float deltaTime, const BaseCamera& camera, std::vector<Flag*>& flags, /*Terrain& terrain,*/ std::vector<WindZone*>& windZones, std::vector<ParticleEmitter*>& particleEmitters, bool updateRigidbodies);
		void physicLateUpdate();
		bool onCollisionBegin( btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0, int partId0, int index0, const btCollisionObjectWrapper* colObj1, int partId1, int index1);

		void debugDraw(const glm::mat4& projection, const glm::mat4& view) const;
	};
}
//
//bool operator<(const Physic::CollisionPair& A, const Physic::CollisionPair& B) {
//	return A.collisionInfoB.point.x < B.collisionInfoB.point.x;
//}
