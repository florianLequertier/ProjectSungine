#pragma once

#include "Mesh.h"
#include "Materials.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Component.h"
#include "ResourcePointer.h"
#include "IRenderableComponent.h"

//forward
class Entity;

class MeshRenderer : public Component, public IRenderableComponent
{
	REFLEXION_HEADER(MeshRenderer)
	COMPONENT_IMPLEMENTATION_HEADER(MeshRenderer)

private:
	//%NOCOMMIT%
	//Mesh* mesh;
	//std::vector<Material3DObject*> material;
	ResourcePtr<Mesh> m_mesh;
	std::vector<std::shared_ptr<SubMesh>> m_subMeshes; // Warning ! Deep copy needed !
	std::vector<ResourcePtr<Material>> m_materials;

public:
	MeshRenderer();
	MeshRenderer(ResourcePtr<Mesh> mesh, ResourcePtr<Material> material);
	MeshRenderer(const MeshRenderer& other);
	virtual ~MeshRenderer();

	virtual void drawInInspector(Scene& scene) override;
	virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat()) override;

	void setMesh(ResourcePtr<Mesh> _mesh);
	void addMaterial(ResourcePtr<Material> _material);
	void removeMaterial(int idx);
	void setMaterial(ResourcePtr<Material> _material, int idx);

	const Material* getMaterial(int idx) const;
	const Mesh* getMesh() const;

	std::string getMaterialName(int idx) const;
	std::string getMeshName() const;

	glm::vec3 getOrigin() const;

	void render(const glm::mat4& projection, const glm::mat4& view);

	void updateLocalMeshDatas();

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;

	virtual const IDrawable& getDrawable(int drawableIndex) const override;
	virtual const Material& getDrawableMaterial(int drawableIndex) const override;
	virtual const int getDrawableCount() const override;
	virtual Component* getAsComponent() override;

	virtual void onAfterComponentAddedToScene(Scene& scene) override;
	virtual void onBeforeComponentErasedFromScene(Scene& scene) override;
	virtual void onAfterComponentAddedToEntity(Entity& entity) override;
};

REFLEXION_CPP(MeshRenderer)
REFLEXION_InheritFrom(MeshRenderer, Component)
