#pragma once

#include <map>
#include <typeinfo>
#include <typeindex>
#include "Behavior.h"

class Entity;

class BehaviorFactory
{
private:
	std::map<std::string, Behavior*> m_behaviors;

	//for UI : 
	char name[20];
	char path[50];

public:
	void add(const std::string& type, Behavior* behavior);
	Component* getInstance(const std::string& type);
	bool contains(const std::string& type);
	void drawUI();
	void drawModalWindow(Entity* entity);

	template<typename T>
	inline bool registerInFactory() {
		//BehaviorFactory::get().add(std::type_index(typeid(T)).name(), new T()); //TODO CORE
		return true;
	}

	// singleton implementation :
private:
	BehaviorFactory();

public:
	inline static BehaviorFactory& get()
	{
		static BehaviorFactory instance;

		return instance;
	}


	BehaviorFactory(const BehaviorFactory& other) = delete;
	void operator=(const BehaviorFactory& other) = delete;
};

//#define BEHAVIOR_INIT(type)	static bool initialized = false; \
//if (!initialized) {\
//	BehaviorFactory::get().add(std::type_index(typeid(type)), this);\
//	initialized = true; }\

//#define BEHAVIOR_INIT(type) constexpr void registerBehaviorToFactory(){	BehaviorFactory::get().add(std::type_index(typeid(type)), this); }\

#define REGISTER_BEHAVIOUR(type) static bool isRegister = BehaviorFactory::get().registerInFactory<type>();