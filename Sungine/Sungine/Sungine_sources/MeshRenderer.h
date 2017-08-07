#pragma once

#include "Mesh.h"
#include "MaterialInstance.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Component.h"
#include "ResourcePointer.h"
#include "IRenderableComponent.h"

//forward
class Entity;

class MeshRenderer : public Component, public IRenderableComponent
{
	COMPONENT_IMPLEMENTATION_HEADER(MeshRenderer)

	CLASS((MeshRenderer, Component),
		((PRIVATE)
			(AssetHandle<Mesh>, m_mesh, "", updateLocalMeshDatas)
			(std::vector<AssetHandle<MaterialInstance>>, m_materials)
		)
	)

private:

	//AssetHandle<Mesh> m_mesh;
	//std::vector<AssetHandle<MaterialInstance>> m_materials;

	std::vector<std::shared_ptr<SubMesh>> m_subMeshes; // Warning ! Deep copy needed !

public:
	MeshRenderer();
	MeshRenderer(AssetHandle<Mesh> mesh, AssetHandle<MaterialInstance> material);
	MeshRenderer(const MeshRenderer& other);
	virtual ~MeshRenderer();

	//virtual void drawInInspector(Scene& scene) override;
	//virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void applyTransform(const glm::vec3& translation, const glm::vec3& scale = glm::vec3(1, 1, 1), const glm::quat& rotation = glm::quat()) override;
	virtual void applyTransformFromPhysicSimulation(const glm::vec3& translation, const glm::quat& rotation = glm::quat()) override;

	void setMesh(const AssetHandle<Mesh>& _mesh);
	void addMaterial(const AssetHandle<MaterialInstance>& _material);
	void removeMaterial(int idx);
	void setMaterial(const AssetHandle<MaterialInstance>& _material, int idx);

	const MaterialInstance* getMaterial(int idx) const;
	const Mesh* getMesh() const;

	std::string getMaterialName(int idx) const;
	std::string getMeshName() const;

	glm::vec3 getOrigin() const;

	void render(const glm::mat4& projection, const glm::mat4& view);

	void updateLocalMeshDatas();

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;

	virtual const IDrawable& getDrawable(int drawableIndex) const override;
	virtual const MaterialInstance& getDrawableMaterial(int drawableIndex) const override;
	virtual const int getDrawableCount() const override;
	virtual Component* getAsComponent() override;

	virtual void onAfterAddedToScene(Scene& scene) override;
	virtual void onBeforeRemovedFromScene(Scene& scene) override;
	virtual void onAfterAddedToEntity(Entity& entity) override;
};

REGISTER_CLASS(MeshRenderer)
