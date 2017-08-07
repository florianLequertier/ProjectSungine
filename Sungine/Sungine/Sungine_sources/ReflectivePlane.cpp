#include "ReflectivePlane.h"

#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "Factories.h"
#include "EditorGUI.h"
#include "Camera.h"
#include "IDGenerator.h"
#include "RenderTarget.h"

COMPONENT_IMPLEMENTATION_CPP(ReflectivePlane)

ReflectivePlane::ReflectivePlane()
	: Component(REFLECTIVE_PLANE)
	, m_mesh(getMeshFactory().getDefault("plane"))
	, m_material(getMaterialFactory().getDefault("defaultLit"))
	, m_castShadow(true)
{
}

ReflectivePlane::ReflectivePlane(AssetHandle<Mesh> mesh, AssetHandle<MaterialInstance> material)
	: Component(REFLECTIVE_PLANE)
	, m_mesh(mesh)
	, m_material(material)
	, m_castShadow(true)
{
}

ReflectivePlane::ReflectivePlane(const ReflectivePlane & other)
	: Component(REFLECTIVE_PLANE)
{
	m_mesh = other.m_mesh;
	m_material = other.m_material;
	m_castShadow = other.m_castShadow;
}

ReflectivePlane::~ReflectivePlane()
{
	m_mesh.reset();
	m_material.reset();
}
//
//void ReflectivePlane::drawInInspector(Scene& scene)
//{
//	AssetHandle<MaterialInstance> materialQuery;
//	if (EditorGUI::ResourceField<MaterialInstance>("materialName", materialQuery))
//	{
//		if (materialQuery.isValid())
//		{
//			setMaterial(materialQuery);
//		}
//	}
//
//	AssetHandle<Mesh> meshQuery;
//	if (EditorGUI::ResourceField<Mesh>("meshName", meshQuery))
//	{
//		if (meshQuery.isValid())
//		{
//			setMesh(meshQuery);
//		}
//	}
//
//	if (ImGui::RadioButton("castShadows", m_castShadow))
//	{
//		m_castShadow = !m_castShadow;
//	}
//}
//
//void ReflectivePlane::drawInInspector(Scene& scene, const std::vector<Component*>& components)
//{
//	AssetHandle<MaterialInstance> materialQuery;
//	EditorGUI::ResourceField<MaterialInstance>("materialName", materialQuery);
//	if (materialQuery.isValid())
//	{
//		for (auto component : components)
//		{
//			ReflectivePlane* castedComponent = static_cast<ReflectivePlane*>(component);
//			castedComponent->setMaterial(materialQuery);
//		}
//	}
//
//	AssetHandle<Mesh> meshQuery;
//	EditorGUI::ResourceField<Mesh>("meshName", meshQuery);
//	if (meshQuery.isValid())
//	{
//		for (auto component : components)
//		{
//			ReflectivePlane* castedComponent = static_cast<ReflectivePlane*>(component);
//			castedComponent->setMesh(meshQuery);
//		}
//	}
//
//	if (ImGui::RadioButton("castShadows", m_castShadow))
//	{
//		m_castShadow = !m_castShadow;
//
//		for (auto component : components)
//		{
//			ReflectivePlane* castedComponent = static_cast<ReflectivePlane*>(component);
//			castedComponent->m_castShadow = m_castShadow;
//		}
//	}
//}

void ReflectivePlane::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	setModelMatrix(m_entity->getModelMatrix());
	setAABB(AABB(m_mesh->getLocalAABB().center + translation, m_mesh->getLocalAABB().halfSizes * scale));
}

void ReflectivePlane::applyTransformFromPhysicSimulation(const glm::vec3 & translation, const glm::quat & rotation)
{
	const glm::vec3 curentScale = m_entity->getScale();
	setModelMatrix(m_entity->getModelMatrix());
	setAABB(AABB(m_mesh->getLocalAABB().center + translation, m_mesh->getLocalAABB().halfSizes * curentScale));
}

void ReflectivePlane::setMesh(const AssetHandle<Mesh>& _mesh)
{
	m_mesh = _mesh;
}

void ReflectivePlane::setMaterial(const AssetHandle<MaterialInstance>& _material)
{
	m_material = _material;
}

const MaterialInstance* ReflectivePlane::getMaterial() const
{
	return m_material.get();
}

const Mesh* ReflectivePlane::getMesh() const
{
	return m_mesh.get();
}

void ReflectivePlane::setModelMatrix(const glm::mat4& modelMatrix)
{
	m_modelMatrix = modelMatrix;
}

void ReflectivePlane::setAABB(const AABB& aabb)
{
	m_aabb = aabb;
}

void ReflectivePlane::clearCameras()
{
	m_reflexionCameras.clear();
}

void ReflectivePlane::addAndSetupCamera(const ID& id, const BaseCamera & camera)
{
	auto newReflectionCamera = std::make_shared<ReflectionCamera>();
	newReflectionCamera->setupFromCamera(m_entity->getTranslation(), m_entity->getRotation() * glm::vec3(0, 1, 0), camera);
	m_reflexionCameras[id] = newReflectionCamera;
}

std::unordered_map<ID, std::shared_ptr<ReflectionCamera>>::iterator ReflectivePlane::getCameraIteratorBegin()
{
	return m_reflexionCameras.begin();
}

std::unordered_map<ID, std::shared_ptr<ReflectionCamera>>::iterator ReflectivePlane::getCameraIteratorEnd()
{
	return m_reflexionCameras.end();
}

void ReflectivePlane::setActiveCamera(ID cameraID)
{
	assert(m_reflexionCameras.find(cameraID) != m_reflexionCameras.end());
	m_activeCameraID = cameraID;
}

ReflectionCamera& ReflectivePlane::getCamera(ID cameraID) const
{
	assert(m_reflexionCameras.find(cameraID) != m_reflexionCameras.end());
	return *m_reflexionCameras.at(cameraID);
}

ReflectionCamera & ReflectivePlane::getActiveCamera() const
{
	return getCamera(m_activeCameraID);
}

glm::vec4 ReflectivePlane::getClipPlane() const
{
	const glm::vec3 planeNormal = glm::normalize(m_entity->getRotation() * glm::vec3(0, 1, 0));
	const float d = -glm::dot(m_entity->getTranslation(), planeNormal);
	return glm::vec4(planeNormal, d);
}

void ReflectivePlane::save(Json::Value & rootComponent) const
{
	Component::save(rootComponent);

	m_mesh.save(rootComponent["mesh"]);
	m_material.save(rootComponent["material"]);
}

void ReflectivePlane::load(const Json::Value & rootComponent)
{
	Component::load(rootComponent);

	m_mesh.load(rootComponent["mesh"]);
	m_material.load(rootComponent["material"]);
}

const IDrawable & ReflectivePlane::getDrawable(int drawableIndex) const
{
	return *this;
}

const MaterialInstance & ReflectivePlane::getDrawableMaterial(int drawableIndex) const
{
	return *m_material.get();
}

const int ReflectivePlane::getDrawableCount() const
{
	return 1;
}

Component * ReflectivePlane::getAsComponent()
{
	return this;
}

void ReflectivePlane::onAfterComponentAddedToScene(Scene & scene)
{
	//Add this components to renderables :
	IRenderableComponent* asRenderable = static_cast<IRenderableComponent*>(this);
	if (asRenderable->getDrawableCount() > 0)
		scene.addToRenderables(this);
}

void ReflectivePlane::onBeforeComponentErasedFromScene(Scene & scene)
{
	//Remove this components from renderables :
	IRenderableComponent* asRenderable = static_cast<IRenderableComponent*>(this);
	if (asRenderable->getDrawableCount() > 0)
		scene.removeFromRenderables(this);
}

void ReflectivePlane::onAfterComponentAddedToEntity(Entity & entity)
{
	assert(m_entity != nullptr && m_entity == &entity);

	auto collider = static_cast<Collider*>(m_entity->getComponent(Component::COLLIDER));
	if (collider != nullptr)
		collider->coverMesh(*m_mesh);
}

const AABB & ReflectivePlane::getVisualBoundingBox() const
{
	return m_aabb;
}

void ReflectivePlane::draw() const
{
	m_mesh->draw();
}

const glm::mat4 & ReflectivePlane::getModelMatrix() const
{
	return m_modelMatrix;
}

bool ReflectivePlane::castShadows() const
{
	return m_castShadow;
}

void ReflectivePlane::setExternalsOf(const MaterialReflection& material, const glm::mat4& projection, const glm::mat4& view, int* texId) const
{
	assert(false && "deprecated");
	//assert(texId != nullptr);

	//// Transform matrices
	//material.glUniform_MVP(projection * view * getModelMatrix());
	//
	//// Reflective texture
	//glActiveTexture(GL_TEXTURE0 + *texId);
	//glBindTexture(GL_TEXTURE_2D, m_reflexionCameras.at(m_activeCameraID)->getFinalFrame());
	//material.glUniform_ReflectionTexture(*texId);
	//(*texId)++;
}

const ReflectivePlane * ReflectivePlane::getAsReflectivePlaneIfPossible() const
{
	return this;
}
