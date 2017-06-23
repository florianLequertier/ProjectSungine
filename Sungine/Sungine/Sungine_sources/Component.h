#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "jsoncpp/json/json.h"

#include "ISerializable.h"
#include "Object.h"

//forward
class Entity;
class Scene;


//Should be called inside all classes which inherite from Component (and are not abstract !), in header file, in class definition.
#define COMPONENT_IMPLEMENTATION_HEADER(ComponentType)\
public:\
virtual void addToSceneAtomic(Scene& scene) override;\
virtual void removeFromSceneAtomic(Scene& scene) override;\
virtual Component* clone(Entity* entity) override;\
private:

//Should be called inside all classes which inherite from Component (and are not abstract !), in .cpp file.
#define COMPONENT_IMPLEMENTATION_CPP(ComponentType)\
void ComponentType::addToSceneAtomic(Scene& scene) override\
{\
	onBeforeComponentAddedToScene(scene);\
	scene.getAccessor().addToScene<ComponentType>(this);\
	onAfterComponentAddedToScene(scene);\
}\
void ComponentType::removeFromSceneAtomic(Scene& scene) override\
{\
	onBeforeComponentErasedFromScene(scene);\
	scene.getAccessor().removeFromScene<ComponentType>(this);\
	onAfterComponentErasedFromScene(scene);\
}\
Component* ComponentType::clone(Entity* entity)\
{\
	GenericHandle& outHandle;\
	ComponentType* newTypedComponent = entity->getScenePtr()->copyAtomic(*this, outHandle);\
	entity->addComponent(newTypedComponent, outHandle);\
	return newTypedComponent;\
}\
Component* ComponentType::eraseFromEntity<ComponentType>(Entity* entity)\
{\
	entity.eraseComponent(this);\
}


class Component : public Object, public ISerializable
{
	CLASS((Component, Object),
	)

public:

protected:
	Entity* m_entity;
	//ComponentType m_type;
	bool m_isActive;

public:
	Component();
	//copying component break the link to the entity, you have to call attachToEntity to be sure to attach the new component to the good entity
	Component(const Component& other);
	//copying component break the link to the entity, you have to call attachToEntity to be sure to attach the new component to the good entity
	Component& operator=(const Component& other);
	virtual ~Component();

	//this function simply set the m_entity parameter on this component.
	//You normally don't have to call this function, it is internally called in entity.add(component).
	//To add a component to an entity, call entity.add(component)
	void setEntityOwner(Entity* entity);
	
	Entity* entity();
	//ComponentType type() const;
	bool getIsActive() const;

	virtual void drawInInspector(Scene& scene) = 0;
	virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) = 0;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat());
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat());

	// function to get component.
	template<typename T>
	T* getComponent(/*Component::ComponentType type*/);
	// function to get components of a certain type.
	template<typename T>
	std::vector<T*> getComponents(/*Component::ComponentType type*/);

	//Erase a component from the scene
	//You normally don't have to directly call this function
	//to erase a component from the scene, call entity.erase(component).
	virtual void removeFromSceneAtomic(Scene& scene) = 0;
	//Add a component to the scene
	//You normally don't have to directly call this function
	//to add a component to the scene, call entity.add(component).
	virtual void addToSceneAtomic(Scene& scene) = 0;

	// Simply call entity.erase(*this). All the logic is done in entity::erase()
	virtual void eraseFromEntity(Entity& entity) = 0;

	// Callbacks for special management when components are created / destroyed
	virtual void onAfterAddedToScene(Scene& scene) {}
	virtual void onAfterAddedToEntity(Entity& entity) {}
	virtual void onAfterRemovedFromEntity(Entity& entity) {}

	virtual void onBeforeAddedToEntity(Entity& entity) {}
	virtual void onBeforeRemovedFromEntity(Entity& entity) {}
	virtual void onBeforeRemovedFromScene(Scene& scene) {}

	//clone a component, and attach it to the given entity
	//This function is internally called by the copy contructor and operator=() of entity, to properly copy the entity.
	virtual Component* clone(Entity* entity) = 0;
};

REGISTER_CLASS(Component)

template<typename T>
T* Component::getComponent(/*Component::ComponentType type*/)
{
	if (m_entity != nullptr)
		return m_entity->getComponent<T>(type);
	else
		return nullptr;
}

template<typename T>
std::vector<T*> Component::getComponents(/*Component::ComponentType type*/)
{
	if (m_entity != nullptr)
		return m_entity->getComponents<T>(type);
	else
		return std::vector<T*>();
}

