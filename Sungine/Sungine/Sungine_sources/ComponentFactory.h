#pragma once

#include <map>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Component.h"

#include "Collider.h"
#include "Camera.h"
#include "PathPoint.h"
#include "MeshRenderer.h"
#include "ParticleEmitter.h"
#include "Flag.h"
#include "Lights.h"
#include "WindZone.h"
#include "Billboard.h"
#include "Rigidbody.h"
#include "Animator.h"
#include "CharacterController.h"

class ComponentFactory
{

private:
	std::map<Component::ComponentType, Component*> m_components;

	//for UI : 
	char name[20];
	char path[50];

public:
	void add(const Component::ComponentType& type, Component* component);
	Component* getInstance(const Component::ComponentType& type);
	bool contains(const Component::ComponentType& type);
	void drawUI();
	void drawModalWindow(Entity* entity);

	// singleton implementation :
private:
	ComponentFactory();

public:
	inline static ComponentFactory& get()
	{
		static ComponentFactory instance;

		return instance;
	}


	ComponentFactory(const ComponentFactory& other) = delete;
	void operator=(const ComponentFactory& other) = delete;
};

