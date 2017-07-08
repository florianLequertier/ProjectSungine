#pragma once

#include <string>
#include <algorithm>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include "imgui_extension.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

//#include "MeshRenderer.h"
//#include "Collider.h"
//#include "Lights.h"
//#include "Flag.h"
//#include "ParticleEmitter.h"
//#include "PathPoint.h"
//#include "Billboard.h"
//#include "Camera.h"
//#include "WindZone.h"
//#include "Rigidbody.h"
//#include "Animator.h"
//#include "CharacterController.h"
//#include "Behavior.h"

#include "glm/gtc/quaternion.hpp"

#include "Object.h"
#include "ObjectPool.h"
#include "TransformNode.h"
#include "Coroutine.h"
#include "IDrawableInInspector.h"
#include "CollisionInfo.h"

//forward
class Component;
class Scene;
struct CollisionInfo;

/*
class Transform
{
protected :
	glm::vec3 m_translation;
	glm::vec3 m_scale;
	glm::quat m_rotation;
	glm::vec3 m_eulerRotation;

	glm::mat4 m_modelMatrix;
public :
	Transform();
	virtual ~Transform();

	glm::mat4 getModelMatrix();

	glm::vec3 getTranslation();
	glm::vec3 getScale();
	glm::quat getRotation();
	glm::vec3 getEulerRotation();

	void translate(glm::vec3 const& t);
	void setTranslation(glm::vec3 const& t);
	void scale(glm::vec3 const& s);
	void setScale(glm::vec3 const& s);
	void rotate(glm::quat const& q);
	void setRotation(glm::quat const& q);
	void setEulerRotation(glm::vec3 const& q);

	void updateModelMatrix();

	virtual void onChangeModelMatrix() = 0;

};*/

template<typename T>
class Test
{
public:
	T* m_data;
};

class Entity : public TransformNode
{
	friend void Component::setEntityOwner(Entity* entity);

	CLASS((Entity, TransformNode),
		((PRIVATE)
			// Point light parameters
			(std::string, m_name)
			(bool, m_isVisible)
			(std::vector<ObjectPtr<Component>>, m_componentsHandles)
			(std::vector<ObjectPtr<Entity>>, m_childs)
			(ObjectPtr<Entity>, m_parent)
		)
	)

public:
	enum CollisionState { BEGIN, STAY, END, NONE };

private:

	Scene* m_scene;

	//coroutines :
	std::vector<BaseCoroutine*> m_coroutines;

	//for collisions : 
	CollisionState m_collisionState;
	CollisionInfo m_collisionInfo;

	//for editing : 
	bool m_isSelected;

public:
	Entity(Scene* scene);
	Entity(const Entity& other);
	Entity& operator=(const Entity& other);
	virtual ~Entity();

	/////////////////////////////////////////

	Entity();
	~Entity();

	template<typename U>
	const ObjectPtr<U>& getComponent()
	{
		auto foundIt = std::find_if(m_componentsHandles.begin(), m_componentsHandles.end(); [](const ObjectPtr<Component> item) { return item->isA<U>(); });
		if (foundIt != m_componentsHandles.end())
			return ObjectPtr<U>(*foundIt);
		else
			return nullptr;
	}

	template<typename U>
	int getComponents(std::vector<const ObjectPtr<U>&>& outComponents)
	{
		assert(outComponents.size() == 0);
		std::vector<int> tmpIdx;
		int componentsCount = 0;
		int index = 0;
		for (auto& component : m_componentsHandles)
		{
			if (component->isA<U>())
			{
				tmpIdx[index] = componentsCount;
				componentsCount++;
			}
			index++;
		}

		// allocation
		outComponents.reserve(componentsCount);

		// loop only in the minimal range
		for (index = 0; index < componentCount; ++index)
		{
			outComponents.push_back(m_componentsHandles[tmpIdx[index]]);
		}

		return outComponents.size();
	}

	template<typename U>
	void eraseComponent(const ObjectPtr<U>& objectPtr)
	{
		ObjectPtr<Component> componentPtr = objectPtr; //TODO : try without this step, opbjectPtr<Component> should be equal to ObjectPtr<T> if they point to the same object
		auto& foundIt = std::find(componentsHandles.begin(), componentsHandles.end(), componentPtr);
		if (foundIt != componentsHandles.end())
		{
			if (ref)
			{
				ref->onBeforeRemovedFromEntity();
				removeComponentAtomic(std::distance(foundIt, componentsHandles.begin()));
				ref->onAfterRemovedFromEntity();

				ref->onBeforeRemovedFromScene();
				// Will handle assynchronous deletion
				m_scene->destroy(objectPtr);
			}
		}
	}

	template<typename U>
	void eraseComponent(Component* component)
	{
		auto& foundIt = std::find(m_componentsRefs.begin(), m_componentsRefs.end(), component);
		if (foundIt != m_componentsRefs.end())
		{
			if (ref)
			{
				ref->onBeforeRemovedFromEntity();
				removeComponentAtomic(std::distance(foundIt, componentsHandles.begin()));
				ref->onAfterRemovedFromEntity();

				ref->onBeforeRemovedFromScene();
				// Will handle assynchronous deletion
				m_scene->destroy(objectPtr);
			}
		}
	}

	void eraseAllComponents();

	template<typename U>
	U* addNewComponent(ObjectPtr<U>& outPtr)
	{
		outPtr = m_scene->createNew<U>();
		outPtr->m_owner = this; // set ownership
		outPtr->onAfterAddedToScene();

		outPtr->onBeforeAddedToEntity();
		addComponentAtomic(ObjectPtr<Component>(outPtr));
		outPtr->onAfterAddedToEntity();

		outHandle = newHandle;
		return ptr;
	}

	void addComponent(const ObjectPtr<Component>& componentPtr);
	void removeComponentAtomic(int index);
	void addComponentAtomic(const ObjectPtr<Component>& componentPtr);

	Scene* getSceneRef() const;


	////////////////////////////////////////////


	//this function is called each time the model matrix of this entity changes.It internally call apply transform. 
	virtual void onChangeModelMatrix() override;

	//apply transform on this entity, and apply transform on all its components.
	virtual void applyTransform() override;
	//function to apply transform to all children.
	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;
	//apply only rotation and translation (usefull for physic driven transform).
	virtual void applyTransform(const glm::vec3& translation, const glm::quat& rotation = glm::quat()) override;
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat()) override;
	void applyTransformFromPhysicSimulation();

	//draw the entity UI
	void drawInInspector();
	void drawInInspector(const std::vector<void*>& objectInstances);

	//return the value of m_isSelected
	bool getIsSelected() const ;

	CollisionState getCollisionState() const;
	CollisionInfo getCollisionInfo() const;
	void resetCollision();

	//get the name of this entity
	std::string getName() const;
	//set the name of this entity
	void setName(const std::string& name);

	//set the visibility, an entity is visible is at least on of its VisibleComponent is rendered by a camera.
	void setVisibility(bool state);
	//get the visibility, an entity is visible is at least on of its VisibleComponent is rendered by a camera.
	bool getVisibility() const;

	//select this entity, set m_isSelected to true
	void select();
	//deselect this entity, set m_isSelected to false
	void deselect();

	//finalyze the creation of the entity, should be called after all components has been added to the entity : 
	//One of the goal of this function is to properly set up the collider such that it cover well all the components of the entity.
	void endCreation();

	void getAllComponentsByTypes(std::map<int, std::vector<ObjectPtr<Component>&>>& outComponents);

	bool hasParent() const;
	bool hasChild() const;
	const ObjectPtr<Entity>& getChild(int idx);
	const ObjectPtr<Entity>& getParent();
	void clearParent();
	void setParent(const ObjectPtr<Entity>& child);
	void addChild(const ObjectPtr<Entity>& child);
	void detachChild(const ObjectPtr<Entity>& child);
	// Delete all childs from this entity.
	void eraseAllChilds();
	int getChildCount() const;

	template<typename R, typename ... Args>
	void startCoroutine(std::function<R(Args...)> action, float callDeltaTime);
	void updateCoroutines();

	void onCollisionBegin(const CollisionInfo& collisionInfo);
	void onCollisionStay(const CollisionInfo& collisionInfo);
	void onCollisionEnd(const CollisionInfo& collisionInfo);

	//virtual void save(Json::Value& entityRoot) const override;
	//virtual void load(const Json::Value& entityRoot) override;

	virtual void drawInInspector() override;
	virtual void OnBeforeObjectSaved() override;
	virtual void OnAfterObjectLoaded() override;

private:
	// Detach a child from this entity. child deletion must be done by the scene.
	void removeChildAtomic(const ObjectPtr<Entity>& child);
	void addChildAtomic(const ObjectPtr<Entity>& child);
	void setParentAtomic(const ObjectPtr<Entity>& parent);

	//helper to draw UI : 
	void displayTreeNodeInspector(Scene& scene, Component* firstComponent, const std::vector<void*>& components);
	void displayTreeNodeInspector(Scene& scene, Component* component, int id, bool& hasToRemoveComponent, int& removeId);

};

REGISTER_CLASS(Entity)

template<typename R, typename ... Args>
void Entity::startCoroutine(std::function<R(Args...)> action, float callDeltaTime)
{
	auto newCoroutine = new Coroutine<R, Args...>(action, callDeltaTime);
	m_coroutines.insert( std::upper_bound(m_coroutines.begin(), m_coroutines.end(), newCoroutine), newCoroutine );
}
