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

#include "MeshRenderer.h"
#include "Collider.h"
#include "Lights.h"
#include "Flag.h"
#include "ParticleEmitter.h"
#include "PathPoint.h"
#include "Billboard.h"
#include "Camera.h"
#include "WindZone.h"
#include "Rigidbody.h"
#include "Animator.h"
#include "CharacterController.h"
#include "Behavior.h"

#include "glm/gtc/quaternion.hpp"

#include "TransformNode.h"
#include "Coroutine.h"
#include "IDrawableInInspector.h"

//forward
class Component;
class Scene;

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

class Entity : public TransformNode, public IDrawableInInspector
{
public:
	enum CollisionState { BEGIN, STAY, END, NONE };

private:

	Scene* m_scene;

	std::string m_name;
	bool m_isVisible;

	std::vector<Component*> m_components;

	std::vector<Entity*> m_childs;
	Entity* m_parent;

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
	void drawInInspector(Scene& scene);
	void drawInInspector(Scene& scene, const std::vector<IDrawableInInspector*>& selection);

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
	
	Entity& add(Component* component);
	Entity& erase(Component* component);

	bool removeComponentAtomic(Component* component);
	void addComponentAtomic(Component* component);

	//finalyze the creation of the entity, should be called after all components has been added to the entity : 
	//One of the goal of this function is to properly set up the collider such that it cover well all the components of the entity.
	void endCreation();

	//delete all the components attached to this entity.
	void eraseAllComponents();

	// function to get component.
	template<typename T = Component>
	T* getComponent(Component::ComponentType type);
	// function to get components of a certain type.
	template<typename T>
	std::vector<T*> getComponents(Component::ComponentType type);
	//If you want to access to all components in this entity, by types
	void getAllComponentsByTypes(std::map<int, std::vector<Component*>>& outComponents);

	bool hasParent() const;
	bool hasChild() const;
	Entity* getChild(int idx);
	Entity* getParent();
	void setParent(Entity* child);
	void addChild(Entity* child);
	void removeChild(Entity* child);
	void eraseAllChilds();
	int getChildCount() const;

	template<typename R, typename ... Args>
	void startCoroutine(std::function<R(Args...)> action, float callDeltaTime);
	void updateCoroutines();

	void onCollisionBegin(const CollisionInfo& collisionInfo);
	void onCollisionStay(const CollisionInfo& collisionInfo);
	void onCollisionEnd(const CollisionInfo& collisionInfo);

	virtual void save(Json::Value& entityRoot) const override;
	virtual void load(const Json::Value& entityRoot) override;

private:
	void removeParent();
	void addChildAtomic(Entity* child);
	void setParentAtomic(Entity* parent);

	//helper to draw UI : 
	void displayTreeNodeInspector(Scene& scene, std::vector<Component*>& components, int id, bool& hasToRemoveComponent, int& removeId);
	void displayTreeNodeInspector(Scene& scene, Component* component, int id, bool& hasToRemoveComponent, int& removeId);

};

template<typename T>
std::vector<T*> Entity::getComponents(Component::ComponentType type)
{

	std::vector<T*> foundComponents;

	for (int i = 0; i < m_components.size(); i++) {
		if ((m_components[i]->type() & type) != 0) {
			
			if ((type & Component::ComponentType::BEHAVIOR) != 0) {
				T* foundComponent = dynamic_cast<T*>(m_components[i]);
				if(foundComponent != nullptr)
					foundComponents.push_back(foundComponent);
			}
			else {
				assert(dynamic_cast<T*>(m_components[i]) == m_components[i]);
				T* foundComponent = static_cast<T*>(m_components[i]);
				foundComponents.push_back(foundComponent);
			}
		}
	}

	return foundComponents;
}

template<typename R, typename ... Args>
void Entity::startCoroutine(std::function<R(Args...)> action, float callDeltaTime)
{
	auto newCoroutine = new Coroutine<R, Args...>(action, callDeltaTime);
	m_coroutines.insert( std::upper_bound(m_coroutines.begin(), m_coroutines.end(), newCoroutine), newCoroutine );
}

template<typename T = Component>
T* Entity::getComponent(Component::ComponentType type)
{
	if ((type & Component::ComponentType::BEHAVIOR) != 0) {
		for (int i = 0; i < m_components.size(); i++) {
			if ((m_components[i]->type() & Component::ComponentType::BEHAVIOR) != 0) {
				T* foundComponent = dynamic_cast<T*>(m_components[i]);
				if (foundComponent != nullptr)
					return foundComponent;
			}
		}
	}
	else {
		auto findIt = std::find_if(m_components.begin(), m_components.end(), [type](Component* c) { return ((c->type() & type) != 0); });
		if (findIt != m_components.end()){
			assert(dynamic_cast<T*>(*findIt) == *findIt);
			T* foundComponent = static_cast<T*>(*findIt);
			return foundComponent;
		}
	}

	return nullptr;
}

