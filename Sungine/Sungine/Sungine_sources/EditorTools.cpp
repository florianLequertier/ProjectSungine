
#include "EditorTools.h"

//forwards : 
#include "Entity.h"
#include "Scene.h"
#include "Editor.h"
#include "Application.h"

///////////////////////////////// INSPECTOR

Inspector::Inspector(Editor* editorPtr)
	: m_editorPtr(editorPtr)
{

}

Inspector::~Inspector()
{

}

void Inspector::setScene(Scene * currentScene)
{
	m_currentScene = currentScene;
}

void Inspector::drawUI()
{
	int entityId = 0;
	
	std::vector<IDrawableInInspector*> selection;
	m_editorPtr->getCurrentDrawableSelection(selection);

	if (!selection.empty())
	{
		//can't add or remove components in multiple editing, only change components parameters
		if (selection.size() == 1)
		{
			if (m_multipleEditing)
				m_multipleEditing = false;
		}
		else if (ImGui::RadioButton("multiple editing", m_multipleEditing))
		{
			m_multipleEditing = !m_multipleEditing;

			//if (m_multipleEditing)
			//{
			//	refreshSelectedComponents(true);
			//}
		}

		if (selection.size() == 1)
		{
			if (selection[0] != nullptr)
				selection[0]->drawInInspector(*m_currentScene);
		}
		else
		{
			if (m_multipleEditing)
			{
				assert(selection.size() > 0);
				selection[0]->drawInInspector(*m_currentScene, selection);
			}
			else
			{
				for (auto& selected : selection)
				{
					ImGui::PushID(entityId);
					if (ImGui::CollapsingHeader(("entity " + patch::to_string(entityId)).c_str())) 
					{
						if(selected != nullptr)
							selected->drawInInspector(*m_currentScene);
					}
					ImGui::PopID();

					entityId++;
				}
			}
		}
	}
}
//
//
//void Inspector::drawUI(const std::vector<Entity*>& entities)
//{
//	if (entities.size() == 0)
//		return;
//
//	std::string tmpName = entities[0]->getName();
//	tmpName.copy(textValue, tmpName.size(), 0);
//	textValue[tmpName.size()] = '\0';
//
//	if (ImGui::InputText("name", textValue, 20))
//	{
//		for (auto& entity : entities)
//		{
//			entity->setName(textValue);
//		}
//	}
//
//	vector3Value = entities[0]->getEulerRotation();
//	if (ImGui::SliderFloat3("rotation", &vector3Value[0], 0, 2 * glm::pi<float>()))
//	{
//		for (auto& entity : entities)
//		{
//			entity->setEulerRotation(vector3Value);
//			//entity->setRotation(glm::quat(vector3Value));
//			//entity->applyTransform();
//		}
//	}
//
//	vector3Value = entities[0]->getScale();
//	if (ImGui::InputFloat3("scale", &vector3Value[0]))
//	{
//		for (auto& entity : entities)
//		{
//			entity->setScale(vector3Value);
//			//entity->applyTransform();
//		}
//	}
//}


//
//void Inspector::drawUI(const std::vector<PointLight*>& pointLights)
//{
//	if (pointLights.size() == 0)
//		return;
//
//	if (ImGui::CollapsingHeader("point light"))
//	{
//		floatValue = pointLights[0]->getIntensity();
//		if (ImGui::SliderFloat("light intensity", &floatValue, 0.f, 50.f))
//		{
//			for (auto& light : pointLights)
//			{
//				light->setIntensity( floatValue );
//			}
//		}
//		vector3Value = pointLights[0]->getColor();
//		if (ImGui::ColorEdit3("light color", &vector3Value[0]))
//		{
//			for (auto& light : pointLights)
//			{
//				light->setColor( vector3Value );
//			}
//		}
//	}
//}
//
//void Inspector::drawUI(const std::vector<DirectionalLight*>& directionalLights)
//{
//	if (directionalLights.size() == 0)
//		return;
//
//	if (ImGui::CollapsingHeader("directional light"))
//	{
//		floatValue = directionalLights[0]->getIntensity();
//		if (ImGui::SliderFloat("light intensity", &floatValue, 0.f, 10.f))
//		{
//			for (auto& light : directionalLights)
//			{
//				light->setIntensity( floatValue );
//			}
//		}
//		vector3Value = directionalLights[0]->getColor();
//		if (ImGui::ColorEdit3("light color", &vector3Value[0]))
//		{
//			for (auto& light : directionalLights)
//			{
//				light->setColor( vector3Value );
//			}
//		}
//	}
//}
//
//void Inspector::drawUI(const std::vector<SpotLight*>& spotLights)
//{
//	if (spotLights.size() == 0)
//		return;
//
//	if (ImGui::CollapsingHeader("spot light"))
//	{
//		floatValue = spotLights[0]->getIntensity();
//		if (ImGui::SliderFloat("light intensity", &floatValue, 0.f, 50.f))
//		{
//			for (auto& light : spotLights)
//			{
//				light->setIntensity( floatValue );
//			}
//		}
//		vector3Value = spotLights[0]->getColor();
//		if (ImGui::ColorEdit3("light color", &vector3Value[0]))
//		{
//			for (auto& light : spotLights)
//			{
//				light->setColor( vector3Value );
//			}
//		}
//		floatValue = spotLights[0]->angle;
//		if (ImGui::SliderFloat("light angles", &floatValue, 0.f, glm::pi<float>()))
//		{
//			for (auto& light : spotLights)
//			{
//				light->angle = floatValue;
//			}
//		}
//	}
//}
//
//void Inspector::drawUI(const std::vector<MeshRenderer*>& meshRenderers)
//{
//	if (meshRenderers.size() == 0)
//		return;
//
//	//std::string tmpName = meshRenderers[0]->getMaterialName(0);
//	//tmpName.copy(textValue, tmpName.size(), 0);
//	//textValue[tmpName.size()] = '\0';
//
//	//%NOCOMMIT%
//	//if (ImGui::InputText("materialName", textValue, 20))
//	//{
//	//	if (getMaterialFactory().contains<Material3DObject>(textValue))
//	//	{
//	//		for (auto& meshRenderer : meshRenderers)
//	//		{
//	//			meshRenderer->setMaterial( getMaterialFactory().get<Material3DObject>(textValue), 0);
//	//		}
//	//	}
//	//}
//	ResourcePtr<Material> materialPtrQuery;
//	//EditorGUI::ResourceField<Material>(materialPtrQuery, "materialName", textValue, 100);
//	EditorGUI::ResourceField<Material>("materialName", materialPtrQuery);
//
//	if (materialPtrQuery.isValid())
//	{
//		for (auto& meshRenderer : meshRenderers)
//		{
//			meshRenderer->setMaterial(materialPtrQuery, 0);
//		}
//	}
//
//	//meshRenderers[0]->getMaterial()->drawUI();
//
//	//tmpName = meshRenderers[0]->getMeshName();
//	//tmpName.copy(textValue, tmpName.size(), 0);
//	//textValue[tmpName.size()] = '\0';
//
//	//if (ImGui::InputText("meshName", textValue, 20))
//	//{
//	//	if (getMeshFactory().contains(textValue))
//	//	{
//	//		for (auto& meshRenderer : meshRenderers)
//	//		{
//	//			meshRenderer->setMesh( getMeshFactory().get(textValue) );
//	//		}
//	//	}
//	//}
//	ResourcePtr<Mesh> meshPtrQuery;
//	//EditorGUI::ResourceField<Mesh>(meshPtrQuery, "meshName", textValue, 100);
//	EditorGUI::ResourceField<Mesh>("meshName", meshPtrQuery);
//
//	if (meshPtrQuery.isValid())
//	{
//		for (auto& meshRenderer : meshRenderers)
//		{
//			meshRenderer->setMesh(meshPtrQuery);
//		}
//	}
//}
//
//void Inspector::drawUI(const std::vector<Collider*>& colliders)
//{
//	if (colliders.size() == 0)
//		return;
//
//	vector3Value = colliders[0]->offsetPosition;
//	if (ImGui::InputFloat3("offset position", &vector3Value[0]))
//	{
//		for (auto& collider : colliders)
//		{
//			collider->setOffsetPosition(vector3Value);
//		}
//	}
//	vector3Value = colliders[0]->offsetScale;
//	if (ImGui::InputFloat3("offset scale", &vector3Value[0]))
//	{
//		for (auto& collider : colliders)
//		{
//			collider->setOffsetScale(vector3Value);
//		}
//	}
//}

////////////////////////////////////////// EDITOR HIERARCHY

SceneHierarchy::SceneHierarchy(Editor * editorPtr)
	: m_editorPtr(editorPtr)
{

}

void SceneHierarchy::setScene(Scene* scene)
{
	m_currentScene = scene;
}

Scene* SceneHierarchy::getScene() const
{
	return m_currentScene;
}


void SceneHierarchy::displayTreeEntityNode(Entity* entity, int &entityId, bool &setParenting, Entity*& parentToAttachSelected)
{
	ImGui::PushID(entityId);
	bool nodeOpen = false;

	bool isSelected = entity->getIsSelected();
	if (isSelected)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 0, 1));
		//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0, 0, 1));
		//ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0.8, 0.8, 1));
	}

	ImVec2 itemPos;
	ImVec2 itemSize;
	if (ImGui::MyTreeNode(std::to_string(entityId).c_str(), itemPos, itemSize))
		nodeOpen = true;
	ImGui::SameLine();


	if (ImGui::Button(entity->getName().c_str(), ImVec2(itemSize.x /*m_bottomLeftPanelRect.z*/ - 36.f, itemSize.y /*16.f*/)))
	{
		if (isSelected)
		{
			glm::vec3 cameraFinalPosition = entity->getTranslation() - glm::vec3(m_editorPtr->getCamera().getCameraForward())*3.f;
			m_editorPtr->getCamera().setTranslation(cameraFinalPosition);
		}
		else
			m_editorPtr->changeCurrentSelected(entity);
	}

	ImGui::SameLine();

	if (ImGui::Button("<"))
	{
		setParenting = true;
		parentToAttachSelected = entity;
	}

	if (nodeOpen)
	{
		if (isSelected)
		{
			//ImGui::PopStyleColor();
			//ImGui::PopStyleColor();
			//ImGui::PopStyleColor();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.8, 0.8, 0.2));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0.2, 0.2, 0.2));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8, 0.8, 0.8, 0.2));
		}

		for (int c = 0; c < entity->getChildCount(); c++)
		{
			entityId++;
			displayTreeEntityNode(entity->getChild(c), entityId, setParenting, parentToAttachSelected);
		}
	}

	if (nodeOpen)
		ImGui::TreePop();

	if (isSelected && !nodeOpen)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.8, 0.8, 0.2));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0.2, 0.2, 0.2));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8, 0.8, 0.8, 0.2));
		//ImGui::PopStyleColor();
		//ImGui::PopStyleColor();
		//ImGui::PopStyleColor();
	}

	ImGui::PopID();
	entityId++;
}


void SceneHierarchy::drawUI()
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.8, 0.8, 0.2));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0.2, 0.2, 0.2));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8, 0.8, 0.8, 0.2));

	auto entities = m_currentScene->getEntities();

	int entityId = 0;
	bool setParenting = false;
	Entity* parentToAttachSelected = nullptr;
	for (auto& entity : entities)
	{
		if (!entity->hasParent())
		{
			displayTreeEntityNode(entity, entityId, setParenting, parentToAttachSelected);
		}
	}

	if (setParenting)
	{
		if (parentToAttachSelected->getIsSelected())
			parentToAttachSelected->setParent(nullptr);
		else
		{
			const std::vector<Entity*>& currentSelection = m_editorPtr->getCurrentSelection();
			for (int i = 0; i < currentSelection.size(); i++)
			{
				currentSelection[i]->setParent(parentToAttachSelected);
			}
		}
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

///////////////////////////////

DebugDrawRenderer::DebugDrawRenderer()
	: m_quadMesh(GL_TRIANGLES, (Mesh::USE_INDEX | Mesh::USE_VERTICES), 2)
	, m_needSeparator(false)
{
	m_material = getMaterialFactory().getDefault("blit");

	m_quadMesh.triangleIndex = { 0, 1, 2, 2, 1, 3 };
	m_quadMesh.vertices = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0 };
	m_quadMesh.initGl();

	m_texture = std::shared_ptr<Texture>(GlHelper::makeNewColorTexture(400, 400));
	m_texture->initGL();

	m_framebuffer.bind();
	m_framebuffer.setDrawBuffer(GL_COLOR_ATTACHMENT0);
	m_framebuffer.attachTexture(m_texture.get(), GL_COLOR_ATTACHMENT0);
	m_framebuffer.checkIntegrity();
	m_framebuffer.unbind();
}

void DebugDrawRenderer::drawTexture(GLuint textureId)
{
	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();
	glViewport(0, 0, 400, 400);
	glDisable(GL_DEPTH_TEST);

	m_framebuffer.bind();
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	m_material->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	m_material->setUniformBlitTexture(0);

	m_quadMesh.draw();

	m_framebuffer.unbind();

	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
}

void DebugDrawRenderer::drawUI()
{
	ImGui::PushID(this);
	ImGui::BeginChild("##DebugDrawRendererChild", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("choose output"))
		{
			auto& separatorIt = m_separatorIndex.begin();
			for (int i = 0; i < m_outputNames.size(); i++)
			{
				if (separatorIt != m_separatorIndex.end() && *separatorIt == i)
				{
					separatorIt++;
					ImGui::Separator();
				}

				ImGui::PushID(i);
				if (ImGui::Selectable(m_outputNames[i].c_str()))
				{
					setCurrentOutputName(m_outputNames[i]);
				}
				ImGui::PopID();
			}
			ImGui::EndMenu();
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(30.f, 0.f));
		ImGui::SameLine();
		ImGui::Text(("current output : " + m_currentOutputName).c_str());

		ImGui::EndMenuBar();
	}

	if (m_currentOutputName != "")
		ImGui::Image((void*)m_texture->glId, ImVec2(m_frameSize.x, m_frameSize.y), ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::PopID();

}

void DebugDrawRenderer::addSeparator()
{
	m_needSeparator = true;
}

void DebugDrawRenderer::setCurrentOutputName(const std::string& outputName)
{
	m_currentOutputName = outputName;
}

void DebugDrawRenderer::drawOutputIfNeeded(const std::string& outputName, GLuint textureId)
{
	if (std::find(m_outputNames.begin(), m_outputNames.end(), outputName) == m_outputNames.end())
	{
		if (m_needSeparator)
		{
			m_separatorIndex.push_back(m_outputNames.size());
			m_needSeparator = false;
		}
		m_outputNames.push_back(outputName);
	}

	if (m_currentOutputName == outputName)
	{
		drawTexture(textureId);
	}
}

void DebugDrawRenderer::setFrameSize(const glm::vec2 & size)
{
	m_frameSize = size;
}

/////////////////////////////////

WorldPropertiesTool::WorldPropertiesTool()
{
}

void WorldPropertiesTool::setScene(Scene * scene)
{
	m_currentScene = scene;
}

Scene * WorldPropertiesTool::getScene() const
{
	return m_currentScene;
}

void WorldPropertiesTool::drawUI()
{
	m_currentScene->drawUI();
}
