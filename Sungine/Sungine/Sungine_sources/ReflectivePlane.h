#pragma once

#include "Mesh.h"
#include "Materials.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Component.h"
#include "ResourcePointer.h"
#include "IRenderableComponent.h"

struct ID;
struct ReflectionCamera;
struct BaseCamera;
class RenderTarget;

class ReflectivePlane : public Component, public IRenderableComponent, public IBatchableWith<MaterialReflection>
{
	COMPONENT_IMPLEMENTATION_HEADER(ReflectivePlane)

	CLASS((ReflectivePlane, Component),
	((PRIVATE)
		(bool		, m_castShadow		)
	)
	)


private:

	// TODO : ->parameter
	ResourcePtr<Mesh> m_mesh;
	ResourcePtr<Material> m_material;

	glm::mat4 m_modelMatrix;
	AABB m_aabb;

	std::unordered_map<ID, std::shared_ptr<ReflectionCamera>> m_reflexionCameras;
	ID m_activeCameraID;

public:

	ReflectivePlane();
	ReflectivePlane(ResourcePtr<Mesh> mesh, ResourcePtr<Material> material);
	ReflectivePlane(const ReflectivePlane& other);
	virtual ~ReflectivePlane();

	void setMesh(ResourcePtr<Mesh> _mesh);
	void setMaterial(ResourcePtr<Material> _material);

	const Material* getMaterial() const;
	const Mesh* getMesh() const;

	void setModelMatrix(const glm::mat4& modelMatrix);
	void setAABB(const AABB& aabb);

	void clearCameras();
	void addAndSetupCamera(const ID& id, const BaseCamera & camera);
	std::unordered_map<ID, std::shared_ptr<ReflectionCamera>>::iterator getCameraIteratorBegin();
	std::unordered_map<ID, std::shared_ptr<ReflectionCamera>>::iterator getCameraIteratorEnd();
	void setActiveCamera(ID cameraID);
	ReflectionCamera& getCamera(ID cameraID) const;
	ReflectionCamera& getActiveCamera() const;
	glm::vec4 getClipPlane() const;

	// Herite from Component
	virtual void drawInInspector(Scene& scene) override;
	virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat()) override;

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;

	virtual void onAfterComponentAddedToScene(Scene& scene) override;
	virtual void onBeforeComponentErasedFromScene(Scene& scene) override;
	virtual void onAfterComponentAddedToEntity(Entity& entity) override;

	// Herite from IRenderableComponent
	virtual const IDrawable& getDrawable(int drawableIndex) const override;
	virtual const Material& getDrawableMaterial(int drawableIndex) const override;
	virtual const int getDrawableCount() const override;
	virtual Component* getAsComponent() override;

	// Herited from IDrawable
	const AABB & getVisualBoundingBox() const override;
	void draw() const override;
	virtual const glm::mat4& getModelMatrix() const override;
	virtual bool castShadows() const override;

	// Herited from IBatchableWith<Material3DObject>
	void setExternalsOf(const MaterialReflection& material, const glm::mat4& projection, const glm::mat4& view, int* texId = nullptr) const override;

	virtual const ReflectivePlane* getAsReflectivePlaneIfPossible() const override;

};