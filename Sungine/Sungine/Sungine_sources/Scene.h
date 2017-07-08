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

#include "ObjectPool.h"

#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/preprocessor/variadic.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/control/if.hpp>

#define SCENE_ELEMENT_COUNT 10000
//
//#define POOL_DEF(type, name) ObjectPool<type> BOOST_PP_CAT(pool, name);
//#define POOL_IMPL(type) \
//template<>\
//inline ObjectPool<type>* Scene3::getPool<type>()\
//{\
//	return &pool##type;\
//}
//
//#define POOL_DEF_(type, name) POOL_DEF(type, name)
//
//#define DEFINE_POOL(r, data, pool)\
//	POOL_DEF_(BOOST_PP_TUPLE_ELEM(0, pool), BOOST_PP_TUPLE_ELEM(1, pool))
//
//
//#define DEFINE_POOLS(pools)\
//	BOOST_PP_SEQ_FOR_EACH(DEFINE_POOL,, pools)
//
//#define POOL_ADD(type, name)\
//	pool##name.resize(SCENE_ELEMENT_COUNT);\
//	m_poolMapping[Object::getStaticClassId<type>()] = &pool##name;
//
//#define POOL_ADD_(type, name) POOL_ADD(type, name)
//
//#define ADD_POOL(r, data, pool)\
//	POOL_ADD_(BOOST_PP_TUPLE_ELEM(0, pool), BOOST_PP_TUPLE_ELEM(1, pool))
//
//#define ADD_POOLS(pools)\
//	public:\
//	void registerPools() {\
//		BOOST_PP_SEQ_FOR_EACH(ADD_POOL,, pools)\
//	}\
//private:
//
//#define SCENE(pools)\
//	DEFINE_POOLS(BOOST_PP_VARIADIC_SEQ_TO_SEQ(pools))\
//	ADD_POOLS(BOOST_PP_VARIADIC_SEQ_TO_SEQ(pools))

#define REGISTER_POOL_IN_SCENE(POOL_TYPE, CAPACITY)\
	bool ObjectPool<POOL_TYPE>::g_isRegister = SceneInitializer::addInitializer([](Scene* scene) { scene->addNewPool<POOL_TYPE>(CAPACITY); });

#define REGISTER_BEHAVIOUR_POOL_IN_SCENE(POOL_TYPE, CAPACITY)\
	bool ObjectPool<POOL_TYPE>::g_isRegister = SceneInitializer::addInitializer([](Scene* scene) { scene->addNewBehaviorPool<POOL_TYPE>(CAPACITY); });

class SceneInitializer : public ISingleton<SceneInitializer>
{
	std::vector<std::function<void(Scene* scene)>> m_initializers;

public:

	static bool addInitializer(const std::function<void(Scene* scene)>& initializer)
	{
		SceneInitializer::instance().m_initializers.push_back(initializer);
		return true;
	}

	void initScene(Scene* scene)
	{
		for (std::function<void(Scene* scene)>& initializer : m_initializers)
		{
			initializer(scene);
		}
	}

	static SceneInitializer& instance()
	{
		static SceneInitializer* _instance = new SceneInitializer();
		return *_instance;
	}
};

	REGISTER_POOL_IN_SCENE(Entity, 1000)
	REGISTER_POOL_IN_SCENE(PointLight, 1000)
	REGISTER_POOL_IN_SCENE(DirectionalLight, 1000)
	REGISTER_POOL_IN_SCENE(SpotLight, 1000)

	//Collidersders
	REGISTER_POOL_IN_SCENE(CapsuleCollider, 1000)
	REGISTER_POOL_IN_SCENE(BoxCollider, 1000)

	//Renderables
	REGISTER_POOL_IN_SCENE(MeshRenderer, 1000)
	REGISTER_POOL_IN_SCENE(ReflectivePlane, 1000)
	REGISTER_POOL_IN_SCENE(Physic::Flag, 1000)
	REGISTER_POOL_IN_SCENE(Physic::ParticleEmitter, 1000)
	REGISTER_POOL_IN_SCENE(Billboard, 1000)

	//Cameras
	REGISTER_POOL_IN_SCENE(Camera, 1000)

	//Physic
	REGISTER_POOL_IN_SCENE(Physic::WindZone, 1000)
	REGISTER_POOL_IN_SCENE(Rigidbody, 1000)
	REGISTER_POOL_IN_SCENE(CharacterController, 1000)

	//Animations
	REGISTER_POOL_IN_SCENE(Animator, 1000)

	//Behaviors
	//REGISTER_POOL_IN_SCENE(Behavior, 1000)

class Scene : public ObjectSpace
{
	friend SceneAccessor;

private:
	// Scene name :
	std::string m_name;

	// Special mapping for behaviors only
	std::vector<IObjectPool*> m_behaviorPools;

	// Ref to few usefull pools
	ObjectPool<Entity>* m_entities;
	ObjectPool<ReflectivePlane>* m_reflectivePlanes;
	ObjectPool<Camera>* m_cameras;
	ObjectPool<PointLight>* m_pointLights;
	ObjectPool<DirectionalLight>* m_directionalLights;
	ObjectPool<SpotLight>* m_spotLights;
	ObjectPool<Physic::ParticleEmitter>* m_particleEmitters;
	ObjectPool<CharacterController>* m_characterControllers;
	ObjectPool<Animator>* m_animators;
	ObjectPool<Physic::Flag>* m_flags;
	ObjectPool<Physic::WindZone>* m_windZones;

	// Renderables :
	Octree<IRenderableComponent, AABB> m_renderables;

	// Systems : 
	Renderer* m_renderer;
	Physic::PhysicManager* m_physicManager;
	PathManager m_pathManager;
	BehaviorManager m_behaviorManager;

	// Parameters : 
	bool m_areCollidersVisible;
	bool m_isDebugDeferredVisible;
	bool m_areLightsBoundingBoxVisible;
	bool m_areOctreesVisible;
	bool m_isDebugPhysicVisible;

	// Icones :
	Texture* m_pointLightIcone;
	Texture* m_directionalLightIcone;
	Texture* m_spotLightIcone;
	Texture* m_particleEmitterIcone;
	Mesh* m_iconeMesh;
	MaterialBillboard* m_iconeMaterial;

	// Asynchonous deletion
	std::vector<ObjectPtr<Object>> m_objectsToDelete;

	///////////////////

public:
	Scene(Renderer* renderer, const std::string& sceneName = "defaultScene");
	~Scene();
	void clear() override;

	//std::vector<Entity*>& getEntities();
	//SceneAccessor& getAccessor() const;
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

	void handleAsynchonousDeletion();

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

	template<typename T>
	void addNewBehaviorPool(int capacity = 1000)
	{
		ObjectPool<T>* newBehaviorPool = addNewPool<T>(capacity);
		m_behaviorPools.push_back(newBehaviorPool);
	}

	// Mask the default destroy of the ObjectSpace base class to handle asynchonous object detruction.
	template<typename T>
	void destroy(ObjectPtr<T> object)
	{
		m_objectsToDelete.push_back(object);
	}
};
