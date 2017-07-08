

#include "Scene.h"
//forwards :
#include "Entity.h"
#include "Lights.h"
#include "Collider.h"
#include "Flag.h"
#include "ParticleEmitter.h"
#include "Billboard.h"
#include "Rigidbody.h"
#include "Camera.h"
#include "Behavior.h"
#include "Animator.h"
#include "CharacterController.h"
#include "ReflectivePlane.h"

#include "OctreeDrawer.h"
#include "PhysicManager.h"
#include "SceneAccessor.h"
#include "EditorTools.h"
#include "IDGenerator.h"
#include "Object.h"
#include "Factories.h"

#include "ObjectPool.h"


Scene::Scene(Renderer* renderer, const std::string& sceneName) 
	: m_renderer(renderer)
	, m_name(sceneName)
	, m_areCollidersVisible(true)
	, m_isDebugDeferredVisible(true)
	, m_isDebugPhysicVisible(true)
	, m_renderables(glm::vec3(0, 0, 0), 500, 3) //TODO RENDERING : put a flexible size
{
	SceneInitializer::instance().initScene(this);

	//m_accessor = std::make_shared<SceneAccessor>(this);
	m_physicManager = new Physic::PhysicManager();
	//m_terrain.initPhysics(m_physicManager->getBulletDynamicSimulation());

	//////////////////////////////////////////////
	//// BEGIN : Component container mapping

	// Lights :
	//m_componentMapping[PointLight::staticClassId()] = &m_pointLights;
	//m_componentMapping[DirectionalLight::staticClassId()] = &m_directionalLights;
	//m_componentMapping[SpotLight::staticClassId()] = &m_spotLights;
	//// Collider :
	//m_componentMapping[BoxCollider::staticClassId()] = &m_colliders;
	//m_componentMapping[CapsuleCollider::staticClassId()] = &m_colliders;
	//m_componentMapping[Collider::staticClassId()] = &m_colliders;
	//// Others :
	//m_componentMapping[MeshRenderer::staticClassId()] = &m_meshRenderers;
	//m_componentMapping[ReflectivePlane::staticClassId()] = &m_reflectivePlanes;
	//m_componentMapping[Physic::Flag::staticClassId()] = &m_flags;
	//m_componentMapping[Physic::ParticleEmitter::staticClassId()] = &m_particleEmitters;
	////m_componentMapping[PathPoint::getClassId()] = m_pathManager; //TODO CORE
	//m_componentMapping[Billboard::staticClassId()] = &m_billboards;
	//m_componentMapping[Camera::staticClassId()] = &m_cameras;
	//m_componentMapping[Physic::WindZone::staticClassId()] = &m_windZones;
	//m_componentMapping[Rigidbody::staticClassId()] = &m_rigidbodies;
	//m_componentMapping[Animator::staticClassId()] = &m_animators;
	//m_componentMapping[CharacterController::staticClassId()] = &m_characterControllers;
	//m_componentMapping[Behavior::staticClassId()] = &m_behaviors;

	m_entities = static_cast<ObjectPool<Entity>*>(m_poolMapping[Object::getStaticClassId<Entity>()]);
	m_reflectivePlanes = static_cast<ObjectPool<ReflectivePlane>*>(m_poolMapping[Object::getStaticClassId<ReflectivePlane>()]);
	m_cameras = static_cast<ObjectPool<Camera>*>(m_poolMapping[Object::getStaticClassId<Camera>()]);
	m_pointLights = static_cast<ObjectPool<PointLight>*>(m_poolMapping[Object::getStaticClassId<PointLight>()]);
	m_directionalLights = static_cast<ObjectPool<DirectionalLight>*>(m_poolMapping[Object::getStaticClassId<DirectionalLight>()]);
	m_spotLights = static_cast<ObjectPool<SpotLight>*>(m_poolMapping[Object::getStaticClassId<SpotLight>()]);
	m_particleEmitters = static_cast<ObjectPool<Physic::ParticleEmitter>*>(m_poolMapping[Object::getStaticClassId<Physic::ParticleEmitter>()]);
	m_characterControllers = static_cast<ObjectPool<CharacterController>*>(m_poolMapping[Object::getStaticClassId<CharacterController>()]);
	m_animators = static_cast<ObjectPool<Animator>*>(m_poolMapping[Object::getStaticClassId<Animator>()]);
	m_flags = static_cast<ObjectPool<Physic::Flag>*>(m_poolMapping[Object::getStaticClassId<Physic::Flag>()]);
	m_windZones = static_cast<ObjectPool<Physic::WindZone>*>(m_poolMapping[Object::getStaticClassId<Physic::WindZone>()]);

	//// END : Component container mapping
	//////////////////////////////////////////////

	m_pointLightIcone = getTextureFactory().getDefault("pointLightIcone");
	m_directionalLightIcone = getTextureFactory().getDefault("directionalLightIcone");
	m_spotLightIcone = getTextureFactory().getDefault("spotLightIcone");
	m_particleEmitterIcone = getTextureFactory().getDefault("particleEmitterIcone");
	m_iconeMesh = getMeshFactory().getDefault("plane");

	m_iconeMaterial = static_cast<MaterialBillboard*>(getMaterialFactory().getDefault("billboard"));
}

Scene::~Scene()
{
	clear();
}

void Scene::clear()
{
	ObjectSpace::clear();

	//Components : 
	/*
	//Cameras : 
	for (int i = 0; i < m_cameras.size(); i++)
	{
		delete m_cameras[i];
		m_cameras.clear();
	}
	//Colliders : 
	for (int i = 0; i < m_colliders.size(); i++)
	{
		delete m_colliders[i];
		m_colliders.clear();
	}
	//Directionnal lights : 
	for (int i = 0; i < m_directionalLights.size(); i++)
	{
		delete m_directionalLights[i];
		m_directionalLights.clear();
	}
	//Flags : 
	for (int i = 0; i < m_flags.size(); i++)
	{
		delete m_flags[i];
		m_flags.clear();
	}
	//Mesh renderers : 
	for (int i = 0; i < m_meshRenderers.size(); i++)
	{
		delete m_meshRenderers[i];
		m_meshRenderers.clear();
	}
	//Particle emitters : 
	for (int i = 0; i < m_particleEmitters.size(); i++)
	{
		delete m_particleEmitters[i];
		m_particleEmitters.clear();
	}
	//Point lights : 
	for (int i = 0; i < m_pointLights.size(); i++)
	{
		delete m_pointLights[i];
		m_pointLights.clear();
	}
	//Point lights : 
	for (int i = 0; i < m_spotLights.size(); i++)
	{
		delete m_spotLights[i];
		m_spotLights.clear();
	}
	//Wind zones : 
	for (int i = 0; i < m_windZones.size(); i++)
	{
		delete m_windZones[i];
		m_windZones.clear();
	}*/

	//Entities : 
	//for (int i = 0; i < m_entities.size(); i++)
	//{
	//	delete m_entities[i];
	//}
	//m_entities.clear();

	//clear systems : 
	//m_terrain.clear();
	m_physicManager->clear();
	delete m_physicManager;
	//m_skybox.clear(); //TODO
}

//std::vector<Entity*>& Scene::getEntities()
//{
//	return m_entities;
//}
//
//SceneAccessor& Scene::getAccessor() const
//{
//	return *m_accessor;
//}

void Scene::addToRenderables(IRenderableComponent* renderable)
{
	m_renderables.add(renderable, renderable->getDrawable(0).getVisualBoundingBox());
}

void Scene::removeFromRenderables(IRenderableComponent* renderable)
{
	bool successfullyRemoved = m_renderables.remove(renderable, renderable->getDrawable(0).getVisualBoundingBox());
	assert(successfullyRemoved);
}

void Scene::clearReflectivePlanes()
{
	for (auto& reflectivePlane : *m_reflectivePlanes)
	{
		reflectivePlane.clearCameras();
	}
}

void Scene::setupReflectivePlanes()
{
	for (auto& camera : *m_cameras)
	{
		for (auto& reflectivePlane : *m_reflectivePlanes)
		{
			reflectivePlane.addAndSetupCamera(camera.getObjectID(), camera);
		}
	}
}

void Scene::setupReflectivePlanes(const ID& id, const BaseCamera& camera)
{
	for (auto& reflectivePlane : *m_reflectivePlanes)
	{
		reflectivePlane.addAndSetupCamera(id, camera);
	}
}

void Scene::computeCulling()
{
	for (auto& camera : *m_cameras)
	{
		if(camera.getIsActive())
			camera.computeCulling(m_renderables);
	}

	for (auto& reflectivePlane : *m_reflectivePlanes)
	{
		if (reflectivePlane.entity()->getVisibility())
		{
			for (auto& it = reflectivePlane.getCameraIteratorBegin(); it != reflectivePlane.getCameraIteratorEnd(); it++)
			{
				it->second->computeCulling(m_renderables);
			}
		}
	}
}

void Scene::computeCullingForSingleCamera(BaseCamera& camera)
{
	camera.computeCulling(m_renderables);
}

void Scene::render(RenderTarget& renderTarget)
{
	// Render reflexion on reflective planes : 
	for (auto& reflective : *m_reflectivePlanes)
	{
		for (auto& it = reflective.getCameraIteratorBegin(); it != reflective.getCameraIteratorEnd(); it++)
		{
			m_renderer->renderReflection(*it->second, renderTarget, reflective, m_pointLights, m_directionalLights, m_spotLights, nullptr);
		}
	}

	// Render scene through cameras
	for (auto& camera : *m_cameras)
	{
		m_renderer->render(camera, renderTarget, m_pointLights, m_directionalLights, m_spotLights, true);
	}
}

void Scene::renderForEditor(CameraEditor& camera, RenderTarget& renderTarget, DebugDrawRenderer& debugDrawer)
{
	// Render reflexion on reflective planes :
	for (auto& reflective : *m_reflectivePlanes)
	{
		for (auto& it = reflective.getCameraIteratorBegin(); it != reflective.getCameraIteratorEnd(); it++)
		{
			m_renderer->renderReflection(*it->second, renderTarget, reflective, m_pointLights, m_directionalLights, m_spotLights, &debugDrawer);
		}
	}

	// Render scene :
	m_renderer->render(camera, renderTarget, m_pointLights, m_directionalLights, m_spotLights, true, &debugDrawer);
	ColorAndDepthRTL* depthFrameBuffer = nullptr;
	m_renderer->transferDepthTo(*renderTarget.getLayer(0), renderTarget.getSize() /*camera.getFrameBuffer(), camera.getViewportSize()*/);// m_renderer->getIntermediateViewportSize());

	// Draw debug render :
	//camera.getFrameBuffer().bind();
	renderTarget.bindFramebuffer();

	renderIcones(camera);

	if (m_areCollidersVisible)
		m_renderer->debugDrawColliders(camera, m_entities);

	if (m_areLightsBoundingBoxVisible)
		m_renderer->debugDrawLights(camera, m_pointLights, m_spotLights);

	m_pathManager.render(camera);
	CHECK_GL_ERROR("error when rendering paths");

	if (m_areOctreesVisible)
	{
		OctreeDrawer::get().render(camera.getProjectionMatrix(), camera.getViewMatrix());
		OctreeDrawer::get().clear();
		CHECK_GL_ERROR("error when rendering octrees");
	}

	if (m_isDebugPhysicVisible)
		m_physicManager->debugDraw(camera.getProjectionMatrix(), camera.getViewMatrix());

	//camera.getFrameBuffer().unbind();
	renderTarget.unbindFramebuffer();
}

void Scene::renderIcones(CameraEditor& camera)
{
	const glm::mat4& view = camera.getViewMatrix();

	glm::vec3 CameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
	glm::vec3 CameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);

	m_iconeMaterial->use();

	m_iconeMaterial->setUniformCameraRight(CameraRight);
	m_iconeMaterial->setUniformCameraUp(CameraUp);
	m_iconeMaterial->setUniformColor(glm::vec4(1,1,1,1));
	m_iconeMaterial->setUniformMVP(camera.getProjectionMatrix() * view);
	m_iconeMaterial->setUniformScale(glm::vec2(0.2, 0.2));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pointLightIcone->glId);
	m_iconeMaterial->setUniformTexture(0);
	for (auto pointLight : *m_pointLights)
	{
		m_iconeMaterial->setUniformTranslation(pointLight.position);
		m_iconeMesh->draw();
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_directionalLightIcone->glId);
	m_iconeMaterial->setUniformTexture(0);
	for (auto directionalLight : *m_directionalLights)
	{
		m_iconeMaterial->setUniformTranslation(directionalLight.position);
		m_iconeMesh->draw();
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_spotLightIcone->glId);
	m_iconeMaterial->setUniformTexture(0);
	for (auto spotLight : *m_spotLights)
	{
		m_iconeMaterial->setUniformTranslation(spotLight.position);
		m_iconeMesh->draw();
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_particleEmitterIcone->glId);
	m_iconeMaterial->setUniformTexture(0);
	for (auto particleEmitter : *m_particleEmitters)
	{
		m_iconeMaterial->setUniformTranslation(particleEmitter.entity()->getTranslation());
		m_iconeMesh->draw();
	}
}

//
//void Scene::renderColliders(const BaseCamera & camera)
//{
//	if(m_areCollidersVisible)
//		m_renderer->debugDrawColliders(camera, m_entities);
//}
//
//void Scene::renderDebugLights(const BaseCamera & camera)
//{
//	if(m_areLightsBoundingBoxVisible)
//		m_renderer->debugDrawLights(camera, m_pointLights, m_spotLights);
//}
//
//void Scene::renderPaths(const BaseCamera& camera)
//{
//	m_pathManager.render(camera);
//}
//
//void Scene::renderDebugOctrees(const BaseCamera & camera)
//{
//	if (m_areOctreesVisible)
//	{
//		OctreeDrawer::get().render(camera.getProjectionMatrix(), camera.getViewMatrix());
//		OctreeDrawer::get().clear();
//	}
//}
//
//void Scene::renderDebugPhysic(const BaseCamera & camera)
//{
//	if (m_isDebugPhysicVisible)
//		m_physicManager->debugDraw(camera.getProjectionMatrix(), camera.getViewMatrix());
//}

void Scene::updatePhysic(float deltaTime, const BaseCamera& camera)
{
	m_physicManager->update(deltaTime, camera, m_flags, /*m_terrain,*/ m_windZones, m_particleEmitters);
}

void Scene::updatePhysic(float deltaTime, const BaseCamera& camera, bool updateInEditMode)
{
	m_physicManager->update(deltaTime, camera, m_flags, /*m_terrain,*/ m_windZones, m_particleEmitters, updateInEditMode);
}

void Scene::updateAnimations(float time)
{
	// TODO animatorManager ? 
	for (int i = 0; i < m_animators->size(); i++) 
	{
		(*m_animators)[i].updateAnimations(time);
	}
}

void Scene::updateControllers(float deltaTime)
{ 
	// TODO controllerManager ? 
	for (auto& controller : *m_characterControllers) 
	{
		controller.update(deltaTime);
	}
}

void Scene::updateBehaviours()
{
	
	for (auto behaviorPool : m_behaviorPools)
	{
		ObjectPoolProxy<Behavior> proxy(behaviorPool);
		for (int i = 0; i < proxy.size(); i++)
		{
			proxy[i].update(*this);
		}
		//for (auto& behavior : *static_cast<ObjectPool<Behavior>*>(behaviorPool))
		//{
		//	behavior.update(*this);
		//}
	}

	//m_behaviorManager.update(*this, m_behaviors);
	m_behaviorManager.updateCoroutines(m_entities);

	//TODO speed up this process :
	for (auto& entity : *m_entities)
	{
		if (entity.getCollisionState() == Entity::CollisionState::END)
			entity.resetCollision();
	}
}

void Scene::handleAsynchonousDeletion()
{
	for (auto& objectToDelete : m_objectsToDelete)
	{
		destroy(objectToDelete);
	}
}

void Scene::toggleColliderVisibility()
{
	m_areCollidersVisible = !m_areCollidersVisible;
}

void Scene::toggleDebugDeferredVisibility()
{
	m_isDebugDeferredVisible = !m_isDebugDeferredVisible;
}

void Scene::toggleLightsBoundingBoxVisibility()
{
	m_areLightsBoundingBoxVisible = !m_areLightsBoundingBoxVisible;
}

void Scene::toggleOctreesVisibility()
{
	m_areOctreesVisible = !m_areOctreesVisible;
}

void Scene::toggleDebugPhysicVisibility()
{
	m_isDebugPhysicVisible = !m_isDebugPhysicVisible;
}

bool Scene::getAreCollidersVisible() const
{
	return m_areCollidersVisible;
}

bool Scene::getIsDebugDeferredVisible() const
{
	return m_isDebugDeferredVisible;
}

bool Scene::getAreLightsBoundingBoxVisible() const
{
	return m_areLightsBoundingBoxVisible;
}

bool Scene::getAreOctreesVisible() const
{
	return m_areOctreesVisible;
}

bool Scene::getIsDebugPhysicVisible() const
{
	return m_isDebugPhysicVisible;
}

void Scene::setAreCollidersVisible(bool value)
{
	m_areCollidersVisible = value;
}

void Scene::setIsDebugDeferredVisible(bool value)
{
	m_isDebugDeferredVisible = value;
}

void Scene::setAreLightsBoundingBoxVisible(bool value)
{
	m_areLightsBoundingBoxVisible = value;
}

void Scene::setAreOctreesVisible(bool value)
{
	m_areOctreesVisible = value;
}

void Scene::setIsDebugPhysicVisible(bool value)
{
	m_isDebugPhysicVisible = value;
}

//Terrain& Scene::getTerrain()
//{
//	return m_terrain;
//}
//
//Skybox& Scene::getSkybox()
//{
//	return m_skybox;
//}

PathManager & Scene::getPathManager()
{
	return m_pathManager;
}

Renderer& Scene::getRenderer()
{
	return *m_renderer;
}

Physic::PhysicManager& Scene::getPhysicManager()
{
	return *m_physicManager;
}

std::string Scene::getName() const
{
	return m_name;
}

void Scene::setName(const std::string & name)
{
	m_name = name;
}

void Scene::resolveEntityChildSaving(Json::Value & rootComponent, Entity* currentEntity)
{
	rootComponent["isRootEntity"] = false;
	rootComponent["childCount"] = currentEntity->getChildCount();
	currentEntity->save(rootComponent);
	for (int i = 0; i < currentEntity->getChildCount(); i++) {
		resolveEntityChildSaving(rootComponent["childs"][i], currentEntity->getChild(i));
	}
}

void Scene::save(const FileHandler::CompletePath& path)
{
	Json::Value root;

	root["entityCount"] = m_entities->size();
	for (int i = 0; i < m_entities->size(); i++)
	{
		if (!(*m_entities)[i].hasParent()) 
		{
			root["entities"][i]["isRootEntity"] = true;
			root["entities"][i]["childCount"] = (*m_entities)[i].getChildCount();
			(*m_entities)[i].save(root["entities"][i]);
			for (int j = 0; j < (*m_entities)[i].getChildCount(); j++) 
			{
				resolveEntityChildSaving(root["entities"][i]["childs"][j], (*m_entities)[i].getChild(j));
			}
		}
	}

	//TODO
	//m_terrain.save(root["terrain"]);
	//m_skybox.save(root["skybox"]);
	
	//DEBUG
	//std::cout << root;

	std::ofstream stream;
	stream.open(path.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't save scene at path : " << path.toString() << std::endl;
		return;
	}
	//save scene
	stream << root;
}

void Scene::resolveEntityChildPreLoading(Json::Value & rootComponent, Entity* currentEntity)
{
	int childcount = rootComponent.get("childCount", 0).asInt();
	for (int j = 0; j < childcount; j++) {
		Entity* newEntity = new Entity(this);
		currentEntity->addChild(newEntity);
		resolveEntityChildPreLoading(rootComponent["childs"][j], newEntity);
	}
}

void Scene::resolveEntityChildLoading(Json::Value & rootComponent, Entity* currentEntity)
{
	int childcount = rootComponent.get("childCount", 0).asInt();
	for (int j = 0; j < childcount; j++) {
		currentEntity->getChild(j)->load(rootComponent["childs"][j]);
		resolveEntityChildLoading(rootComponent["childs"][j], currentEntity->getChild(j));
	}
}

void Scene::load(const FileHandler::CompletePath& path)
{
	Json::Value root;

	std::ifstream stream;
	stream.open(path.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load scene at path : " << path.toString() << std::endl;
		return;
	}
	stream >> root;
	
	int entityCount = root.get("entityCount", 0).asInt();
	assert(m_entities->size() == 0);
	for (int i = 0; i < entityCount; i++) {
		if (root["entities"][i].get("isRootEntity", false).asBool() == true) {
			auto newEntity = new Entity(this);
			resolveEntityChildPreLoading(root["entities"][i], newEntity);
		}
	}
	for (int i = 0; i < entityCount; i++) {
		if (root["entities"][i].get("isRootEntity", false).asBool() == true) {
			m_entities[i]->load(root["entities"][i]);
			resolveEntityChildLoading(root["entities"][i], m_entities[i]);
		}
	}

	//TODO
	//m_terrain.load(root["terrain"]);
	//m_terrain.initPhysics(m_physicManager.getBulletDynamicSimulation()); //TODO automatize this process in loading ? 
	//m_skybox.load(root["skybox"]);

}

// TODO : access pool for ObjectPtr
ObjectPtr<BaseCamera> Scene::getMainCamera() const
{
	return m_cameras->size() > 0 ? (*m_cameras)[0] : nullptr;
}

//void Scene::onViewportResized(const glm::vec2 & newSize)
//{
//	for (auto& camera : m_cameras)
//	{
//		camera->onViewportResized(newSize);
//	}
//}

void Scene::drawUI()
{
	ImGui::Text("Visibility octree debug :");
	m_renderables.drawDebug();
}
