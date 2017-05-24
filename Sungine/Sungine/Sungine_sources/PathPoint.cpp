

#include "PathPoint.h"
//forwards : 
#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "Factories.h"
#include "PathManager.h"

COMPONENT_IMPLEMENTATION_CPP(PathPoint)

PathPoint::PathPoint(): Component(ComponentType::PATH_POINT), m_pathId(0), m_pointIdx(0)
{
	
}

PathPoint::~PathPoint()
{

}

glm::vec3 PathPoint::getPosition()
{
	if (m_entity != nullptr)
		return m_entity->getTranslation();
	else
		return glm::vec3(0, 0, 0);
}

int PathPoint::getPointIdx() const
{
	return m_pointIdx;
}

int PathPoint::getPathId() const
{
	return m_pathId;
}

void PathPoint::drawInInspector(Scene& scene)
{
	int oldPathId = m_pathId;
	if (ImGui::InputInt("path id", &m_pathId))
	{
		scene.getPathManager().updatePathId(m_pathId, oldPathId, this);
	}
	if (ImGui::InputInt("point index", &m_pointIdx))
	{
		scene.getPathManager().updatePointIdx(m_pathId);
	}
	if (ImGui::Button("update visual"))
	{
		scene.getPathManager().updateVisual(m_pathId);
	}
}

void PathPoint::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	int oldPathId = m_pathId;
	if (ImGui::InputInt("path id", &m_pathId))
	{
		scene.getPathManager().updatePathId(m_pathId, oldPathId, this);
		for (auto component : components)
		{
			PathPoint* castedComponent = static_cast<PathPoint*>(component);
			castedComponent->m_pathId = m_pathId;
			scene.getPathManager().updatePathId(m_pathId, oldPathId, castedComponent);
		}
	}
	//No reason to use this in multi editing because multiple points in a same path can't have the same index !
	//if (ImGui::InputInt("point index", &m_pointIdx))
	//{
	//	scene.getPathManager().updatePointIdx(m_pathId);
	//}
	if (ImGui::Button("update visual"))
	{
		for (auto component : components)
		{
			PathPoint* castedComponent = static_cast<PathPoint*>(component);
			scene.getPathManager().updateVisual(castedComponent->m_pathId);
		}
	}
}

void PathPoint::save(Json::Value & rootComponent) const
{
	Component::save(rootComponent);

	rootComponent["pathId"] = m_pathId;
	rootComponent["pointIdx"] = m_pointIdx;
}

void PathPoint::load(const Json::Value & rootComponent)
{
	Component::load(rootComponent);

	m_pathId = rootComponent.get("pathId", 0).asInt();
	m_pointIdx = rootComponent.get("pointIdx", 0).asInt();
}
