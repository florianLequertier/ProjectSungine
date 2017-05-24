

#include "CharacterController.h"
//forwards :
#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "PhysicManager.h"

COMPONENT_IMPLEMENTATION_CPP(CharacterController)

CharacterController::CharacterController(): Component(ComponentType::CHARACTER_CONTROLLER),
m_translation(0, 0, 0), m_scale(1, 1, 1), m_height(2.f), m_radius(0.5f), m_jumpFactor(1.f), m_gravityFactor(1.f),
m_isJumping(false), m_isOnGround(true), m_speed(0,0,0), m_force(0,0,0),
m_bulletGhostbody(nullptr), m_shape(nullptr), m_ptrToPhysicWorld(nullptr), m_isGhostInWorld(false)
{
	for (int i = 0; i < 4; i++) {
		m_speedFactor[i] = 1.f;
	}
}

CharacterController::CharacterController(const CharacterController & other) : Component(other),
m_translation(other.m_translation), m_scale(other.m_scale), m_height(other.m_height), m_radius(other.m_radius), m_jumpFactor(other.m_jumpFactor), m_gravityFactor(other.m_gravityFactor),
m_isJumping(other.m_isJumping), m_isOnGround(other.m_isOnGround), m_speed(other.m_speed), m_force(other.m_force),
m_bulletGhostbody(nullptr), m_shape(nullptr), m_ptrToPhysicWorld(other.m_ptrToPhysicWorld), m_isGhostInWorld(false)
{
	for (int i = 0; i < 4; i++) {
		m_speedFactor[i] = other.m_speedFactor[i];
	}
}

CharacterController & CharacterController::operator=(const CharacterController & other)
{
	Component::operator=(other);
	m_translation = other.m_translation;
	m_scale = other.m_scale;
	m_height = other.m_height;
	m_radius = other.m_radius;
	m_jumpFactor = other.m_jumpFactor;
	m_gravityFactor = other.m_gravityFactor;
	m_isJumping = false;
	m_isOnGround = other.m_isOnGround;
	m_speed = other.m_speed;
	m_force = other.m_force;

	m_isGhostInWorld = false;
	m_bulletGhostbody = nullptr;
	m_shape = nullptr;
	m_ptrToPhysicWorld = other.m_ptrToPhysicWorld;

		for (int i = 0; i < 4; i++) {
		m_speedFactor[i] = other.m_speedFactor[i];
	}

	return *this;
}

CharacterController::~CharacterController()
{
	popFromSimulation();

	delete m_bulletGhostbody;
	delete m_shape;
	m_bulletGhostbody = nullptr;
	m_shape = nullptr;
	m_ptrToPhysicWorld = nullptr;
}

void CharacterController::pushToSimulation()
{
	if (m_bulletGhostbody != nullptr && !m_isGhostInWorld){
		m_ptrToPhysicWorld->addCollisionObject(m_bulletGhostbody, Physic::ACTOR_GROUP, Physic::ACTOR);
		m_isGhostInWorld = true;
	}
}

void CharacterController::popFromSimulation()
{
	//remove btRigidBody from the world :
	if (m_bulletGhostbody != nullptr && m_isGhostInWorld) {
		m_ptrToPhysicWorld->removeCollisionObject(m_bulletGhostbody);
		m_isGhostInWorld = false;
	}
}

void CharacterController::makeShape()
{

	if (m_shape != nullptr)
		delete m_shape;

	m_shape = new btCapsuleShape(m_radius, m_height);

	//if this collider already is in the simulation, update it : 
	if (m_bulletGhostbody != nullptr) {
		popFromSimulation();
		m_bulletGhostbody->setCollisionShape(m_shape);
		pushToSimulation();
	}
}

void CharacterController::init(btDiscreteDynamicsWorld* physicSimulation)
{
	m_ptrToPhysicWorld = physicSimulation;
	//call it in case rigidbody or ghost object already is in world : 
	popFromSimulation();

	//create a btRigidBody base on the previous infos :
	if (m_bulletGhostbody != nullptr)
		delete m_bulletGhostbody;
	m_bulletGhostbody = new btPairCachingGhostObject();
	m_bulletGhostbody->setUserPointer(this);
	m_bulletGhostbody->setUserIndex(2); //TODO
	m_bulletGhostbody->setCollisionFlags(m_bulletGhostbody->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);

	if (m_shape!= nullptr)
		delete m_shape;
	m_shape = new btCapsuleShape(m_radius, m_height);
	m_bulletGhostbody->setCollisionShape(m_shape);

	pushToSimulation();
}

void CharacterController::setPtrToPhysicWorld(btDiscreteDynamicsWorld * ptrToPhysicWorld)
{
	m_ptrToPhysicWorld = ptrToPhysicWorld;
}


void CharacterController::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{	
	//update directly the rigidbody with the entity's transform
	btTransform newPhysicTransform;
	newPhysicTransform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
	newPhysicTransform.setOrigin(btVector3(translation.x, translation.y, translation.z));

	m_bulletGhostbody->setWorldTransform(newPhysicTransform);

	 //scale :
	if (glm::distance(m_scale, scale) > 0.01f) {
		m_bulletGhostbody->getCollisionShape()->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
	}

	m_ptrToPhysicWorld->updateSingleAabb(m_bulletGhostbody);

	m_translation = translation;
	m_rotation = rotation;
	m_scale = scale;
}

void CharacterController::applyTransformFromPhysicSimulation(const glm::vec3 & translation, const glm::quat & rotation)
{
	m_translation = translation;
	m_rotation = rotation;
}

void CharacterController::checkBlockingByCollision(glm::vec3& nextPosition)
{
	const btVector3 tickSpeed(nextPosition.x - m_translation.x, nextPosition.y - m_translation.y, nextPosition.z - m_translation.z);
	btVector3 tickSpeedAfterCollisionThroughX(tickSpeed.x(), 0.f, 0.f);
	btVector3 tickSpeedAfterCollisionThroughY(0.f, tickSpeed.y(), 0.f);
	btVector3 tickSpeedAfterCollisionThroughZ(0.f, 0.f, tickSpeed.z());

	btTransform nextTransform_x;
	nextTransform_x.setRotation(btQuaternion(m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w));
	nextTransform_x.setOrigin(btVector3(nextPosition.x, m_translation.y, m_translation.z));

	btTransform nextTransform_y;
	nextTransform_y.setRotation(btQuaternion(m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w));
	nextTransform_y.setOrigin(btVector3(m_translation.x, nextPosition.y, m_translation.z));

	btTransform nextTransform_z;
	nextTransform_z.setRotation(btQuaternion(m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w));
	nextTransform_z.setOrigin(btVector3(m_translation.x, m_translation.y, nextPosition.z));

	btKinematicClosestNotMeConvexResultCallback cb_x(m_bulletGhostbody, btVector3(0.f, 1.f, 0.f), btScalar(0.5f));
	cb_x.m_collisionFilterGroup = m_bulletGhostbody->getBroadphaseHandle()->m_collisionFilterGroup;
	cb_x.m_collisionFilterMask = m_bulletGhostbody->getBroadphaseHandle()->m_collisionFilterMask;
	m_bulletGhostbody->convexSweepTest(m_shape, m_bulletGhostbody->getWorldTransform(), nextTransform_x, cb_x);
	if (cb_x.hasHit())
	{
		const btVector3 hitNormal = cb_x.m_hitNormalWorld;
		const btVector3 hitTangent1 = btCross(hitNormal, btVector3(1, 0, 0));
		const btVector3 hitTangent2 = btCross(hitNormal, hitTangent1);

		const float speedProjectedOnTangent1 = btDot(hitTangent1, tickSpeed);
		const float speedProjectedOnTangent2 = btDot(hitTangent2, tickSpeed);

		tickSpeedAfterCollisionThroughX = speedProjectedOnTangent1 * hitTangent1 + speedProjectedOnTangent2 * hitTangent2;
	}
	
	btKinematicClosestNotMeConvexResultCallback cb_y(m_bulletGhostbody, btVector3(0.f, 1.f, 0.f), btScalar(0.5f));
	cb_y.m_collisionFilterGroup = m_bulletGhostbody->getBroadphaseHandle()->m_collisionFilterGroup;
	cb_y.m_collisionFilterMask = m_bulletGhostbody->getBroadphaseHandle()->m_collisionFilterMask;
	m_bulletGhostbody->convexSweepTest(m_shape, m_bulletGhostbody->getWorldTransform(), nextTransform_y, cb_y);
	if (cb_y.hasHit()) 
	{
		const btVector3 hitNormal = cb_y.m_hitNormalWorld;
		const btVector3 hitTangent1 = btCross(hitNormal, btVector3(1, 0, 0));
		const btVector3 hitTangent2 = btCross(hitNormal, hitTangent1);

		const float speedProjectedOnTangent1 = btDot(hitTangent1, tickSpeed);
		const float speedProjectedOnTangent2 = btDot(hitTangent2, tickSpeed);

		tickSpeedAfterCollisionThroughY = speedProjectedOnTangent1 * hitTangent1 + speedProjectedOnTangent2 * hitTangent2;

		m_isOnGround = true;
	}

	btKinematicClosestNotMeConvexResultCallback cb_z(m_bulletGhostbody, btVector3(0.f, 1.f, 0.f), btScalar(0.5f));
	cb_z.m_collisionFilterGroup = m_bulletGhostbody->getBroadphaseHandle()->m_collisionFilterGroup;
	cb_z.m_collisionFilterMask = m_bulletGhostbody->getBroadphaseHandle()->m_collisionFilterMask;
	m_bulletGhostbody->convexSweepTest(m_shape, m_bulletGhostbody->getWorldTransform(), nextTransform_z, cb_z);
	if (cb_z.hasHit())
	{
		const btVector3 hitNormal = cb_z.m_hitNormalWorld;
		const btVector3 hitTangent1 = btCross(hitNormal, btVector3(1, 0, 0));
		const btVector3 hitTangent2 = btCross(hitNormal, hitTangent1);

		const float speedProjectedOnTangent1 = btDot(hitTangent1, tickSpeed);
		const float speedProjectedOnTangent2 = btDot(hitTangent2, tickSpeed);

		tickSpeedAfterCollisionThroughZ = speedProjectedOnTangent1 * hitTangent1 + speedProjectedOnTangent2 * hitTangent2;
	}

	nextPosition.x = m_translation.x + (tickSpeedAfterCollisionThroughX.x() + tickSpeedAfterCollisionThroughY.x() + tickSpeedAfterCollisionThroughZ.x()) / 3.f;
	nextPosition.y = m_translation.y + (tickSpeedAfterCollisionThroughX.y() + tickSpeedAfterCollisionThroughY.y() + tickSpeedAfterCollisionThroughZ.y()) / 3.f;
	nextPosition.z = m_translation.z + (tickSpeedAfterCollisionThroughX.z() + tickSpeedAfterCollisionThroughY.z() + tickSpeedAfterCollisionThroughZ.z()) / 3.f;
}

void CharacterController::update(float deltaTime)
{
	m_force += m_gravityFactor * glm::vec3(0.f, -1.f, 0.f);

	glm::vec3 nextPosition;
	m_speed += (deltaTime / 1.f)*m_force;
	nextPosition = m_translation + deltaTime*m_speed;

	checkBlockingByCollision(nextPosition);

	if (m_entity != nullptr)
		entity()->setTranslation(nextPosition);

	m_force = glm::vec3(0, 0, 0);
}

void CharacterController::setGravity(float gravityFactor)
{
	m_gravityFactor = gravityFactor;
}

float CharacterController::getGravity() const
{
	return m_gravityFactor;
}

void CharacterController::setSpeed(Direction direction, float speedFactor)
{
	m_speedFactor[direction] = speedFactor;
}

float CharacterController::getSpeed(Direction direction) const
{
	return m_speedFactor[direction];
}

void CharacterController::setSpeed(float speedFactor)
{
	for (int i = 0; i < 4; i++) {
		m_speedFactor[i] = speedFactor;
	}
}

void CharacterController::setJumpFactor(float jumpFactor)
{
	m_jumpFactor = jumpFactor;
}

float CharacterController::getJumpFactor() const
{
	return m_jumpFactor;
}

void CharacterController::setHeight(float height)
{
	m_height = height;
	makeShape();
}

float CharacterController::getHeight() const
{
	return m_height;
}

void CharacterController::setRadius(float radius)
{
	m_radius = radius;
	makeShape();
}

float CharacterController::getRadius() const
{
	return m_radius;
}

void CharacterController::jump()
{
	if (!m_isOnGround)
		return;

	m_isJumping = true;
	m_force += glm::vec3(0, m_jumpFactor, 0);
}

void CharacterController::move(glm::vec3 direction)
{
	float xSpeed = direction.x > 0 ? m_speedFactor[Direction::RIGHT] : m_speedFactor[Direction::LEFT];
	float zSpeed = direction.z > 0 ? m_speedFactor[Direction::FORWARD]: m_speedFactor[Direction::BACKWARD];
	float ySpeed = 1.f;

	m_force += glm::vec3(direction.x * xSpeed, direction.y * ySpeed, direction.z * zSpeed);
}

void CharacterController::drawInInspector(Scene & scene)
{
	ImGui::InputFloat("forward speed", &m_speedFactor[Direction::FORWARD]);
	ImGui::InputFloat("backward speed", &m_speedFactor[Direction::BACKWARD]);
	ImGui::InputFloat("right speed", &m_speedFactor[Direction::RIGHT]);
	ImGui::InputFloat("left speed", &m_speedFactor[Direction::LEFT]);

	ImGui::InputFloat("gravity", &m_gravityFactor);
	ImGui::InputFloat("jump", &m_jumpFactor);

	float tmpFloat = m_height;
	if (ImGui::InputFloat("height", &tmpFloat)) {
		setHeight(tmpFloat);
	}
	tmpFloat = m_radius;
	if (ImGui::InputFloat("radius", &tmpFloat)) {
		setRadius(tmpFloat);
	}
}

void CharacterController::drawInInspector(Scene & scene, const std::vector<Component*>& components)
{
	if (ImGui::InputFloat("forward speed", &m_speedFactor[Direction::FORWARD]))
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->m_speedFactor[Direction::FORWARD] = m_speedFactor[Direction::FORWARD];
		}
	}
	if (ImGui::InputFloat("backward speed", &m_speedFactor[Direction::BACKWARD]))
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->m_speedFactor[Direction::BACKWARD] = m_speedFactor[Direction::BACKWARD];
		}
	}
	if (ImGui::InputFloat("right speed", &m_speedFactor[Direction::RIGHT]))
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->m_speedFactor[Direction::RIGHT] = m_speedFactor[Direction::RIGHT];
		}
	}
	if (ImGui::InputFloat("left speed", &m_speedFactor[Direction::LEFT]))
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->m_speedFactor[Direction::LEFT] = m_speedFactor[Direction::LEFT];
		}
	}

	if (ImGui::InputFloat("gravity", &m_gravityFactor))
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->m_gravityFactor = m_gravityFactor;
		}
	}
	if (ImGui::InputFloat("jump", &m_jumpFactor))
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->m_jumpFactor = m_jumpFactor;
		}
	}

	float tmpFloat = m_height;
	if (ImGui::InputFloat("height", &tmpFloat)) 
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->setHeight(tmpFloat);
		}
	}
	tmpFloat = m_radius;
	if (ImGui::InputFloat("radius", &tmpFloat)) 
	{
		for (auto component : components)
		{
			CharacterController* castedComponent = static_cast<CharacterController*>(component);
			castedComponent->setRadius(tmpFloat);
		}
	}
}

void CharacterController::save(Json::Value & componentRoot) const
{
	Component::save(componentRoot);

	componentRoot["translation"] = toJsonValue(m_translation);
	componentRoot["rotation"] = toJsonValue(m_rotation);
	componentRoot["scale"] = toJsonValue(m_scale);
	componentRoot["height"] = m_height;
	componentRoot["radius"] = m_radius;
	componentRoot["jumpFactor"] = m_jumpFactor;
	componentRoot["gravityFactor"] = m_gravityFactor;
	componentRoot["isJumping"] = m_isJumping;
	componentRoot["isOnGround"] = m_isOnGround;

	componentRoot["speed"] = toJsonValue(m_speed);
	componentRoot["force"] = toJsonValue(m_force);

	for (int i = 0; i < 4; i++) {
		componentRoot["speedFactor"][i] = m_speedFactor[i];
	}
}

void CharacterController::load(const Json::Value & componentRoot)
{
	Component::load(componentRoot);

	m_translation = fromJsonValue<glm::vec3>(componentRoot["translation"], glm::vec3(0, 0, 0));
	m_rotation = fromJsonValue<glm::quat>(componentRoot["rotation"], glm::quat());
	m_scale = fromJsonValue<glm::vec3>(componentRoot["scale"], glm::vec3(0, 0, 0));
	m_height = componentRoot.get("height", 2.f).asFloat();
	m_radius = componentRoot.get("radius", 0.5f).asFloat();
	m_jumpFactor = componentRoot.get("jumpFactor", 1.f).asFloat();
	m_gravityFactor = componentRoot.get("gravityFactor", 1.f).asFloat();
	m_isJumping = componentRoot.get("isJumping", false).asBool();
	m_isOnGround = componentRoot.get("isOnGround", true).asBool();

	m_speed = fromJsonValue<glm::vec3>(componentRoot["speed"], glm::vec3(0, 0, 0));
	m_force = fromJsonValue<glm::vec3>(componentRoot["force"], glm::vec3(0, 0, 0));

	for (int i = 0; i < 4; i++) {
		m_speedFactor[i] = componentRoot["speedFactor"][i].asFloat();
	}

	m_isGhostInWorld = false;
	m_bulletGhostbody = nullptr;
	m_shape = nullptr;
	m_ptrToPhysicWorld = nullptr;
}

void CharacterController::onAfterComponentAddedToScene(Scene& scene)
{
	makeShape(); 
	init(scene.getPhysicManager().getBulletDynamicSimulation());
}

void CharacterController::onAfterComponentAddedToEntity(Entity& entity)
{
	entity.applyTransform();
}