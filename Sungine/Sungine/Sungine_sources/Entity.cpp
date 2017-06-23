

#include "Entity.h"
//forwards : 
#include "Component.h"
#include "Scene.h"
#include "SceneAccessor.h"
#include "ComponentFactory.h"
#include "BehaviorFactory.h"
#include "PhysicManager.h"
#include "Application.h"

Entity::Entity(Scene* scene) 
	: TransformNode()
	, m_scene(scene)
	, m_isSelected(false)
	, m_name("default_entity")
	, m_parent(nullptr)
	, m_isVisible(false)
{
	scene->getAccessor().addToScene(this);
}

Entity::Entity(const Entity& other) 
	: TransformNode(other)
	, m_isSelected(other.m_isSelected)
	, m_name(other.m_name)
	, m_scene(other.m_scene)
	, m_parent(other.m_parent)
	, m_isVisible(other.m_isVisible)
{
	m_scene->getAccessor().addToScene(this);

	for (int i = 0; i < other.m_componentsRefs.size(); i++)
	{
		other.m_componentsRefs[i]->clone(this); // clone function will directly attach the component to the new entity
	}
	eraseAllChilds();
	for (int i = 0; i < other.m_childs.size(); i++)
	{
		auto newEntity = new Entity(*other.m_childs[i]);
		addChild(newEntity);
	}

	updateModelMatrix();
}

Entity& Entity::operator=(const Entity& other)
{
	TransformNode::operator=(other);

	m_isSelected = other.m_isSelected;
	m_name = other.m_name;
	m_isVisible = other.m_isVisible;
	m_scene = other.m_scene;
	m_parent = other.m_parent;

	m_scene->getAccessor().addToScene(this);

	eraseAllComponents();

	for (int i = 0; i < other.m_componentsRefs.size(); i++)
	{
		//copy the component
		//auto newComponent = other.m_componentsRefs[i]->clone(this);
		//addComponentFromExisting(*other.m_componentsRefs[i], newComponent);

		other.m_componentsRefs[i]->clone(this); // clone function will directly attach the component to the new entity
	}
	eraseAllChilds();
	for (int i = 0; i < other.m_childs.size(); i++)
	{
		auto newEntity = new Entity(*other.m_childs[i]);
		addChild(newEntity);
	}

	return *this;
}

Entity::~Entity()
{
	setParent(nullptr);

	eraseAllComponents();

	eraseAllChilds();
}

/////////////////////////////////////////////////////////////

void Entity::eraseAllComponents()
{
	const int size = m_componentsRefs.size();
	for (int i = 0; i < size; i++)
	{
		Component* ref = m_componentsRefs[i];
		GenericHandle& handle = m_componentsHandles[i];
		auto pool = m_scene->getPool(handle.getClassTypeId());

		ref->onBeforeRemovedFromEntity(*this);
		removeComponentAtomic(i);
		ref->onAfterRemovedFromEntity(*this);

		ref->onBeforeRemovedFromScene(*m_scene);
		pool->deallocate(handle.getIndex(), handle.getGeneration());
	}
}

void Entity::removeComponentAtomic(int index)
{
	std::iter_swap(m_componentsRefs.begin() + index, m_componentsRefs.end() - 1);
	m_componentsRefs.erase(m_componentsRefs.end() - 1);

	std::iter_swap(m_componentsHandles.begin() + index, m_componentsHandles.end() - 1);
	m_componentsHandles.erase(m_componentsHandles.end() - 1);

	assert(m_componentsRefs.size() == m_componentsHandles.size());
}

void Entity::addComponentAtomic(Component* componentPtr, const GenericHandle& handle)
{
	m_componentsRefs.push_back(componentPtr);
	m_componentsHandles.push_back(handle);

	assert(m_componentsRefs.size() == m_componentsHandles.size());
}

Scene* Entity::getSceneRef() const
{
	return m_scene;
}

/////////////////////////////////////////////////////////////

void Entity::onChangeModelMatrix()
{
	applyTransform();
}

void Entity::applyTransform()
{
	//if (collider != nullptr)
	//	collider->applyTransform(m_translation, m_scale);

	for (auto& c : m_componentsRefs)
	{
		c->applyTransform(m_translation, m_scale, m_rotation);
	}
	for (auto& e : m_childs)
	{
		e->applyTransform(m_translation, m_scale, m_rotation);
		//e->applyTransform(); //recursivity on all childs
	}
}

void Entity::applyTransform(const glm::vec3 & parentTranslation, const glm::vec3 & parentScale, const glm::quat & parentRotation)
{
	m_translation = parentRotation * m_localTranslation + parentTranslation;
	m_scale = m_localScale * parentScale;
	m_rotation = m_localRotation * parentRotation;

	//glm::mat4 updateMatrix = glm::translate(glm::mat4(1), parentTranslation);// *glm::mat4_cast(parentRotation);// *glm::scale(glm::mat4(1), parentScale);

	////combine transforms to get world model matrix of the model
	////and make the child launch applyTransform recursivly
	setParentTransform(parentTranslation, parentScale, parentRotation);
	updateModelMatrix();
}

void Entity::applyTransform(const glm::vec3 & parentTranslation, const glm::quat & parentRotation)
{
	m_translation = parentRotation * m_localTranslation + parentTranslation;
	m_rotation = m_localRotation * parentRotation;

	//glm::mat4 updateMatrix = glm::translate(glm::mat4(1), parentTranslation);// *glm::mat4_cast(parentRotation);// *glm::scale(glm::mat4(1), parentScale);

	////combine transforms to get world model matrix of the model
	////and make the child launch applyTransform recursivly
	setParentTransform(parentTranslation, parentRotation);
	updateModelMatrix();
}

void Entity::applyTransformFromPhysicSimulation(const glm::vec3 & translation, const glm::quat & rotation)
{
	//setTranslation : 
	m_translation = translation;
	m_localTranslation = translation;

	//setRotation : 
	m_rotation = rotation;
	m_localRotation = rotation;

	//updateModelMatrix : 
	m_translation = m_parentRotation * m_localTranslation + m_parentTranslation;
	m_scale = m_localScale * m_parentScale;
	m_rotation = m_localRotation * m_parentRotation;
	m_modelMatrix = glm::translate(glm::mat4(1), m_translation) * glm::mat4_cast(m_rotation) * glm::scale(glm::mat4(1), m_scale);

	//onChangeModelMatrix : 
	applyTransformFromPhysicSimulation();


	//m_translation = parentRotation * m_localTranslation + parentTranslation;
	//m_rotation = m_localRotation * parentRotation;

	////glm::mat4 updateMatrix = glm::translate(glm::mat4(1), parentTranslation);// *glm::mat4_cast(parentRotation);// *glm::scale(glm::mat4(1), parentScale);

	//////combine transforms to get world model matrix of the model
	//////and make the child launch applyTransform recursivly
	//setParentTransform(parentTranslation, parentRotation);
	////updateModelMatrix();
	//m_translation = m_parentRotation * m_localTranslation + m_parentTranslation;
	//m_scale = m_localScale * m_parentScale;
	//m_rotation = m_localRotation * m_parentRotation;

	//m_modelMatrix = glm::translate(glm::mat4(1), m_translation) * glm::mat4_cast(m_rotation) * glm::scale(glm::mat4(1), m_scale);


	//set transformations to components and childs :
	applyTransformFromPhysicSimulation();
}

void Entity::applyTransformFromPhysicSimulation()
{
	for (auto& c : m_componentsRefs)
	{
		c->applyTransformFromPhysicSimulation(m_translation, m_rotation);
	}
	for (auto& e : m_childs)
	{
		e->applyTransformFromPhysicSimulation(m_translation, m_rotation);
		//e->applyTransform(); //recursivity on all childs
	}
}

void Entity::displayTreeNodeInspector(Scene& scene, Component* component, int id, bool& hasToRemoveComponent, int& removeId)
{
	ImGui::SetNextWindowContentWidth(80);
	bool nodeOpen = false;

	ImVec2 itemPos;
	ImVec2 itemSize;
	if (ImGui::MyTreeNode("", itemPos, itemSize))
		nodeOpen = true;
	ImGui::SameLine();


	ImGui::Text(component->getClassName().c_str());//, ImVec2(itemSize.x /*m_bottomLeftPanelRect.z*/ - 36.f, itemSize.y /*16.f*/)))

	ImGui::SameLine();
	if (ImGui::Button("remove"))
	{
		hasToRemoveComponent = true;
		removeId = id;
	}

	if (nodeOpen) {
		//ImGui::SetNextWindowContentWidth(500);
		//ImGui::BeginChild(ImGuiID(id), ImVec2(500, 0), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_ChildWindowAutoFitY | ImGuiWindowFlags_ChildWindowAutoFitX);
		component->drawInInspector(scene);
		//ImGui::EndChild();
	}

	if (nodeOpen)
		ImGui::TreePop();
}

void Entity::displayTreeNodeInspector(Scene& scene, std::vector<Component*>& components, int id, bool& hasToRemoveComponent, int& removeId)
{
	if (components.size() == 0)
		return;

	ImGui::SetNextWindowContentWidth(80);
	bool nodeOpen = false;

	ImVec2 itemPos;
	ImVec2 itemSize;
	if (ImGui::MyTreeNode("", itemPos, itemSize))
		nodeOpen = true;
	ImGui::SameLine();

	ImGui::Text(components[0]->getClassName().c_str());

	if (nodeOpen) {
		components[0]->drawInInspector(scene, components);
	}

	if (nodeOpen)
		ImGui::TreePop();
}

void Entity::drawInInspector(Scene& scene)
{
	char tmpName[20];
	m_name.copy(tmpName, m_name.size(), 0);
	tmpName[m_name.size()] = '\0';
	if (ImGui::InputText("name", tmpName, 20))
	{
		m_name = tmpName;
	}

	TransformNode::drawUI(hasParent());

	bool hasToRemoveComponent = false;
	int removeId = 0;
	//bool expendComponent = false;
	for (int i = 0; i < m_componentsRefs.size(); i++)
	{
		ImGui::PushID(i);
		//ImGui::Separator();
		///*const float frame_height = 10;
		//ImRect bb = ImRect(ImGui::GetCurrentWindow()->DC.CursorPos, ImVec2(ImGui::GetCurrentWindow()->Pos.x + ImGui::GetContentRegionMax().x, ImGui::GetCurrentWindow()->DC.CursorPos.y + frame_height));
		//bool hovered, held;
		//const ImGuiID id = ImGui::GetCurrentWindow()->DC. ->GetID(str_id);
		//bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_NoKeyModifiers);
		//ImGui::RenderCollapseTriangle(bb.Min, false);*/
		//if (ImGui::Button("expend"))
		//	expendComponent = true;
		//ImGui::SameLine();
		//ImGui::Text(Component::ComponentTypeName[m_componentsRefs[i]->type()].c_str());
		//ImGui::SameLine();
		//if (ImGui::Button("remove"))
		//{
		//	hasToRemoveComponent = true;
		//	removeId = i;
		//}	
		//ImGui::Separator();
		//if(expendComponent)
		//	m_componentsRefs[i]->drawUI(scene);
		displayTreeNodeInspector(scene, m_componentsRefs[i], i, hasToRemoveComponent, removeId);

		ImGui::PopID();

		//expendComponent = false;
	}
	if(hasToRemoveComponent)
		m_componentsRefs[removeId]->eraseFromEntity(*this);

	if (ImGui::Button("add component"))
		ImGui::OpenPopup("add component window");

	bool addComponentWindowOpened = true;

	if (ImGui::BeginPopupModal("add component window", &addComponentWindowOpened))
	{
		ComponentFactory::get().drawModalWindow(this);
		BehaviorFactory::get().drawModalWindow(this);

		ImGui::EndPopup();
	}

}

void extractAllComponents(const std::vector<Entity*>& entitiesIn, std::map<int, std::vector<Component*>>& componentsOut)
{
	for (auto& entity : entitiesIn)
	{
		entity->getAllComponentsByTypes(componentsOut);
	}
}

void Entity::drawInInspector(Scene& scene, const std::vector<IDrawableInInspector*>& selection)
{
	char tmpName[20];
	m_name.copy(tmpName, m_name.size(), 0);
	tmpName[m_name.size()] = '\0';
	if (ImGui::InputText("name", tmpName, 20))
	{
		for (int i = 0; i < selection.size(); i++)
		{
			Entity* entity = static_cast<Entity*>(selection[i]);
			entity->m_name = tmpName;
		}
	}

	TransformNode::drawInInspector(hasParent(), selection);

	bool hasToRemoveComponent = false;
	int removeId = 0;

	std::map<int, std::vector<Component*>> outComponents;
	for (int i = 0; i < selection.size(); i++)
	{
		Entity* entity = static_cast<Entity*>(selection[i]);
		entity->getAllComponentsByTypes(outComponents);
	}

	for (auto& componentsByType : outComponents)
	{
		ImGui::PushID(componentsByType.first);
		displayTreeNodeInspector(scene, componentsByType.second, componentsByType.first, hasToRemoveComponent, removeId);
		ImGui::PopID();
	}
	
}

bool Entity::getIsSelected() const
{
	return m_isSelected;
}

Entity::CollisionState Entity::getCollisionState() const
{
	return m_collisionState;
}

CollisionInfo Entity::getCollisionInfo() const
{
	return m_collisionInfo;
}

void Entity::resetCollision()
{
	m_collisionInfo.receiver = nullptr;
	m_collisionInfo.rigidbody = nullptr;

	m_collisionState = CollisionState::NONE;
}

std::string Entity::getName() const
{
	return m_name;
}

void Entity::setName(const std::string & name)
{
	m_name = name;
}

void Entity::setVisibility(bool state)
{
	m_isVisible = state;
}

bool Entity::getVisibility() const
{
	return m_isVisible;
}

void Entity::select()
{
	m_isSelected = true;
}

void Entity::deselect()
{
	m_isSelected = false;
}

void Entity::endCreation()
{
	Collider* colliderComponent = static_cast<Collider*>(getComponent<Collider>());
	if (colliderComponent != nullptr)
	{
		// if a component containing a mesh is present in the entity, cover it with the collider : 
		Physic::Flag* flagComponent = static_cast<Physic::Flag*>(getComponent<Physic::Flag>());
		MeshRenderer* meshRendererComponent = static_cast<MeshRenderer*>(getComponent<MeshRenderer>());
		if (flagComponent != nullptr)
		{
			colliderComponent->setOrigin(flagComponent->getOrigin());
			colliderComponent->coverMesh(flagComponent->getMesh());
			colliderComponent->addOffsetScale(glm::vec3(0.f, 0.f, 0.2f));
		}
		else if (meshRendererComponent != nullptr)
		{
			colliderComponent->setOrigin(meshRendererComponent->getOrigin());
			colliderComponent->coverMesh(*meshRendererComponent->getMesh());
		}
	}
}

void Entity::getAllComponentsByTypes(std::map<int, std::vector<Component*>>& outComponents)
{
	for (int i = 0; i < m_componentsRefs.size(); i++)
	{
		outComponents[m_componentsRefs[i]->getClassId()].push_back(m_componentsRefs[i]);
	}
}

bool Entity::hasParent() const
{
	return m_parent != nullptr;
}

bool Entity::hasChild() const
{
	return m_childs.size() > 0;
}

Entity* Entity::getChild(int idx)
{
	return m_childs[idx];
}

Entity* Entity::getParent()
{
	return m_parent;
}

void Entity::setParent(Entity* parent)
{
	if (m_parent != nullptr)
		removeParent();

	setParentAtomic(parent);
	if(m_parent != nullptr)
		m_parent->addChildAtomic(this);
}

void Entity::addChild(Entity* child)
{
	child->removeParent();

	child->setParentAtomic(this);
	addChildAtomic(child);
}

void Entity::removeParent()
{
	if(m_parent != nullptr)
		m_parent->removeChild(this);
	m_parent = nullptr;
}

void Entity::removeChild(Entity* child)
{
	if (m_childs.size() > 0) //has childs ?
	{
		auto findIt = std::find(m_childs.begin(), m_childs.end(), child);
		if (findIt != m_childs.end())
			m_childs.erase(findIt);
	}
}

void Entity::eraseAllChilds()
{
	while(m_childs.size() > 0)
	{
		m_scene->getAccessor().removeFromScene(m_childs.back());
		delete m_childs.back();
		m_childs.back() = nullptr;
		//m_childs[i] = nullptr;
	}
	if(m_childs.size() > 0)
		m_childs.clear();
}

int Entity::getChildCount() const
{
	return m_childs.size();
}

void Entity::updateCoroutines()
{
	float currentTime = Application::get().getTime();
	for (auto& c : m_coroutines) {
		if (c->getNextExecutionTime() <= currentTime)
			c->execute(currentTime);
	}
}

void Entity::onCollisionBegin(const CollisionInfo & collisionInfo)
{
	//TODO speed up this operation
	//auto behaviors = getComponents<Behavior>(Component::ComponentType::BEHAVIOR);
	//for (auto& b : behaviors) {
	//	b->onColliderEnter(collisionInfo);
	//}
	m_collisionState = CollisionState::BEGIN;
	m_collisionInfo = collisionInfo;
}

void Entity::onCollisionStay(const CollisionInfo & collisionInfo)
{
	//TODO speed up this operation
	//auto behaviors = getComponents<Behavior>(Component::ComponentType::BEHAVIOR);
	//for (auto& b : behaviors) {
	//	b->onColliderEnter(collisionInfo);
	//}
	m_collisionState = CollisionState::STAY;
	m_collisionInfo = collisionInfo;
}

void Entity::onCollisionEnd(const CollisionInfo & collisionInfo)
{
	//TODO speed up this operation
	//auto behaviors = getComponents<Behavior>(Component::ComponentType::BEHAVIOR);
	//for (auto& b : behaviors) {
	//	b->onColliderEnter(collisionInfo);
	//}
	m_collisionState = CollisionState::END;
	m_collisionInfo = collisionInfo;
}

void Entity::save(Json::Value& entityRoot) const
{
	TransformNode::save(entityRoot);

	//Scene* m_scene; scene is already set by constructor 
	entityRoot["name"] = m_name;

	//std::vector<Entity*> m_childs; TODO
	//Entity* m_parent; TODO

	//bool m_isSelected; selected information isn't serialized

	entityRoot["componentCount"] = m_componentsRefs.size();
	for (int i = 0; i < m_componentsRefs.size(); i++)
	{
		m_componentsRefs[i]->save(entityRoot["components"][i]);
	}

	//entityRoot["childCount"] = m_childs.size();
	//for (int i = 0; i < m_childs.size(); i++)
	//{
	//	m_childs[i]->save(entityRoot["childs"][i]);
	//}
}

void Entity::load(const Json::Value& entityRoot)
{
	TransformNode::load(entityRoot);
	applyTransform();

	//Scene* m_scene; scene is already set by constructor 
	m_name = entityRoot.get("name", "defaultEntity").asString();

	//std::vector<Entity*> m_childs; TODO
	//Entity* m_parent; TODO

	//bool m_isSelected; selected information isn't serialized

	int componentCount = entityRoot.get("componentCount", 0).asInt();
	for (int i = 0; i < componentCount; i++)
	{
		Component* newComponent;
		Component::ComponentType type = (Component::ComponentType)entityRoot["components"][i].get("type", 0).asInt();
		if (type == Component::ComponentType::BEHAVIOR) {
			std::string behaviourTypeIndexName = entityRoot["components"][i].get("typeIndexName", "NONE").asString();
			if (behaviourTypeIndexName == "NONE")
				continue;
			newComponent = BehaviorFactory::get().getInstance(behaviourTypeIndexName);
		}
		else {
			newComponent = ComponentFactory::get().getInstance(type);
		}
		newComponent->load(entityRoot["components"][i]);

		newComponent->addToEntity(*this);
	}

	applyTransform();

	//int childCount = entityRoot.get("childCount", 0).asInt();
	//for (int i = 0; i < childCount; i++)
	//{
	//	Entity* newEntity = new Entity(m_scene);
	//	newEntity->load(entityRoot["childs"][i]);
	//}
}

void Entity::addChildAtomic(Entity* child)
{
	m_childs.push_back(child);
}

void Entity::setParentAtomic(Entity* parent)
{
	m_parent = parent;

	if (m_parent == nullptr)
		setParentTransform();
	else
		setParentTransform(*m_parent);
}