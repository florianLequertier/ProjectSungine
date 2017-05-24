#include "SceneAccessor.h"
// Forwards : 
#include "Scene.h"

void* SceneAccessor::getSceneComponentContainer(int componentClassId)
{
	return m_scene->m_componentMapping[componentClassId];
}

SceneAccessor& SceneAccessor::addToScene(Entity* entity)
{
	auto& entities = m_scene->m_entities;

	auto findIt = std::find(entities.begin(), entities.end(), entity);

	if (findIt == entities.end())
		entities.push_back(entity);
	else
		PRINT_WARNING("You are trying to put an entity multiple time into the scene.");

	return *this;
}

SceneAccessor& SceneAccessor::removeFromScene(Entity* entity)
{
	auto& entities = m_scene->m_entities;

	//delete entity;
	m_scene->m_entities.erase(std::remove(entities.begin(), entities.end(), entity));

	return *this;
}
