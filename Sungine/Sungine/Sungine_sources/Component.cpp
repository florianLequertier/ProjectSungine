

#include "Component.h"
#include "Entity.h"

const std::vector<std::string> Component::ComponentTypeName = { "box collider", "capsule collider", "sphere collider", "mesh collider", "mesh renderer", "point light", "directional light", "spot light", "flag", "particle emitter", "path point", "billboard", "camera", "wind zone", "rigidbody", "animator", "character controller", "reflectivePlane", "componentCount", "behavior", "light", "none" };


Component::Component(Component::ComponentType type) 
	: m_entity(nullptr)
	, m_type(type)
	, m_isActive(true)
{
}

Component::Component(const Component& other) 
	: m_entity(nullptr) // Warning ! You have to reattach the component to an entity after copying it !
	, m_type(other.m_type)
	, m_isActive(other.m_isActive)
{

}

Component& Component::operator=(const Component& other)
{
	m_entity = nullptr;
	m_type = other.m_type;
	m_isActive = other.m_isActive;

	return *this;
}

Component::~Component()
{
	m_entity = nullptr;
}

void Component::attachToEntity(Entity * entity)
{
	m_entity = entity;
}

Entity* Component::entity()
{
	return m_entity;
}

Component::ComponentType Component::type() const
{
	return m_type;
}

bool Component::getIsActive() const
{
	return m_isActive;
}

void Component::applyTransform(const glm::vec3& translation, const glm::vec3& scale, const glm::quat& rotation)
{
	//to override
}

void Component::applyTransformFromPhysicSimulation(const glm::vec3 & translation, const glm::quat & rotation)
{
	//to override
}

void Component::eraseFromEntity(Entity & entity)
{
	entity.erase(this);
}

void Component::addToEntity(Entity & entity)
{
	entity.add(this);
}

void Component::save(Json::Value & componentRoot) const
{
	//m_entity already set
	componentRoot["type"] = (int)m_type;
}

void Component::load(const Json::Value & componentRoot)
{
	//m_entity already set
	m_type = (ComponentType)componentRoot.get("type", 0).asInt();
}
