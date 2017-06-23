

#include "Component.h"
#include "Entity.h"

Component::Component() 
	: m_entity(nullptr)
	, m_isActive(true)
{
}

Component::Component(const Component& other) 
	: m_entity(nullptr) // Warning ! You have to reattach the component to an entity after copying it !
	, m_isActive(other.m_isActive)
{

}

Component& Component::operator=(const Component& other)
{
	m_entity = nullptr;
	m_isActive = other.m_isActive;

	return *this;
}

Component::~Component()
{
	m_entity = nullptr;
}

void Component::setEntityOwner(Entity * entity)
{
	m_entity = entity;
}

Entity* Component::entity()
{
	return m_entity;
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