

#include "Behavior.h"
//forwards : 
#include "BehaviorFactory.h"
#include "Scene.h"

Behavior::Behavior(/*const std::type_index& type*/) : Component(ComponentType::BEHAVIOR), m_isInitialized(false)
{
	//static bool initialized = false;
	//if (!initialized) {
	//	BehaviorFactory::get().add(type, this);
	//	initialized = true;
	//}
}

Behavior::~Behavior()
{
}

void Behavior::start(Scene& scene)
{
	//to override
}

void Behavior::update(Scene& scene)
{
	//to override
}

void Behavior::onCollisionEnter(Scene& scene, const CollisionInfo& collisionInfo)
{
	//to override
}

void Behavior::onCollisionEnd(Scene& scene, const CollisionInfo& collisionInfo)
{
	//to override
}

void Behavior::onCollisionStay(Scene& scene, const CollisionInfo& collisionInfo)
{
	//to override
}

void Behavior::drawInInspector(Scene & scene)
{
	//to override
}

void Behavior::drawInInspector(Scene & scene, const std::vector<Component*>& components)
{
	//to override
}

void Behavior::save(Json::Value & entityRoot) const
{
	Component::save(entityRoot);
	//entityRoot["typeIndexName"] = getTypeIndex().name(); //TODO CORE
}

void Behavior::load(const Json::Value & entityRoot)
{
	Component::load(entityRoot);
}
