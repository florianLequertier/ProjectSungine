

#include "ComponentFactory.h"
//forwards : 
#include "Factories.h"
#include "ReflectivePlane.h"

ComponentFactory::ComponentFactory()
{
	//COLLIDER, MESH_RENDERER, LIGHT, POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT, COMPONENT_COUNT, FLAG, PARTICLE_EMITTER, PATH_POINT, CAMERA, WIND_ZONE, RIGIDBODY;
	add(Component::ComponentType::BOX_COLLIDER, new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe")));
	add(Component::ComponentType::CAPSULE_COLLIDER, new CapsuleCollider());
	add(Component::ComponentType::SPHERE_COLLIDER, new CapsuleCollider()); //TODO
	add(Component::ComponentType::MESH_COLLIDER, new CapsuleCollider()); //TODO
	add(Component::ComponentType::MESH_RENDERER, new MeshRenderer());
	add(Component::ComponentType::POINT_LIGHT, new PointLight());
	add(Component::ComponentType::DIRECTIONAL_LIGHT, new DirectionalLight());
	add(Component::ComponentType::SPOT_LIGHT, new SpotLight());
	add(Component::ComponentType::FLAG, new Physic::Flag(getMaterialFactory().getDefault("defaultLit")));
	add(Component::ComponentType::PARTICLE_EMITTER, new Physic::ParticleEmitter());
	add(Component::ComponentType::PATH_POINT, new PathPoint());
	add(Component::ComponentType::CAMERA, new Camera());
	add(Component::ComponentType::WIND_ZONE, new Physic::WindZone());
	add(Component::ComponentType::BILLBOARD, new Billboard());
	add(Component::ComponentType::RIGIDBODY, new Rigidbody());
	add(Component::ComponentType::ANIMATOR, new Animator());
	add(Component::ComponentType::CHARACTER_CONTROLLER, new CharacterController());
	add(Component::ComponentType::REFLECTIVE_PLANE, new ReflectivePlane());

	//assert(m_components.size() == Component::ComponentType::INTERNAL_COMPONENT_COUNT);
}

void ComponentFactory::add(const Component::ComponentType& type, Component* component)
{
	m_components[type] = component;
}

Component* ComponentFactory::getInstance(const Component::ComponentType& type)
{
	return m_components[type]->clone(nullptr);
}

bool ComponentFactory::contains(const Component::ComponentType& type)
{
	return m_components.find(type) != m_components.end();
}

void ComponentFactory::drawUI()
{
	ImGui::PushID("componentFactory");

	for (auto& c : m_components)
	{
		//COLLIDER, MESH_RENDERER, LIGHT, POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT, COMPONENT_COUNT, FLAG, PARTICLE_EMITTER, PATH_POINT, CAMERA, WIND_ZONE;
		ImGui::Text(Component::ComponentTypeName[flagBitToInt(c.first)].c_str());
	}

	ImGui::PopID();
}

void ComponentFactory::drawModalWindow(Entity* entity)
{
	ImGui::PushID("addComponentWindow");

	for (auto& c : m_components)
	{
		//COLLIDER, MESH_RENDERER, LIGHT, POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT, COMPONENT_COUNT, FLAG, PARTICLE_EMITTER, PATH_POINT, CAMERA, WIND_ZONE;
		if (ImGui::Button(Component::ComponentTypeName[flagBitToInt(c.first)].c_str()))
		{
			auto component = getInstance(c.first);
			component->addToEntity(*entity);
			ImGui::CloseCurrentPopup();
		}
	}

	ImGui::PopID();
}

