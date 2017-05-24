#pragma once

#include <vector>
#include <unordered_map>

class Entity;
struct DirectionalLight;
struct PointLight;
struct SpotLight;
class Collider;
class Billboard;
class Rigidbody;
class Camera;
class Behavior;
class Animator;
class CharacterController;
class ReflectivePlane;
namespace Physic {
	class Flag;
	class ParticleEmitter;
	class WindZone;
}
struct ID;
struct BaseCamera;

#include "PathManager.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Terrain.h"
#include "Skybox.h"
#include "BehaviorManager.h"
namespace Physic {
	class PhysicManager;
}
class DebugDrawRenderer;

#include "FileHandler.h"
#include "BasicColliders.h"
#include "IRenderableComponent.h"
class SceneAccessor;

#include "jsoncpp/json/json.h"
#include <iostream>
#include <fstream>


class Scene
{
	friend SceneAccessor;

private:
	//scene name :
	std::string m_name;

	//entities : 
	std::vector<Entity*> m_entities;

	//components : 
	//lights : 
	std::vector<PointLight*> m_pointLights;
	std::vector<DirectionalLight*> m_directionalLights;
	std::vector<SpotLight*> m_spotLights;

	//colliders : 
	std::vector<Collider*> m_colliders;

	//meshRenderers : 
	std::vector<MeshRenderer*> m_meshRenderers;

	//reflectivePlane : 
	std::vector<ReflectivePlane*> m_reflectivePlanes;

	//flag : 
	std::vector<Physic::Flag*> m_flags;

	//particles : 
	std::vector<Physic::ParticleEmitter*> m_particleEmitters;

	//billboards : 
	std::vector<Billboard*> m_billboards;

	//cameras : 
	std::vector<Camera*> m_cameras;
	
	//windZones : 
	std::vector<Physic::WindZone*> m_windZones;

	//rigidbodies : 
	std::vector<Rigidbody*> m_rigidbodies;

	//animators : 
	std::vector<Animator*> m_animators;

	//character controllers : 
	std::vector<CharacterController*> m_characterControllers;

	//behaviors : 
	std::vector<Behavior*> m_behaviors;

	//special components : 
	//terrain : 
	//Terrain m_terrain;

	//skybox : 
	//Skybox m_skybox;

	//Renderables :
	Octree<IRenderableComponent, AABB> m_renderables;

	//systems : 
	Renderer* m_renderer;
	Physic::PhysicManager* m_physicManager;
	PathManager m_pathManager;
	BehaviorManager m_behaviorManager;
	//TODO
	//CloudSystem m_cloudSystem;

	//parameters : 
	bool m_areCollidersVisible;
	bool m_isDebugDeferredVisible;
	bool m_areLightsBoundingBoxVisible;
	bool m_areOctreesVisible;
	bool m_isDebugPhysicVisible;

	// Mapping for components (component class id <-> void* representing the vector of components)
	std::unordered_map<int, void*> m_componentMapping;

	std::shared_ptr<SceneAccessor> m_accessor;

	// Icones :
	Texture* m_pointLightIcone;
	Texture* m_directionalLightIcone;
	Texture* m_spotLightIcone;
	Texture* m_particleEmitterIcone;
	Mesh* m_iconeMesh;
	MaterialBillboard* m_iconeMaterial;

public:
	Scene(Renderer* renderer, const std::string& sceneName = "defaultScene");
	~Scene();

	void clear();

	std::vector<Entity*>& getEntities();
	SceneAccessor& getAccessor() const;
	void addToRenderables(IRenderableComponent* renderable);
	void removeFromRenderables(IRenderableComponent* renderable);

	void clearReflectivePlanes();
	void setupReflectivePlanes();
	void setupReflectivePlanes(const ID& id, const BaseCamera& camera);

	void computeCulling();
	void computeCullingForSingleCamera(BaseCamera& camera);

	void render(RenderTarget& renderTarget);
	void renderForEditor(CameraEditor& camera, RenderTarget& renderTarget, DebugDrawRenderer& debugDrawer);
	void renderIcones(CameraEditor& camera);
	//void renderColliders(const BaseCamera& camera);
	//void renderDebugLights(const BaseCamera& camera);
	//void renderPaths(const BaseCamera& camera);
	//void renderDebugOctrees(const BaseCamera& camera);
	//void renderDebugPhysic(const BaseCamera& camera);

	void updatePhysic(float deltaTime, const BaseCamera& camera);
	void updatePhysic(float deltaTime, const BaseCamera& camera, bool updateInEditMode);

	void updateAnimations(float time);
	void updateControllers(float deltaTime);
	void updateBehaviours();

	void toggleColliderVisibility();
	void toggleDebugDeferredVisibility();
	void toggleLightsBoundingBoxVisibility();
	void toggleOctreesVisibility();
	void toggleDebugPhysicVisibility();
	bool getAreCollidersVisible() const;
	bool getIsDebugDeferredVisible() const;
	bool getAreLightsBoundingBoxVisible() const;
	bool getAreOctreesVisible() const;
	bool getIsDebugPhysicVisible() const;
	void setAreCollidersVisible(bool value);
	void setIsDebugDeferredVisible(bool value);
	void setAreLightsBoundingBoxVisible(bool value);
	void setAreOctreesVisible(bool value);
	void setIsDebugPhysicVisible(bool value);

	//Terrain& getTerrain();
	//Skybox& getSkybox();
	PathManager& getPathManager();
	Renderer& getRenderer();
	Physic::PhysicManager& getPhysicManager();

	std::string getName() const;
	void setName(const std::string& name);

	//helper to load entities, for making links between entities and their childs
	void resolveEntityChildSaving(Json::Value & rootComponent, Entity* currentEntity);
	void resolveEntityChildPreLoading(Json::Value & rootComponent, Entity* currentEntity);
	void resolveEntityChildLoading(Json::Value & rootComponent, Entity* currentEntity);
	void save(const FileHandler::CompletePath& path);
	void load(const FileHandler::CompletePath& path);

	BaseCamera* getMainCamera() const;

	//void onViewportResized(const glm::vec2& newSize);

	void drawUI();
};

