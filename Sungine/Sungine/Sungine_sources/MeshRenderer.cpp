

#include "MeshRenderer.h"
#include "Scene.h"
#include "SceneAccessor.h"
#include "Entity.h"
#include "Factories.h"

#include "EditorGUI.h"

COMPONENT_IMPLEMENTATION_CPP(MeshRenderer)

MeshRenderer::MeshRenderer() 
	: Component(MESH_RENDERER)
	, m_mesh(getMeshFactory().getDefault("default"))
{
	updateLocalMeshDatas();
	m_materials.push_back(getMaterialFactory().getDefault("defaultLit"));
}

MeshRenderer::MeshRenderer(ResourcePtr<Mesh> mesh, ResourcePtr<Material> material) 
	: Component(MESH_RENDERER)
	, m_mesh(mesh)
{
	updateLocalMeshDatas();
	m_materials.push_back(material);
}

MeshRenderer::MeshRenderer(const MeshRenderer & other)
	: Component(MESH_RENDERER)
{
	m_mesh = other.m_mesh;
	m_materials = other.m_materials;
	updateLocalMeshDatas();
}

MeshRenderer::~MeshRenderer()
{
	m_mesh.reset();
	m_materials.clear();
}

void MeshRenderer::drawInInspector(Scene& scene)
{
	ResourcePtr<Material> materialQuery;
	if (EditorGUI::ResourceField<Material>("materialName", materialQuery))
	{
		if (materialQuery.isValid())
		{
			m_materials.push_back(materialQuery);
		}
	}

	for (int i = 0; i < m_materials.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::Text(m_materials[i]->getName().c_str());
		if (m_materials.size() > 1)
		{
			ImGui::SameLine();
			if (ImGui::Button("remove"))
			{
				m_materials.erase(m_materials.begin() + i);
			}
		}
		ImGui::PopID();
	}

	ResourcePtr<Mesh> meshQuery;
	EditorGUI::ResourceField<Mesh>("meshName", meshQuery);

	if (meshQuery.isValid())
	{
		setMesh(meshQuery);
	}
}

void MeshRenderer::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	ResourcePtr<Material> materialQuery;
	EditorGUI::ResourceField<Material>("materialName", materialQuery);
	if (materialQuery.isValid())
	{
		for (auto component : components)
		{
			MeshRenderer* castedComponent = static_cast<MeshRenderer*>(component);
			castedComponent->m_materials.push_back(materialQuery);
		}
	}

	for (int i = 0; i < m_materials.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::Text(m_materials[i]->getName().c_str());
		if (m_materials.size() > 1) 
		{
			ImGui::SameLine();
			if (ImGui::Button("remove")) 
			{
				for (auto component : components)
				{
					MeshRenderer* castedComponent = static_cast<MeshRenderer*>(component);
					castedComponent->m_materials.erase(m_materials.begin() + i);
				}
			}
		}
		ImGui::PopID();
	}

	ResourcePtr<Mesh> meshQuery;
	EditorGUI::ResourceField<Mesh>("meshName", meshQuery);
	if (meshQuery.isValid())
	{
		for (auto component : components)
		{
			MeshRenderer* castedComponent = static_cast<MeshRenderer*>(component);
			castedComponent->setMesh(meshQuery);
		}
	}
}

void MeshRenderer::applyTransform(const glm::vec3 & translation, const glm::vec3 & scale, const glm::quat & rotation)
{
	if (m_mesh.isValid())
	{
		for (auto& subMesh : m_subMeshes)
		{
			subMesh->setModelMatrix(m_entity->getModelMatrix());
			subMesh->setAABB( AABB(m_mesh->getLocalAABB().center + translation, m_mesh->getLocalAABB().halfSizes * scale) );
		}
	}
}

void MeshRenderer::applyTransformFromPhysicSimulation(const glm::vec3 & translation, const glm::quat & rotation)
{
	if (m_mesh.isValid())
	{
		for (auto& subMesh : m_subMeshes)
			subMesh->setModelMatrix(m_entity->getModelMatrix());
	}
}

void MeshRenderer::setMesh(ResourcePtr<Mesh> _mesh)
{
	m_mesh = _mesh;
	updateLocalMeshDatas();
}

void MeshRenderer::addMaterial(ResourcePtr<Material> _material)
{
	m_materials.push_back(_material);
}

void MeshRenderer::removeMaterial(int idx)
{
	assert(idx >= 0 && idx < m_materials.size());

	m_materials.erase(m_materials.begin() + idx);
}

void MeshRenderer::setMaterial(ResourcePtr<Material> _material, int idx)
{
	assert(idx >= 0 && idx < m_materials.size());

	m_materials[idx] = _material;
}

//void MeshRenderer::setMaterial(Material3DObject * _material)
//{
//	if (_material != nullptr)
//		materialName = _material->name;
//
//	m_materials = _material;
//}

const Material* MeshRenderer::getMaterial(int idx) const
{
	return m_materials[idx].get();
}

const Mesh* MeshRenderer::getMesh() const
{
	return m_mesh.get();
}

std::string MeshRenderer::getMaterialName(int idx) const
{
	assert(idx >= 0 && idx < m_materials.size());

	return m_materials[idx]->getCompletePath().getFilename();
}

std::string MeshRenderer::getMeshName() const
{
	return m_mesh->getCompletePath().getFilename();
}

glm::vec3 MeshRenderer::getOrigin() const
{
	return m_mesh->origin;
}

void MeshRenderer::render(const glm::mat4 & projection, const glm::mat4 & view)
{
	glm::mat4 modelMatrix = entity()->getModelMatrix(); //get modelMatrix
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
	glm::mat4 mvp = projection * view * modelMatrix;

	int minMatMeshCount = std::min((int)m_materials.size(), m_mesh->subMeshCount);
	for (int i = 0; i < minMatMeshCount; i++)
	{
		Material3DObject* castedMaterial = static_cast<Material3DObject*>(m_materials[i].get()); //TODO : a enlever lors de l'upgrade du pipeline graphique.

		int texCount = 0;
		castedMaterial->use();
		castedMaterial->pushInternalsToGPU(texCount);

		//castedMaterial->setUniform_MVP(mvp);
		//castedMaterial->setUniform_normalMatrix(normalMatrix);
		castedMaterial->setUniformModelMatrix(modelMatrix);
		castedMaterial->setUniformViewMatrix(view);
		castedMaterial->setUniformProjectionMatrix(projection);
		if (m_mesh->getIsSkeletalMesh()) {
			for (int boneIdx = 0; boneIdx < m_mesh->getSkeleton()->getBoneCount(); boneIdx++)
				castedMaterial->setUniformBonesTransform(boneIdx, m_mesh->getSkeleton()->getBoneTransform(boneIdx));
		}
		castedMaterial->setUniformUseSkeleton(m_mesh->getIsSkeletalMesh());

		m_mesh->draw(i);	
	}
	//if there are more sub mesh than materials draw them with the last material
	for (int i = minMatMeshCount; i < m_mesh->subMeshCount; i++)
	{
		Material3DObject* castedMaterial = static_cast<Material3DObject*>(m_materials.back().get()); //TODO : a enlever lors de l'upgrade du pipeline graphique.

		int texCount = 0;
		castedMaterial->use();
		castedMaterial->pushInternalsToGPU(texCount);

		//castedMaterial->setUniform_MVP(mvp);
		//castedMaterial->setUniform_normalMatrix(normalMatrix);
		castedMaterial->setUniformModelMatrix(modelMatrix);
		castedMaterial->setUniformViewMatrix(view);
		castedMaterial->setUniformProjectionMatrix(projection);
		if (m_mesh->getIsSkeletalMesh())
			for (int boneIdx = 0; boneIdx < m_mesh->getSkeleton()->getBoneCount(); boneIdx++)
				castedMaterial->setUniformBonesTransform(boneIdx, m_mesh->getSkeleton()->getBoneTransform(boneIdx));
		castedMaterial->setUniformUseSkeleton(m_mesh->getIsSkeletalMesh());

		m_mesh->draw(i);
	}
}

void MeshRenderer::updateLocalMeshDatas()
{
	m_subMeshes.clear();
	if (!m_mesh.isValid())
		return;

	for (int i = 0; i < m_mesh->getSubMeshCount(); i++)
	{
		m_subMeshes.push_back(m_mesh->makeSharedSubMesh(i));
	}

	if (m_entity != nullptr)
	{
		auto collider = static_cast<Collider*>(m_entity->getComponent(Component::COLLIDER));
		if (collider != nullptr)
			collider->coverMesh(*m_mesh);
	}
}

void MeshRenderer::save(Json::Value & rootComponent) const
{
	Component::save(rootComponent);

	m_mesh.save(rootComponent["mesh"]);

	//rootComponent["meshKey"] = mesh->name;

	rootComponent["materialCount"] = m_materials.size();
	for (int i = 0; i < m_materials.size(); i++)
		m_materials[i].save(rootComponent["material"][i]);
}

void MeshRenderer::load(const Json::Value & rootComponent)
{
	Component::load(rootComponent);

	m_mesh.load(rootComponent["mesh"]);
	updateLocalMeshDatas();

	int materialCount = rootComponent.get("materialCount", 0).asInt();
	m_materials.clear();
	for (int i = 0; i < materialCount; i++)
	{
		ResourcePtr<Material> newMaterial(rootComponent["material"][i]);
		m_materials.push_back(newMaterial);
		//material.back()->initGL(); //TODO INITGL
	}
}

const IDrawable & MeshRenderer::getDrawable(int drawableIndex) const
{
	assert(drawableIndex >= 0 && drawableIndex < m_subMeshes.size());
	return *m_subMeshes[drawableIndex];
}

const Material & MeshRenderer::getDrawableMaterial(int drawableIndex) const
{
	return (drawableIndex >= m_materials.size()) ? *getMaterial(m_materials.size() - 1) : *getMaterial(drawableIndex);
}

const int MeshRenderer::getDrawableCount() const
{
	return getMesh()->getSubMeshCount();
}

Component * MeshRenderer::getAsComponent()
{
	return this;
}

void MeshRenderer::onAfterComponentAddedToScene(Scene & scene)
{
	//Add this components to renderables :
	IRenderableComponent* asRenderable = static_cast<IRenderableComponent*>(this);
	if (asRenderable->getDrawableCount() > 0)
		scene.addToRenderables(this);
}

void MeshRenderer::onBeforeComponentErasedFromScene(Scene & scene)
{
	//Remove this components from renderables :
	IRenderableComponent* asRenderable = static_cast<IRenderableComponent*>(this);
	if (asRenderable->getDrawableCount() > 0)
		scene.removeFromRenderables(this);
}

void MeshRenderer::onAfterComponentAddedToEntity(Entity & entity)
{
	if (m_entity != nullptr)
	{
		auto collider = static_cast<Collider*>(m_entity->getComponent(Component::COLLIDER));
		if (collider != nullptr)
			collider->coverMesh(*m_mesh);
	}
}