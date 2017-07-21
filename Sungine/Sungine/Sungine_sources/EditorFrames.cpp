#include "Editor.h"
#include "EditorFrames.h"
#include "Texture.h"
#include "Project.h"
#include "Scene.h"
#include "EditorWindows.h"
#include "EditorTools.h"
#include "Camera.h"
#include "PostProcess.h"
#include "EditorWindowManager.h"
#include "MaterialVisualScripting.h"
#include "AnimationStateMachine.h"


SaveSceneAsEditorFrame::SaveSceneAsEditorFrame(EditorWindowManager * windowManager)
	: EditorFrame("SaveSceneAsWindow")
	, m_windowManagerRef(windowManager)
{
	for (int i = 0; i < 60; i++)
		m_savePath[i] = '\0';
}

void SaveSceneAsEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	bool validPathEntered = true;
	ImGui::InputText("set scene name", m_savePath, 60);
	if (m_savePath[0] == '\0' || m_savePath == "")
		validPathEntered = false;

	if (validPathEntered)
	{
		FileHandler::CompletePath scenePath(project.getScenesFolderPath().toString() + m_savePath + ".json");

		if (!FileHandler::fileExists(scenePath))
		{
			if (ImGui::Button("save"))
			{
				project.saveAsActiveScene(scenePath);
				parentWindow->closeModal();
			}
		}
		else if (validPathEntered)
		{
			ImGui::Text("A scene with the same name already exists.");
		}
	}
	else
	{
		ImGui::Text("Invalid name.");
	}
	
	if (ImGui::Button("Cancel"))
	{
		parentWindow->closeModal();
	}
}


LoadSceneEditorFrame::LoadSceneEditorFrame(EditorWindowManager * windowManager, Editor* editorPtr)
	: EditorFrame("LoadSceneWindow")
	, m_windowManagerRef(windowManager)
	, m_needToSaveScene(true)
	, m_editorPtr(editorPtr)
{
}

void LoadSceneEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	if (m_needToSaveScene)
	{
		ImGui::Text("Would you like to save the current scene ?");
		if (ImGui::Button("Yes"))
		{
			if (!project.activeSceneExists())
				m_windowManagerRef->addModal(std::make_shared<SaveSceneAsEditorFrame>(m_windowManagerRef));
			else
				project.saveActiveScene();
			m_needToSaveScene = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
		{
			m_needToSaveScene = false;
		}
	}
	else
	{
		std::vector<std::string> outSceneNames;
		FileHandler::getAllFileNames(project.getScenesFolderPath(), outSceneNames);
		for (int i = 0; i < outSceneNames.size(); i++)
		{
			std::string filename;
			std::string extention;
			FileHandler::splitFileNameExtention(outSceneNames[i], filename, extention);

			if (ImGui::Button(filename.c_str()))
			{
				m_editorPtr->deselectAll();
				project.loadSceneAsynchrone(filename);
				ImGui::CloseCurrentPopup();
			}
		}
		if (ImGui::Button("Cancel"))
		{
			parentWindow->closeModal();
		}
	}
}

NewSceneEditorFrame::NewSceneEditorFrame(EditorWindowManager * windowManager)
	: EditorFrame("NewSceneWindow")
	, m_windowManagerRef(windowManager)
	, m_needToSaveScene(true)
{
}

void NewSceneEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	if (m_needToSaveScene)
	{
		ImGui::Text("Would you like to save the current scene ?");
		if (ImGui::Button("Yes"))
		{
			if (!project.activeSceneExists())
				m_windowManagerRef->addModal(std::make_shared<SaveSceneAsEditorFrame>(m_windowManagerRef));
			else
				project.saveActiveScene();
			m_needToSaveScene = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
		{
			m_needToSaveScene = false;
		}
	}
	else
	{
		project.createAndSwitchToNewSceneAsynchrone();
		parentWindow->closeModal();
	}
}

///////////////////////////////

//DroppedFileEditorWindow* DroppedFileEditorWindow::modalRef = nullptr;

DroppedFileEditorFrame::DroppedFileEditorFrame(Editor* editorRef)
	: EditorFrame("DroppedFileWindow")
	, m_treeView("DroppedFileWindow", editorRef->getResourceTree(), editorRef)
	, m_editorRef(editorRef)
{
	assert(editorRef != nullptr);
}
//
//void DroppedFileEditorWindow::openPopUp(Editor& editor)
//{
//	auto modaleSharedRef = std::make_shared<DroppedFileEditorWindow>(&editor);
//	modalRef = modaleSharedRef.get();
//	modalRef->treeView.setModel(editor.getResourceTree());
//	editor.addModal(modaleSharedRef);
//}
//
//void DroppedFileEditorWindow::closePopUp(Editor& editor)
//{
//	if (!modalRef)
//		return;
//
//	editor.removeModal(modalRef);
//	modalRef = nullptr;
//}

void DroppedFileEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
{
	ImGui::Columns(2);

	//draw file list
	for (int i = 0; i < m_editorRef->getDroppedFilesCount(); i++)
	{
		ImGui::Button(m_editorRef->getDroppedFilesPath(i).c_str());
		//files drag and drop
		if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
		{
			DragAndDropManager::beginDragAndDrop(std::make_shared<DroppedFileDragAndDropOperation>(m_editorRef->getDroppedFilesPath(i)));
		}
	}

	ImGui::NextColumn();

	//draw resource tree 
	ImGui::SetNextWindowContentSize( ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50) );
	ImGui::BeginChild("resourceTreeView");
	m_treeView.drawContent(project);
	ImGui::EndChild();

	ImGui::Columns(1);

	ImGui::Separator();

	if (ImGui::Button("Done"))
	{
		m_editorRef->clearDroppedFiles();
		parentWindow->closeModal();
	}
}

///////////////////////////////

ViewportEditorFrame::ViewportEditorFrame(const std::string& name, std::shared_ptr<Viewport> model, Editor* editorRef)
	: EditorFrame(name)
	, m_viewport(model)
	, m_editorRef(editorRef)
{
}

void ViewportEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	GLuint finalFrame = m_viewport.lock()->getRenderTarget().getFinalFrame(0); //m_editorRef->getViewportRenderTarget().getFinalFrame();// project.getActiveScene()->getRenderer().getFinalFrame();
	ImGui::Image((void*)(finalFrame), ImVec2(m_size.x, m_size.y)/*ImVec2(finalFrame->w, finalFrame->h)*/, ImVec2(0, 1), ImVec2(1,0) );
	if (ImGui::IsItemHovered())
		m_viewport.lock()->setIsHovered(true);
	else
		m_viewport.lock()->setIsHovered(false);
}

void ViewportEditorFrame::onFrameMoved()
{
	m_viewport.lock()->setPosition(m_position);
}

void ViewportEditorFrame::onFrameResized()
{
	m_viewport.lock()->setSize(m_size);
}

///////////////////////////////

//TerrainToolEditorFrame::TerrainToolEditorFrame(const std::string& name)
//	: EditorFrame(name)
//{}
//
//void TerrainToolEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
//{
//	Scene* scene = project.getActiveScene();
//	scene->getTerrain().drawUI();
//}
//
////////////////////////////////
//
//SkyboxToolEditorFrame::SkyboxToolEditorFrame(const std::string& name)
//	: EditorFrame(name)
//{}
//
//void SkyboxToolEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
//{
//	Scene* scene = project.getActiveScene();
//	scene->getSkybox().drawUI();
//}

//////////////////////////////

//SceneManagerEditorFrame::SceneManagerEditorFrame(const std::string& name)
//	: EditorFrame(name)
//{}
//
//void SceneManagerEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
//{
//	project.drawUI();
//}

//////////////////////////////

FactoriesDebugEditorFrame::FactoriesDebugEditorFrame(const std::string& name)
	: EditorFrame(name)
{
	hideAllDebugViews();
	m_textureFactoryVisible = true;
}

void FactoriesDebugEditorFrame::hideAllDebugViews()
{
	m_textureFactoryVisible = false;
	m_cubeTextureFactoryVisible = false;
	m_meshFactoryVisible = false;
	m_programFactoryVisible = false;
	m_materialFactoryVisible = false;
	m_skeletalAnimationFactoryVisible = false;
}

void FactoriesDebugEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
{
	if (ImGui::RadioButton("texture factory", m_textureFactoryVisible))
	{
		hideAllDebugViews();
		m_textureFactoryVisible = true;
	}
	if (ImGui::RadioButton("cube texture factory", m_cubeTextureFactoryVisible))
	{
		hideAllDebugViews();
		m_cubeTextureFactoryVisible = true;
	}
	if (ImGui::RadioButton("mesh factory", m_meshFactoryVisible))
	{
		hideAllDebugViews();
		m_meshFactoryVisible = true;
	}
	if (ImGui::RadioButton("program factory", m_programFactoryVisible))
	{
		hideAllDebugViews();
		m_programFactoryVisible = true;
	}
	if (ImGui::RadioButton("material factory", m_materialFactoryVisible))
	{
		hideAllDebugViews();
		m_materialFactoryVisible = true;
	}
	if (ImGui::RadioButton("skeletal animation factory", m_skeletalAnimationFactoryVisible))
	{
		hideAllDebugViews();
		m_skeletalAnimationFactoryVisible = true;
	}

	//////////////////////

	//TODO : implement drawUI in factories
	/*
	if (m_textureFactoryVisible)
	{
	ImGui::BeginChild("Texture factory");
	getTextureFactory().drawUI();
	ImGui::End();
	}

	if (m_cubeTextureFactoryVisible)
	{
	ImGui::BeginChild("Cube Texture factory");
	getCubeTextureFactory().drawUI();
	ImGui::End();
	}

	if (m_meshFactoryVisible)
	{
	ImGui::BeginChild("Mesh factory");
	getMeshFactory().drawUI();
	ImGui::End();
	}

	if (m_programFactoryVisible)
	{
	ImGui::BeginChild("Program factory");
	getProgramFactory().drawUI();
	ImGui::End();
	}

	if (m_materialFactoryVisible)
	{
	ImGui::BeginChild("MaterialInstance factory");
	getMaterialFactory().drawUI();
	ImGui::End();
	}

	if (m_skeletalAnimationFactoryVisible)
	{
	ImGui::BeginChild("SkeletalAnimation factory");
	getSkeletalAnimationFactory().drawUI();
	ImGui::End();
	}*/
}

//////////////////////////////

InspectorEditorFrame::InspectorEditorFrame(const std::string & name, std::shared_ptr<Inspector> model)
	: EditorFrame(name)
{
	m_inspector = model;
}

void InspectorEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	if (!m_inspector.expired())
	{
		m_inspector.lock()->drawUI();
	}
}

//////////////////////////////

DebugRenderEditorFrame::DebugRenderEditorFrame(const std::string& name, std::shared_ptr<DebugDrawRenderer> model)
	: EditorFrame(name)
	, m_debugDrawRenderer(model)
{

}

void DebugRenderEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
{
	if (!m_debugDrawRenderer.expired())
	{
		m_debugDrawRenderer.lock()->setFrameSize(m_size);
		m_debugDrawRenderer.lock()->drawUI();
	}
}

//////////////////////////////

SceneHierarchyEditorFrame::SceneHierarchyEditorFrame(const std::string& name, std::shared_ptr<SceneHierarchy> model)
	: EditorFrame(name)
{
	m_sceneHierarchy = model;
}

void SceneHierarchyEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
{
	if (!m_sceneHierarchy.expired())
	{
		m_sceneHierarchy.lock()->drawUI();
	}
}

//////////////////////////////

EditorCameraEditorFrame::EditorCameraEditorFrame(const std::string & name, CameraEditor * camera)
	: EditorFrame(name)
	, m_editorCamera(camera)
{
}

void EditorCameraEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	m_editorCamera->drawUI();
}

//////////////////////////////

WorldPropertiesEditorFrame::WorldPropertiesEditorFrame(const std::string & name, std::shared_ptr<WorldPropertiesTool> model)
	: EditorFrame(name)
	, m_worldPropertiesTool(model)
{
}

void WorldPropertiesEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	m_worldPropertiesTool.lock()->drawUI();
}

//////////////////////////////

MVSEditorFrame::MVSEditorFrame(MVS::NodeManager * content)
	: EditorFrame("MaterialVisualScripting")
	, m_content(content)
{
}

void MVSEditorFrame::drawContent(Project & project, EditorModal * parentWindow)
{
	m_content->drawUI(project.getActiveScene()->getRenderer());
}

//////////////////////////////

ASMEditorFrame::ASMEditorFrame(Sungine::Animation::AnimationStateMachine* content)
	: EditorFrame("AnimationStateMachine")
	, m_content(content)
{

}

void ASMEditorFrame::drawContent(Project& project, EditorModal* parentWindow)
{
	m_content->drawUI();
}