

#include "Rigidbody.h"
//forwards : 
#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "PhysicManager.h"

COMPONENT_IMPLEMENTATION_CPP(Rigidbody)

Rigidbody::Rigidbody() : Component(ComponentType::RIGIDBODY),
m_translation(0,0,0), m_scale(1,1,1), m_mass(0), m_inertia(0,0,0), m_isTrigger(false), m_useGravity(true),
m_bulletRigidbody(nullptr), m_motionState(nullptr), m_shape(nullptr), m_ptrToPhysicWorld(nullptr)
{
	for (int i = 0; i < 3; i++) {
		m_frozenAxis[i] = false;
		m_frozenAngles[i] = false;
	}
}

Rigidbody::Rigidbody(const Rigidbody & other): Component(other),
m_translation(other.m_translation), m_scale(other.m_scale), m_mass(other.m_mass), m_inertia(other.m_inertia), m_isTrigger(other.m_isTrigger), m_useGravity(other.m_useGravity),
m_bulletRigidbody(nullptr), m_motionState(nullptr), m_shape(nullptr), m_ptrToPhysicWorld(other.m_ptrToPhysicWorld)
{
	for (int i = 0; i < 3; i++) {
		m_frozenAxis[i] = other.m_frozenAxis[i];
		m_frozenAngles[i] = other.m_frozenAngles[i];
	}
}

Rigidbody & Rigidbody::operator=(const Rigidbody & other)
{
	Component::operator=(other);
	m_translation = other.m_translation;
	m_scale = other.m_scale;
	m_mass = other.m_mass;
	m_inertia = other.m_inertia;
	m_isTrigger = other.m_isTrigger;
	m_useGravity = other.m_useGravity;
	m_bulletRigidbody = nullptr;
	m_motionState = nullptr;
	m_shape = nullptr;
	m_ptrToPhysicWorld = other.m_ptrToPhysicWorld;

	for (int i = 0; i < 3; i++) {
		m_frozenAxis[i] = other.m_frozenAxis[i];
		m_frozenAngles[i] = other.m_frozenAngles[i];
	}

	return *this;
}

Rigidbody::~Rigidbody()
{
	popFromSimulation();

	delete m_bulletRigidbody;
	delete m_motionState;
	delete m_shape;
}

void Rigidbody::pushToSimulation()
{
	if (m_bulletRigidbody != nullptr && !m_bulletRigidbody->isInWorld()) {
		m_ptrToPhysicWorld->addRigidBody(m_bulletRigidbody, Physic::DYNAMIC_GROUP, Physic::DYNAMIC); //TODO
		setUseGravity(m_useGravity);
	}
}

void Rigidbody::popFromSimulation()
{
	//remove btRigidBody from the world :
	if (m_bulletRigidbody != nullptr && m_bulletRigidbody->isInWorld())
		m_ptrToPhysicWorld->removeRigidBody(m_bulletRigidbody);
}

void Rigidbody::makeShape()
{
	//make a shape
	std::vector<Collider*> colliders = getComponents<Collider>(ComponentType::COLLIDER);

	if (m_shape != nullptr)
		delete m_shape;

	m_shape = new btCompoundShape();

	for (auto collider : colliders)
	{
		btCollisionShape* childShape = collider->makeShape();
		btTransform childTransform;
		childTransform.setIdentity();
		childTransform.setOrigin(btVector3(collider->offsetPosition.x, collider->offsetPosition.y, collider->offsetPosition.z));
		//childTransform.setFromOpenGLMatrix(glm::value_ptr(collider->getOffsetMatrix()));

		m_shape->addChildShape(childTransform, childShape);
	}

	//calculate mass, motion state and inertia :
	if (m_mass != 0)
		m_shape->calculateLocalInertia(m_mass, m_inertia);
	
	//if this collider already is in the simulation, update it : 
	if (m_bulletRigidbody != nullptr) {
		popFromSimulation();
			m_bulletRigidbody->setCollisionShape(m_shape);
		pushToSimulation();
	}
}

void Rigidbody::init(btDiscreteDynamicsWorld* physicSimulation)
{
	m_ptrToPhysicWorld = physicSimulation;
	//call it in case rigidbody or ghost object already is in world : 
	popFromSimulation();

	//calculate mass, motion state and inertia :
	if (m_mass != 0)
		m_shape->calculateLocalInertia(m_mass, m_inertia);

	if (m_motionState != nullptr)
		delete m_motionState;
	m_motionState = new MotionState(entity());

	btRigidBody::btRigidBodyConstructionInfo constructorInfo(m_mass, m_motionState, m_shape, m_inertia);

	//create a btRigidBody base on the previous infos :
	if (m_bulletRigidbody != nullptr)
		delete m_bulletRigidbody;
	m_bulletRigidbody = new btRigidBody(constructorInfo);
	m_bulletRigidbody->setUserPointer(this);
	m_bulletRigidbody->setUserIndex(1);
	m_bulletRigidbody->setCollisionFlags(m_bulletRigidbody->getCollisionFlags() /*| btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK*/ /*| btCollisionObject::CF_KINEMATIC_OBJECT*/);
	freezeAxis(m_frozenAxis[0], m_frozenAxis[1], m_frozenAxis[2]);
	freezeAngles(m_frozenAngles[0], m_frozenAngles[1], m_frozenAngles[2]);
	setUseGravity(m_useGravity);
	

	//if the rigidbody already has a collider shape, set it to the rigidbody and push rigidbody to simulation
	//if (m_shape != nullptr) {
	//	popFromSimulation();
	//		m_bulletRigidbody->setCollisionShape(m_shape);
	//}

	//if trigger, disable contact response
	if (m_isTrigger)
		m_bulletRigidbody->setCollisionFlags(m_bulletRigidbody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	else
		m_bulletRigidbody->setCollisionFlags(m_bulletRigidbody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
	
	pushToSimulation();

	popFromSimulation();
	btVector3 newInertia;
	m_bulletRigidbody->getCollisionShape()->calculateLocalInertia(m_mass, newInertia);
	m_bulletRigidbody->setMassProps(m_mass, newInertia);
	m_bulletRigidbody->updateInertiaTensor();
	//m_target->forceActivationState(DISABLE_DEACTIVATION);
	m_bulletRigidbody->activate(true);
	pushToSimulation();
}

float Rigidbody::getMass() const
{
	return m_mass;
}

void Rigidbody::setMass(float mass)
{
	m_mass = mass;

	popFromSimulation();
		btVector3 newInertia;
		m_bulletRigidbody->getCollisionShape()->calculateLocalInertia(m_mass, newInertia);
		m_bulletRigidbody->setMassProps(m_mass, newInertia);
		m_bulletRigidbody->updateInertiaTensor();
		//m_target->forceActivationState(DISABLE_DEACTIVATION);
		m_bulletRigidbody->activate(true);
	pushToSimulation();
}

btVector3 Rigidbody::getInertia() const
{
	return m_inertia;
}

void Rigidbody::setInertia(const glm::vec3 & inertia)
{
	m_inertia.setX(inertia.x);
	m_inertia.setY(inertia.y);
	m_inertia.setZ(inertia.z);
}

void Rigidbody::setPtrToPhysicWorld(btDiscreteDynamicsWorld * ptrToPhysicWorld)
{
	m_ptrToPhysicWorld = ptrToPhysicWorld;
}

void Rigidbody::addColliders(std::vector<Collider*> colliders)
{
	m_colliders.insert(m_colliders.begin(), colliders.begin(), colliders.end());
}

void Rigidbody::addCollider(Collider * collider)
{
	m_colliders.push_back(collider);
}

void Rigidbody::removeCollider(Collider * collider)
{
	m_colliders.erase(std::remove(m_colliders.begin(), m_colliders.end(), collider), m_colliders.end());
}

void Rigidbody::removeAllColliders()
{
	m_colliders.clear();
}

void Rigidbody::freezeAxis(bool x, bool y, bool z)
{
	m_frozenAxis[0] = x;
	m_frozenAxis[1] = y;
	m_frozenAxis[2] = z;
	if (m_bulletRigidbody != nullptr)
		m_bulletRigidbody->setLinearFactor(btVector3((x == true ? 0 : 1), (y == true ? 0 : 1), (z == true ? 0 : 1)));
}

void Rigidbody::freezeAngles(bool x, bool y, bool z)
{
	m_frozenAngles[0] = x;
	m_frozenAngles[1] = y;
	m_frozenAngles[2] = z;
	if (m_bulletRigidbody != nullptr)
		m_bulletRigidbody->setAngularFactor(btVector3((x == true ? 0 : 1), (y == true ? 0 : 1), (z == true ? 0 : 1)));
}

void Rigidbody::setUseGravity(bool useGravity)
{
	m_useGravity = useGravity;
	btVector3 gravity = useGravity == false ? btVector3(0, 0, 0) : (m_ptrToPhysicWorld == nullptr ? btVector3(0, -9.8, 0) : m_ptrToPhysicWorld->getGravity());
	m_bulletRigidbody->setGravity(gravity);
}

void Rigidbody::setIsTrigger(bool state)
{
	m_isTrigger = state;
	popFromSimulation();

	//if trigger, disable contact response
	if (m_isTrigger)
		m_bulletRigidbody->setCollisionFlags(m_bulletRigidbody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	else
		m_bulletRigidbody->setCollisionFlags(m_bulletRigidbody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);

	pushToSimulation();
}

void Rigidbody::onMassChanged()
{
	popfromSimulation();

	// mass changed
	btVector3 newInertia;
	m_bulletRigidbody->getCollisionShape()->calculateLocalInertia(m_mass, newInertia);
	m_bulletRigidbody->setMassProps(m_mass, newInertia);
	m_bulletRigidbody->updateInertiaTensor();
	//m_target->forceActivationState(DISABLE_DEACTIVATION);
	m_bulletRigidbody->activate(true);

	pushToSimulation();
}

void Rigidbody::onPhysicPropChanged()
{
	popFromSimulation();

	// is trigger ?
	//if trigger, disable contact response
	if (m_isTrigger)
		m_bulletRigidbody->setCollisionFlags(m_bulletRigidbody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	else
		m_bulletRigidbody->setCollisionFlags(m_bulletRigidbody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);

	// use gravity ?
	m_useGravity = useGravity;
	btVector3 gravity = useGravity == false ? btVector3(0, 0, 0) : (m_ptrToPhysicWorld == nullptr ? btVector3(0, -9.8, 0) : m_ptrToPhysicWorld->getGravity());
	m_bulletRigidbody->setGravity(gravity);

	// Frozen axis
	if (m_bulletRigidbody != nullptr)
		m_bulletRigidbody->setLinearFactor(btVector3((m_frozenAxis[0] == true ? 0 : 1), (m_frozenAxis[1] == true ? 0 : 1), (m_frozenAxis[2] == true ? 0 : 1)));

	// Frozen angles
	if (m_bulletRigidbody != nullptr)
		m_bulletRigidbody->setAngularFactor(btVector3((m_frozenAngles[0] == true ? 0 : 1), (m_frozenAngles[1] == true ? 0 : 1), (m_frozenAngles[2] == true ? 0 : 1)));

	pushToSimulation();
}

//void Rigidbody::drawInInspector(Scene & scene)
//{
//	float tmpMass = m_mass;
//	if (ImGui::InputFloat("mass", &tmpMass)) {
//		setMass(tmpMass);
//	}
//	if (ImGui::RadioButton("is trigger", m_isTrigger)) {
//		setIsTrigger(!m_isTrigger);
//	}
//	if (ImGui::RadioButton("use gravity", m_useGravity)) {
//		setUseGravity(!m_useGravity);
//	}
//
//	ImGui::BeginChild("linear constraint", ImVec2(0, 22));
//	ImGui::Text("linear constraint");
//	ImGui::SameLine();
//	if (ImGui::RadioButton("x", m_frozenAxis[0]))
//		freezeAxis(!m_frozenAxis[0], m_frozenAxis[1], m_frozenAxis[2]);
//	ImGui::SameLine();
//	if (ImGui::RadioButton("y", m_frozenAxis[1]))
//		freezeAxis(m_frozenAxis[0], !m_frozenAxis[1], m_frozenAxis[2]);
//	ImGui::SameLine();
//	if (ImGui::RadioButton("z", m_frozenAxis[2]))
//		freezeAxis(m_frozenAxis[0], m_frozenAxis[1], !m_frozenAxis[2]);
//	ImGui::EndChild();
//
//	ImGui::BeginChild("angular constraint", ImVec2(0, 22));
//	ImGui::Text("angular constraint");
//	ImGui::SameLine();
//	if (ImGui::RadioButton("x", m_frozenAngles[0]))
//		freezeAngles(!m_frozenAngles[0], m_frozenAngles[1], m_frozenAngles[2]);
//	ImGui::SameLine();
//	if (ImGui::RadioButton("y", m_frozenAngles[1]))
//		freezeAngles(m_frozenAngles[0], !m_frozenAngles[1], m_frozenAngles[2]);
//	ImGui::SameLine();
//	if (ImGui::RadioButton("z", m_frozenAngles[2]))
//		freezeAngles(m_frozenAngles[0], m_frozenAngles[1], !m_frozenAngles[2]);
//	ImGui::EndChild();
//
//}
//
//void Rigidbody::drawInInspector(Scene& scene, const std::vector<Component*>& components)
//{
//	float tmpMass = m_mass;
//	if (ImGui::InputFloat("mass", &tmpMass)) 
//	{
//		for (auto component : components)
//		{
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->setMass(tmpMass);
//		}
//	}
//	if (ImGui::RadioButton("is trigger", m_isTrigger)) 
//	{
//		setIsTrigger(!m_isTrigger);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->setIsTrigger(m_isTrigger);
//		}
//	}
//	if (ImGui::RadioButton("use gravity", m_useGravity)) 
//	{
//		setUseGravity(!m_useGravity);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->setUseGravity(m_useGravity);
//		}
//	}
//
//	ImGui::BeginChild("linear constraint", ImVec2(0, 22));
//	ImGui::Text("linear constraint");
//	ImGui::SameLine();
//	if (ImGui::RadioButton("x", m_frozenAxis[0]))
//	{
//		freezeAxis(!m_frozenAxis[0], m_frozenAxis[1], m_frozenAxis[2]);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->freezeAxis(m_frozenAxis[0], m_frozenAxis[1], m_frozenAxis[2]);
//		}
//	}
//	ImGui::SameLine();
//	if (ImGui::RadioButton("y", m_frozenAxis[1]))
//	{
//		freezeAxis(m_frozenAxis[0], !m_frozenAxis[1], m_frozenAxis[2]);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->freezeAxis(m_frozenAxis[0], m_frozenAxis[1], m_frozenAxis[2]);
//		}
//	}
//	ImGui::SameLine();
//	if (ImGui::RadioButton("z", m_frozenAxis[2]))
//	{
//		freezeAxis(m_frozenAxis[0], m_frozenAxis[1], !m_frozenAxis[2]);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->freezeAxis(m_frozenAxis[0], m_frozenAxis[1], m_frozenAxis[2]);
//		}
//	}
//	ImGui::EndChild();
//
//	ImGui::BeginChild("angular constraint", ImVec2(0, 22));
//	ImGui::Text("angular constraint");
//	ImGui::SameLine();
//	if (ImGui::RadioButton("x", m_frozenAngles[0]))
//	{
//		freezeAngles(!m_frozenAngles[0], m_frozenAngles[1], m_frozenAngles[2]);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->freezeAngles(m_frozenAngles[0], m_frozenAngles[1], m_frozenAngles[2]);
//		}
//	}
//	ImGui::SameLine();
//	if (ImGui::RadioButton("y", m_frozenAngles[1]))
//	{
//		freezeAngles(m_frozenAngles[0], !m_frozenAngles[1], m_frozenAngles[2]);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->freezeAngles(m_frozenAngles[0], m_frozenAngles[1], m_frozenAngles[2]);
//		}
//	}
//	ImGui::SameLine();
//	if (ImGui::RadioButton("z", m_frozenAngles[2]))
//	{
//		freezeAngles(m_frozenAngles[0], m_frozenAngles[1], !m_frozenAngles[2]);
//		for (auto component : components)
//		{
//			if (component == this) continue;
//
//			Rigidbody* castedComponent = static_cast<Rigidbody*>(component);
//			castedComponent->freezeAngles(m_frozenAngles[0], m_frozenAngles[1], m_frozenAngles[2]);
//		}
//	}
//	ImGui::EndChild();
//}

void Rigidbody::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	//update directly the rigidbody with the entity's transform
	btTransform newPhysicTransform;
	newPhysicTransform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
	newPhysicTransform.setOrigin(btVector3(translation.x, translation.y, translation.z));

	//    if(m_target && m_target->isInWorld())
	//    {
	//        popFromSimulation();
	//    }

	m_bulletRigidbody->setWorldTransform(newPhysicTransform); // ???

	//scale :
	if (glm::distance(m_scale, scale) > 0.01f) {
		m_bulletRigidbody->getCollisionShape()->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
		btVector3 newInertia;
		m_bulletRigidbody->getCollisionShape()->calculateLocalInertia(m_mass, newInertia);
		m_bulletRigidbody->setMassProps(m_mass, newInertia);
		m_bulletRigidbody->updateInertiaTensor();
	}

	//add target to simulation :
	//pushToSimulation();

	if(m_bulletRigidbody->isKinematicObject())
		m_ptrToPhysicWorld->updateSingleAabb(m_bulletRigidbody);
	else {
		popFromSimulation();
		pushToSimulation();
	}

	m_bulletRigidbody->activate(true);
	m_ptrToPhysicWorld->updateSingleAabb(m_bulletRigidbody);


	m_translation = translation;
	m_rotation = rotation;
	m_scale = scale;
}

void Rigidbody::applyTransformFromPhysicSimulation(const glm::vec3 & translation, const glm::quat & rotation)
{
	m_translation = translation;
	m_rotation = rotation;
}

void Rigidbody::save(Json::Value & componentRoot) const
{
	Component::save(componentRoot);

	componentRoot["translation"] = toJsonValue(m_translation);
	componentRoot["rotation"] = toJsonValue(m_rotation);
	componentRoot["scale"] = toJsonValue(m_scale);

	componentRoot["mass"] = m_mass*BT_ONE;
	componentRoot["inertia"] = toJsonValue(glm::vec3(m_inertia.x(), m_inertia.y(), m_inertia.z()));
	componentRoot["isTrigger"] = m_isTrigger;
	componentRoot["useGravity"] = m_useGravity;

	for (int i = 0; i < 3; i++)
		componentRoot["frozenAxis"][i] = m_frozenAxis[i];
	for (int i = 0; i < 3; i++)
		componentRoot["frozenAngles"][i] = m_frozenAngles[i];
}

void Rigidbody::load(const Json::Value & componentRoot)
{
	Component::load(componentRoot);

	m_translation = glm::vec3(0, 0, 0); // fromJsonValue<glm::vec3>(componentRoot["translation"], glm::vec3(0, 0, 0));
	m_rotation = glm::quat(); // fromJsonValue<glm::quat>(componentRoot["rotation"], glm::quat());
	m_scale = glm::vec3(1, 1, 1); //fromJsonValue<glm::vec3>(componentRoot["scale"], glm::vec3(1,1,1));

	m_mass = btScalar(componentRoot.get("mass", 0.0f).asFloat());
	glm::vec3 tmpInertia = fromJsonValue<glm::vec3>(componentRoot["inertia"], glm::vec3(0,0,0));
	m_inertia = btVector3(tmpInertia.x, tmpInertia.y, tmpInertia.z);
	m_isTrigger = componentRoot.get("isTrigger", false).asBool();
	m_useGravity = componentRoot.get("useGravity", true).asBool();

	for (int i = 0; i < 3; i++)
		m_frozenAxis[i] = componentRoot["frozenAxis"][i].asBool();
	for (int i = 0; i < 3; i++)
		m_frozenAngles[i] = componentRoot["frozenAngles"][i].asBool();


	m_bulletRigidbody = nullptr;
	m_motionState = nullptr;
	m_shape = nullptr;
	m_ptrToPhysicWorld = nullptr;

	//initialization made when the rigidbody is attached to the entity.

}

void Rigidbody::onAfterAddedToScene(Scene & scene)
{
	makeShape(); //order the ridigbody to reupdate it collider shape
	init(scene.getPhysicManager().getBulletDynamicSimulation()); //must be call after the rigidbody has been attached to an entity
}

void Rigidbody::onAfterAddedToEntity(Entity & entity)
{
	entity.applyTransform();
}

