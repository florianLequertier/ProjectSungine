

#include "Entity.h"
//forwards : 
#include "Component.h"
#include "Scene.h"
#include "SceneAccessor.h"
#include "ComponentFactory.h"
#include "BehaviorFactory.h"
#include "PhysicManager.h"
#include "Application.h"
#include "Collider.h" // CollisionInfo

Entity::Entity(Scene* scene) 
	: TransformNode()
	, m_scene(scene)
	, m_isSelected(false)
	, m_name("default_entity")
	, m_isVisible(false)
	//, m_parent(nullptr)
{
	//scene->getAccessor().addToScene(this);
}

Entity::Entity(const Entity& other) 
	: TransformNode(other)
	, m_isSelected(other.m_isSelected)
	, m_name(other.m_name)
	, m_scene(other.m_scene)
	, m_parent(other.m_parent)
	, m_isVisible(other.m_isVisible)
{
	//m_scene->getAccessor().addToScene(this);

	eraseAllComponents();
	for (int i = 0; i < other.m_componentsHandles.size(); i++)
	{
		ObjectPtr<Component> newComponent = other.m_componentsHandles[i].clonePointedObject();
		addComponent(newComponent);
	}

	eraseAllChilds();
	for (int i = 0; i < other.m_childs.size(); i++)
	{
		ObjectPtr<Entity> newEntity = other.m_childs[i].clonePointedObject();//m_scene->instantiate(*other.m_childs[i]);
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

	//m_scene->getAccessor().addToScene(this);

	// Duplicate components
	eraseAllComponents();
	for (int i = 0; i < other.m_componentsHandles.size(); i++)
	{
		//copy the component
		//auto newComponent = other.m_componentsRefs[i]->clone(this);
		//addComponentFromExisting(*other.m_componentsRefs[i], newComponent);

		ObjectPtr<Component> newComponent = other.m_componentsHandles[i].clonePointedObject();
		addComponent(newComponent);
	}

	// Duplicate childs
	eraseAllChilds();
	for (int i = 0; i < other.m_childs.size(); i++)
	{
		ObjectPtr<Entity> newEntity = other.m_childs[i].clonePointedObject();//m_scene->instantiate(*other.m_childs[i]);
		addChild(newEntity);
	}

	return *this;
}

Entity::~Entity()
{
	clearParent();

	eraseAllComponents();

	eraseAllChilds();
}

/////////////////////////////////////////////////////////////

void Entity::eraseAllComponents()
{
	const int size = m_componentsHandles.size();
	for (int i = 0; i < size; i++)
	{
		ObjectPtr<Component>& current = m_componentsHandles[i];

		current->onBeforeRemovedFromEntity(*this);
		removeComponentAtomic(i);
		current->onAfterRemovedFromEntity(*this);

		current->onBeforeRemovedFromScene(*m_scene);
		m_scene->destroy(current);
	}
}

void Entity::removeComponentAtomic(int index)
{
	std::iter_swap(m_componentsHandles.begin() + index, m_componentsHandles.end() - 1);
	m_componentsHandles.erase(m_componentsHandles.end() - 1);
}

void Entity::addComponent(const ObjectPtr<Component>& componentPtr)
{
	componentPtr->setEntityOwner(this); // set ownership
	componentPtr->onAfterAddedToScene(*m_scene);

	componentPtr->onBeforeAddedToEntity(*this);
	addComponentAtomic(componentPtr);
	componentPtr->onAfterAddedToEntity(*this);
}

void Entity::addComponentAtomic(const ObjectPtr<Component>& componentPtr)
{
	m_componentsHandles.push_back(componentPtr);
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

	for (auto& c : m_componentsHandles)
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
	//m_translation = parentRotation * m_localTranslation + parentTranslation;
	//m_scale = m_localScale * parentScale;
	//m_rotation = m_localRotation * parentRotation;

	//glm::mat4 updateMatrix = glm::translate(glm::mat4(1), parentTranslation);// *glm::mat4_cast(parentRotation);// *glm::scale(glm::mat4(1), parentScale);

	////combine transforms to get world model matrix of the model
	////and make the child launch applyTransform recursivly
	setParentTransform(parentTranslation, parentScale, parentRotation);
	updateModelMatrix();
}

void Entity::applyTransform(const glm::vec3 & parentTranslation, const glm::quat & parentRotation)
{
	//m_translation = parentRotation * m_localTranslation + parentTranslation;
	//m_rotation = m_localRotation * parentRotation;

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
	for (auto& c : m_componentsHandles)
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
		component->drawInInspector();
		//ImGui::EndChild();
	}

	if (nodeOpen)
		ImGui::TreePop();
}

void Entity::displayTreeNodeInspector(Scene& scene, Component* firstComponent, const std::vector<void*>& selection)
{
	ImGui::SetNextWindowContentWidth(80);
	bool nodeOpen = false;

	ImVec2 itemPos;
	ImVec2 itemSize;
	if (ImGui::MyTreeNode("", itemPos, itemSize))
		nodeOpen = true;
	ImGui::SameLine();

	ImGui::Text(firstComponent->getClassName().c_str());

	if (nodeOpen) 
	{
		firstComponent->drawInInspector(selection);
	}

	if (nodeOpen)
		ImGui::TreePop();
}

void Entity::drawInInspector()
{
	char tmpName[20];
	m_name.copy(tmpName, m_name.size(), 0);
	tmpName[m_name.size()] = '\0';
	if (ImGui::InputText("name", tmpName, 20))
	{
		m_name = tmpName;
	}

	TransformNode::drawInInspector();

	bool hasToRemoveComponent = false;
	int removeId = 0;
	for (int i = 0; i < m_componentsHandles.size(); i++)
	{
		ImGui::PushID(i);

		displayTreeNodeInspector(*m_scene, m_componentsHandles[i].getPtr(), i, hasToRemoveComponent, removeId);

		ImGui::PopID();
	}
	if(hasToRemoveComponent)
		m_componentsHandles[removeId]->eraseFromEntity(*this);

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

void extractAllComponents(const std::vector<Entity*>& entitiesIn, std::map<int, std::vector<ObjectPtr<Component>&>>& componentsOut)
{
	for (auto& entity : entitiesIn)
	{
		entity->getAllComponentsByTypes(componentsOut);
	}
}

void Entity::drawInInspector(const std::vector<void*>& objectInstances)
{
	char tmpName[20];
	m_name.copy(tmpName, m_name.size(), 0);
	tmpName[m_name.size()] = '\0';
	if (ImGui::InputText("name", tmpName, 20))
	{
		for (int i = 0; i < objectInstances.size(); i++)
		{
			Entity* entity = static_cast<Entity*>(objectInstances[i]);
			entity->m_name = tmpName;
		}
	}

	TransformNode::drawInInspector(objectInstances);

	bool hasToRemoveComponent = false;
	int removeId = 0;

	std::map<int, std::vector<ObjectPtr<Component>&>> outComponents;
	for (int i = 0; i < objectInstances.size(); i++)
	{
		Entity* entity = static_cast<Entity*>(objectInstances[i]);
		entity->getAllComponentsByTypes(outComponents);
	}

	for (auto& componentsByType : outComponents)
	{
		std::vector<void*> componentPtrs(componentsByType.second.size());
		int index = 0;
		for (auto component : componentsByType.second)
		{
			componentPtrs[index] = component.getPtr();
			index++;
		}

		ImGui::PushID(componentsByType.first);
		displayTreeNodeInspector(*m_scene, componentsByType.second[0].getPtr(), componentPtrs);
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
	const ObjectPtr<Collider>& colliderComponent = getComponent<Collider>();
	if (colliderComponent.isValid())
	{
		// if a component containing a mesh is present in the entity, cover it with the collider : 
		const ObjectPtr<Physic::Flag>& flagComponent = getComponent<Physic::Flag>();
		const ObjectPtr<MeshRenderer>& meshRendererComponent = getComponent<MeshRenderer>();
		if (flagComponent.isValid())
		{
			colliderComponent->setOrigin(flagComponent->getOrigin());
			colliderComponent->coverMesh(flagComponent->getMesh());
			colliderComponent->addOffsetScale(glm::vec3(0.f, 0.f, 0.2f));
		}
		else if (meshRendererComponent.isValid())
		{
			colliderComponent->setOrigin(meshRendererComponent->getOrigin());
			colliderComponent->coverMesh(*meshRendererComponent->getMesh());
		}
	}
}

void Entity::getAllComponentsByTypes(std::map<int, std::vector<ObjectPtr<Component>&>>& outComponents)
{
	for (int i = 0; i < m_componentsHandles.size(); i++)
	{
		outComponents[m_componentsHandles[i]->getClassId()].push_back(m_componentsHandles[i]);
	}
}

bool Entity::hasParent() const
{
	return m_parent.isValid();
}

bool Entity::hasChild() const
{
	return m_childs.size() > 0;
}

const ObjectPtr<Entity>& Entity::getChild(int idx)
{
	return m_childs[idx];
}

const ObjectPtr<Entity>& Entity::getParent()
{
	return m_parent;
}

void Entity::clearParent()
{
	if (m_parent.isValid())
		m_parent->removeChildAtomic(ObjectPtr<Entity>(this, m_scene));
	m_parent.reset();
}

void Entity::setParent(const ObjectPtr<Entity>& parent)
{
	if (m_parent.isValid())
		clearParent();

	setParentAtomic(parent);
	if(m_parent.isValid())
		m_parent->addChildAtomic( ObjectPtr<Entity>(this, m_scene) );
}

void Entity::addChild(const ObjectPtr<Entity>& child)
{
	child->clearParent();

	child->setParentAtomic( ObjectPtr<Entity>(this, m_scene) );
	addChildAtomic(child);
}

void Entity::detachChild(const ObjectPtr<Entity>& child)
{
	if (m_childs.size() > 0) //has childs ?
	{
		auto findIt = std::find(m_childs.begin(), m_childs.end(), child);
		if (findIt != m_childs.end())
		{
			(*findIt)->m_parent.reset();
			m_childs.erase(findIt);
		}
	}
}

void Entity::removeChildAtomic(const ObjectPtr<Entity>& child)
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
	for (auto& child : m_childs)
	{
		m_scene->destroy(child);
	}

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

void Entity::OnBeforeObjectSaved()
{
	//TODO
}

void Entity::OnAfterObjectLoaded()
{
	for (auto& component : m_componentsHandles)
	{
		component->onAfterAddedToEntity(*this);
	}
}

void Entity::addChildAtomic(const ObjectPtr<Entity>& child)
{
	m_childs.push_back(child);
}

void Entity::setParentAtomic(const ObjectPtr<Entity>& parent)
{
	m_parent = parent;

	if (m_parent == nullptr)
		setParentTransform();
	else
		setParentTransform(*m_parent.getPtr());
}