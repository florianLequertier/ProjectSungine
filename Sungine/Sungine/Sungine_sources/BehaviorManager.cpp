

#include "BehaviorManager.h"
//forwards :
#include "Entity.h"


BehaviorManager::BehaviorManager()
{
}


BehaviorManager::~BehaviorManager()
{
}

void BehaviorManager::update(Scene& scene, const std::vector<Behavior*> behaviors)
{
	for (int i = 0; i < behaviors.size(); i++) {
		//if this behavior isn't initialized, call start on it, then set it to initialized :
		if (!behaviors[i]->m_isInitialized) {
			behaviors[i]->start(scene);
			behaviors[i]->m_isInitialized = true;
		}
		//apply collisions callbacks on behaviours : 
		auto entity = behaviors[i]->entity();
		switch (entity->getCollisionState()) {
		case Entity::CollisionState::BEGIN :
			behaviors[i]->onCollisionEnter(scene, entity->getCollisionInfo());
			break;
		case Entity::CollisionState::STAY:
			behaviors[i]->onCollisionStay(scene, entity->getCollisionInfo());
			break;
		case Entity::CollisionState::END:
			behaviors[i]->onCollisionEnd(scene, entity->getCollisionInfo());
			break;
		default:
			break;
		}
		//apply the update of behaviours :
		behaviors[i]->update(scene);
	}
}

void BehaviorManager::updateCoroutines(const std::vector<Entity*> entities)
{
	for (auto& e : entities) {
		e->updateCoroutines();
	}
}


