

#include "BehaviorFactory.h"
//forwards : 
#include "Entity.h"


BehaviorFactory::BehaviorFactory()
{

}

void BehaviorFactory::add(const std::string& type, Behavior* behavior)
{
	m_behaviors[type] = behavior;
}

Component* BehaviorFactory::getInstance(const std::string& type)
{
	return m_behaviors[type]->clone(nullptr);
}

bool BehaviorFactory::contains(const std::string& type)
{
	return m_behaviors.find(type) != m_behaviors.end();
}

void BehaviorFactory::drawUI()
{
	ImGui::PushID("behaviorFactory");
	
	//TODO

	ImGui::PopID();
}

void BehaviorFactory::drawModalWindow(Entity* entity)
{
	ImGui::PushID("adBehaviourWindow");

	for (auto& c : m_behaviors)
	{
		//COLLIDER, MESH_RENDERER, LIGHT, POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT, COMPONENT_COUNT, FLAG, PARTICLE_EMITTER, PATH_POINT, CAMERA, WIND_ZONE;
		if (ImGui::Button(c.first.c_str()))
		{
			auto behavior = getInstance(c.first);
			behavior->addToEntity(*entity);
			ImGui::CloseCurrentPopup();
		}
	}

	ImGui::PopID();
}

