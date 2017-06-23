#pragma once

#include <typeinfo>
#include <typeindex>

#include "Collider.h"
#include "Component.h"
#include "ISerializable.h"

//#define BEHAVIOR(type) inline virtual std::type_index getTypeIndex() const {return std::type_index(typeid(type));}\
//inline virtual Component * clone(Entity * entity) override { \
//	type* behavior = new type(*this);\
//	behavior->attachToEntity(entity);\
//	return behavior; }\
//constexpr int registerBehavior(int toto) { /*BehaviorFactory::get().add(std::type_index(typeid(type)), this);*/ }\

class Scene;

//template <typename T>
//void registerBehavior() {
//	BehaviorFactory::get().add(std::type_index(typeid(T)), this);
//}

class Behavior : public Component
{
	COMPONENT_IMPLEMENTATION_HEADER(Behavior)

	OBJECT_CLASS(Behavior,
		ObjectDescriptor<Behavior>::registerParentClass<Component>()
	)

private:
	friend class BehaviorManager;
	bool m_isInitialized;
public:
	Behavior(/*const std::type_index& type*/);
	virtual ~Behavior();

	//virtual std::type_index getTypeIndex() const = 0;

	virtual void start(Scene& scene);
	virtual void update(Scene& scene);
	virtual void onCollisionEnter(Scene& scene, const CollisionInfo& collisionInfo);
	virtual void onCollisionEnd(Scene& scene, const CollisionInfo& collisionInfo);
	virtual void onCollisionStay(Scene& scene, const CollisionInfo& collisionInfo);

	virtual void drawInInspector(Scene & scene) override;
	virtual void drawInInspector(Scene & scene, const std::vector<Component*>& components) override;

	virtual void save(Json::Value & entityRoot) const override;
	virtual void load(const Json::Value & entityRoot) override;
};

REGISTER_CLASS(Behavior)