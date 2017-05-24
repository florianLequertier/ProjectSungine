

#include "Editor.h"
#include "EditorGUI.h"
#include "ResourceTree.h"

///////////////////////////////////////////////////////////
//// BEGIN : DroppedFileDragAndDropOperation

DroppedFileDragAndDropOperation::DroppedFileDragAndDropOperation(const FileHandler::CompletePath& resourcePath)
	: DragAndDropOperation(EditorDragAndDropType::DroppedFileDragAndDrop, (EditorDropContext::DropIntoFileOrFolder))
	, m_resourcePath(resourcePath)
{ 
}

void DroppedFileDragAndDropOperation::dragOperation()
{ }

void DroppedFileDragAndDropOperation::dropOperation(void* customData, int dropContext)
{
	if (!canDropInto(customData, dropContext))
	{
		cancelOperation();
		return;
	}

	if (dropContext == EditorDropContext::DropIntoFileOrFolder)
	{
		ResourceFolder* resourceFolder = static_cast<ResourceFolder*>(customData);

		if (!resourceFolder->hasFile(ResourceFileKey(m_resourcePath)))
		{
			ResourceTree::addExternalResourceTo(m_resourcePath, *resourceFolder);
			Editor::instance().removeDroppedFile(m_resourcePath);

			m_resourcePath = FileHandler::CompletePath();
		}
		else
			cancelOperation();
	}
	else
		cancelOperation();
}

void DroppedFileDragAndDropOperation::cancelOperation()
{
	m_resourcePath = FileHandler::CompletePath();
}

void DroppedFileDragAndDropOperation::updateOperation()
{
	ImVec2 mousePos = ImGui::GetMousePos();
	ImGui::SetNextWindowPos(mousePos);
	ImGui::Begin("DragAndDropWidget", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs);
	ImGui::Text(m_resourcePath.getFilename().c_str());
	ImGui::End();
}

bool DroppedFileDragAndDropOperation::canDropInto(void* customData, int dropContext)
{
	if (dropContext == EditorDropContext::DropIntoFileOrFolder)
	{
		ResourceFolder* resourceFolder = static_cast<ResourceFolder*>(customData);
		return !resourceFolder->hasFile(ResourceFileKey(m_resourcePath));
	}
	else
		return false;
}

//// END : DroppedFileDragAndDropOperation
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//// BEGIN : ResourceDragAndDropOperation

ResourceDragAndDropOperation::ResourceDragAndDropOperation(const ResourceFile* resource, ResourceFolder* resourceFolder)
	: DragAndDropOperation(EditorDragAndDropType::ResourceDragAndDrop, (EditorDropContext::DropIntoFileOrFolder | EditorDropContext::DropIntoResourceField))
	, m_folderResourceBelongsTo(resourceFolder)
	, m_resourceDragged(resource)
{ }

void ResourceDragAndDropOperation::dragOperation()
{ }

void ResourceDragAndDropOperation::dropOperation(void* customData, int dropContext)
{
	if (!canDropInto(customData, dropContext))
	{
		cancelOperation();
		return;
	}

	if (dropContext == EditorDropContext::DropIntoFileOrFolder)
	{
		ResourceFolder* resourceFolder = static_cast<ResourceFolder*>(customData);

		if (!resourceFolder->hasFile(m_resourceDragged->getKey()))
		{
			//const FileHandler::CompletePath newResourcePath(resourceFolder->getPath(), m_resourceDragged->getPath().getFilenameWithExtention());
			//On fait expres de copier ca c'est une nouvelle resource (copier collé)
			ResourceTree::moveResourceTo(*m_resourceDragged, *m_folderResourceBelongsTo, *resourceFolder);
			//resourceFolder->addFile(ResourceFile(newResourcePath));
			//m_folderResourceBelongsTo->removeFile(m_resourceDragged->getName());

			//FileHandler::copyPastFile(m_resourceDragged->getPath(), newResourcePath.getPath()); //NOT_SAFE

			m_folderResourceBelongsTo = nullptr;
			m_resourceDragged = nullptr;
		}
		else
			cancelOperation();
	}
	else if (dropContext == EditorDropContext::DropIntoResourceField)
	{
		FileHandler::CompletePath* resourcePath = static_cast<FileHandler::CompletePath*>(customData);
		*resourcePath = m_resourceDragged->getPath();

		m_folderResourceBelongsTo = nullptr;
		m_resourceDragged = nullptr;
	}
	else
		cancelOperation();
}

void ResourceDragAndDropOperation::cancelOperation()
{
	m_folderResourceBelongsTo = nullptr;
	m_resourceDragged = nullptr;
}

void ResourceDragAndDropOperation::updateOperation()
{
	ImVec2 mousePos = ImGui::GetMousePos();
	ImGui::SetNextWindowPos(mousePos);
	ImGui::Begin("DragAndDropWidget", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs);
	ImGui::Text(m_resourceDragged->getName().c_str());
	ImGui::End();
}

bool ResourceDragAndDropOperation::canDropInto(void* customData, int dropContext)
{
	if (dropContext == EditorDropContext::DropIntoFileOrFolder)
	{
		ResourceFolder* resourceFolder = static_cast<ResourceFolder*>(customData);
		return !resourceFolder->hasFile(m_resourceDragged->getKey());
	}
	else if(dropContext == EditorDropContext::DropIntoResourceField)
		return (m_resourceDragged->getType() && *static_cast<ResourceType*>(customData)) != 0;
}

//// END : ResourceDragAndDropOperation
///////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////
//// BEGIN : ResourceFolderDragAndDropOperation

ResourceFolderDragAndDropOperation::ResourceFolderDragAndDropOperation(const ResourceFolder* folder, ResourceFolder* parentFolder, ResourceTree* resourceTree)
	: DragAndDropOperation(EditorDragAndDropType::ResourceFolderDragAndDrop, EditorDropContext::DropIntoFileOrFolder)
	, m_resourceTree(resourceTree)
	, m_parentFolder(parentFolder)
	, m_folderDragged(folder)
{

}

void ResourceFolderDragAndDropOperation::dragOperation()
{

}

void ResourceFolderDragAndDropOperation::dropOperation(void* customData, int dropContext)
{
	if (!canDropInto(customData, dropContext))
	{
		cancelOperation();
		return;
	}

	ResourceFolder* resourceFolder = static_cast<ResourceFolder*>(customData);

	if (dropContext == EditorDropContext::DropIntoFileOrFolder
		&& !resourceFolder->hasSubFolder(m_folderDragged->getName()) && resourceFolder->getName() != m_folderDragged->getName())
	{
		if (m_parentFolder != nullptr)
			ResourceTree::moveSubFolderTo(m_folderDragged->getName(), *m_parentFolder , *resourceFolder);

		//if (m_parentFolder != nullptr)
		//	m_parentFolder->moveSubFolderToNewLocation(m_folderDragged->getName(), *resourceFolder);
		//else
		//	m_resourceTree->moveSubFolderToNewLocation(m_folderDragged->getName(), *resourceFolder);
		//%NOCOMMIT% : TODO : transvaser les fichiers et sous dossier dans le dossier cible

		m_parentFolder = nullptr;
		m_folderDragged = nullptr;
		m_resourceTree = nullptr;
	}
	else
		cancelOperation();
}

void ResourceFolderDragAndDropOperation::cancelOperation()
{
	m_parentFolder = nullptr;
	m_folderDragged = nullptr;
	m_resourceTree = nullptr;
}

void ResourceFolderDragAndDropOperation::updateOperation()
{
	ImVec2 mousePos = ImGui::GetMousePos();
	ImGui::SetNextWindowPos(mousePos);
	ImGui::Begin("DragAndDropWidget", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs);
	ImGui::Text(m_folderDragged->getName().c_str());
	ImGui::End();
}

bool ResourceFolderDragAndDropOperation::canDropInto(void* customData, int dropContext)
{
	if (dropContext == EditorDropContext::DropIntoFileOrFolder)
	{
		ResourceFolder* resourceFolder = static_cast<ResourceFolder*>(customData);
		return !resourceFolder->hasSubFolder(m_folderDragged->getName()) && (resourceFolder->getName() != m_folderDragged->getName());
	}
}


//// END : ResourceFolderDragAndDropOperation
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//// BEGIN : editor frame drag and drop


EditorFrameDragAndDropOperation::EditorFrameDragAndDropOperation(int currentDraggedWindowId, Editor* editorPtr)
	: DragAndDropOperation(EditorDragAndDropType::EditorFrameDragAndDrop, EditorDropContext::DropIntoEditorWindow)
	, m_currentDraggedWindowId(currentDraggedWindowId)
	, m_editorPtr(editorPtr)
{
	m_editorPtr->getWindowManager()->getWindow(m_currentDraggedWindowId)->setAlpha(0.2f);
	m_editorPtr->getWindowManager()->getWindow(m_currentDraggedWindowId)->setIsActive(false);
	m_editorPtr->getWindowManager()->showSeparators();
}

void EditorFrameDragAndDropOperation::dragOperation()
{
}

void EditorFrameDragAndDropOperation::dropOperation(void* customData, int dropContext)
{
	//std::shared_ptr<EditorNode>* ptrToNodeWeDropOn = (std::shared_ptr<EditorNode>*)customData;
	//*ptrToNodeWeDropOn = m_currentDraggedWindow->getNode();
	//m_currentDraggedWindow = nullptr;
	int* ptrToWindowsId = (int*)customData;
	*ptrToWindowsId = m_currentDraggedWindowId;

	m_editorPtr->getWindowManager()->getWindow(m_currentDraggedWindowId)->setAlpha(1.0f);
	m_editorPtr->getWindowManager()->getWindow(m_currentDraggedWindowId)->setIsActive(true);
	m_editorPtr->getWindowManager()->hideSeparators();
	m_currentDraggedWindowId = -1;
	m_editorPtr = nullptr;
}

void EditorFrameDragAndDropOperation::cancelOperation()
{
	m_editorPtr->getWindowManager()->getWindow(m_currentDraggedWindowId)->setAlpha(1.0f);
	m_editorPtr->getWindowManager()->getWindow(m_currentDraggedWindowId)->setIsActive(true);
	m_editorPtr->getWindowManager()->hideSeparators();
	m_currentDraggedWindowId = -1;
	m_editorPtr = nullptr;
}

void EditorFrameDragAndDropOperation::updateOperation()
{
	m_editorPtr->getWindowManager()->getWindow(m_currentDraggedWindowId)->move(ImGui::GetIO().MouseDelta);
}

bool EditorFrameDragAndDropOperation::canDropInto(void * customData, int dropContext)
{
	return false;
}

//// END : editor frame drag and drop
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//// BEGIN : Specialization for ValueFields

namespace EditorGUI {

//string field
bool ValueField(const std::string& label, std::string& outString, char* buf, int bufSize)
{
	bool textEdited = ImGui::InputText(label.c_str(), buf, bufSize, ImGuiInputTextFlags_EnterReturnsTrue);
	if (textEdited)
	{
		outString.clear();
		outString = buf;
	}
	return textEdited;
}


//drag field

//float
template<>
bool ValueField<float>(const std::string& label, float& value, const float& minValue, const float& maxValue, FieldDisplayType displayType)
{
	if(displayType == FieldDisplayType::SLIDER)
		return ImGui::SliderFloat(label.data(), &value, minValue, maxValue);
	else
		return ImGui::DragFloat(label.data(), &value, (maxValue - minValue)*0.1f, minValue, maxValue);
}

//input field

//float 
template<>
bool ValueField<float>(const std::string& label, float& value, FieldDisplayType displayType)
{
	return ImGui::InputFloat(label.data(), &value);
}

template<>
bool ValueField<glm::vec2>(const std::string& label, glm::vec2& value, FieldDisplayType displayType)
{
	return ImGui::InputFloat2(label.data(), &value[0]);
}

template<>
bool ValueField<glm::vec3>(const std::string& label, glm::vec3& value, FieldDisplayType displayType)
{
	if (displayType == FieldDisplayType::COLOR)
	{
		return ImGui::ColorEdit3(label.data(), &value[0]);
	}
	else
		return ImGui::InputFloat3(label.data(), &value[0]);
}

template<>
bool ValueField<glm::vec4>(const std::string& label, glm::vec4& value, FieldDisplayType displayType)
{
	if (displayType == FieldDisplayType::COLOR)
	{
		return ImGui::ColorEdit4(label.data(), &value[0]);
	}
	else
		return ImGui::InputFloat4(label.data(), &value[0]);
}


//int 
template<>
bool ValueField<int>(const std::string& label, int& value, FieldDisplayType displayType)
{
	return ImGui::InputInt(label.data(), &value);
}

template<>
bool ValueField<glm::ivec2>(const std::string& label, glm::ivec2& value, FieldDisplayType displayType)
{
	return ImGui::InputInt2(label.data(), &value[0]);
}

template<>
bool ValueField<glm::ivec3>(const std::string& label, glm::ivec3& value, FieldDisplayType displayType)
{
	return ImGui::InputInt3(label.data(), &value[0]);
}

template<>
bool ValueField<glm::ivec4>(const std::string& label, glm::ivec4& value, FieldDisplayType displayType)
{
	return ImGui::InputInt4(label.data(), &value[0]);
}

}


//// END : Specialization for ValueFields
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//// BEGIN : Editor style sheet

EditorStyleSheet::EditorStyleSheet()
	: m_mainColor(0.5f, 0.5f, 0.f)
	, m_mainBgColor(0.4f, 0.4f, 0.4f)
	, m_mainBgTitleColor(0.6f, 0.6f, 0.6f)
	, m_mainBgChildColor(0.3f, 0.3f, 0.3f)
	, m_mainBorderColor(0.5f, 0.5f, 0.0f)
	, m_mainTextColor(1.f, 1.f, 1.f)
	, m_windowRounding(0.5f)
	, m_frameRounding(0.5f)
	, m_childWindowRounding(0.5f)
{
}

const ImColor & EditorStyleSheet::getMainColor() const
{
	return m_mainColor;
}

void EditorStyleSheet::applyDefaultStyleSheet() const
{
	ImGuiStyle& style = ImGui::GetStyle();

	//Scrollbar
	style.ScrollbarRounding = 0.f;
	style.ScrollbarSize = 10.0f;
	//Frame
	style.FrameRounding = 0.5f;
	style.FramePadding = ImVec2(1, 1);
	//Grab
	style.GrabMinSize = 3.f;
	style.GrabRounding = 0.5f;
	//Window
	style.WindowPadding = ImVec2(0.0f, 0.0f);
	style.WindowRounding = 0.5f;
	//Spacing
	style.ItemSpacing = ImVec2(0.0f, 0.0f);


	style.Colors[ImGuiCol_Text] = m_mainTextColor;
	style.Colors[ImGuiCol_WindowBg] = m_mainBgColor;              // Background of normal windows
	style.Colors[ImGuiCol_ChildWindowBg] = m_mainBgChildColor;         // Background of child windows
	style.Colors[ImGuiCol_PopupBg] = m_mainBgChildColor;               // Background of popups, menus, tooltips windows
	style.Colors[ImGuiCol_Border] = m_mainBorderColor;
	style.Colors[ImGuiCol_TitleBg] = m_mainBgTitleColor;
	style.Colors[ImGuiCol_TitleBgActive] = m_mainBgTitleColor;
	style.Colors[ImGuiCol_TitleBgCollapsed] = m_mainBgTitleColor;
	style.Colors[ImGuiCol_MenuBarBg] = m_mainBgTitleColor;
	//style.Colors[ImGuiCol_TextDisabled] = mainDisabledColor;
	//style.Colors[ImGuiCol_BorderShadow] 
	//style.Colors[ImGuiCol_FrameBg] = mainBgColor;               // Background of checkbox, radio button, plot, slider, text input
	//style.Colors[ImGuiCol_FrameBgHovered] = mainHoveredColor;
	//style.Colors[ImGuiCol_FrameBgActive] = mainActiveColor;
	//style.Colors[ImGuiCol_ScrollbarBg],
	//style.Colors[ImGuiCol_ScrollbarGrab],
	//style.Colors[ImGuiCol_ScrollbarGrabHovered],
	//style.Colors[ImGuiCol_ScrollbarGrabActive],
	//style.Colors[ImGuiCol_ComboBg],
	//style.Colors[ImGuiCol_CheckMark],
	//style.Colors[ImGuiCol_SliderGrab],
	//style.Colors[ImGuiCol_SliderGrabActive],
	//style.Colors[ImGuiCol_Button],
	//style.Colors[ImGuiCol_ButtonHovered],
	//style.Colors[ImGuiCol_ButtonActive],
	//style.Colors[ImGuiCol_Header],
	//style.Colors[ImGuiCol_HeaderHovered],
	//style.Colors[ImGuiCol_HeaderActive],
	//style.Colors[ImGuiCol_Column],
	//style.Colors[ImGuiCol_ColumnHovered],
	//style.Colors[ImGuiCol_ColumnActive],
	//style.Colors[ImGuiCol_ResizeGrip],
	//style.Colors[ImGuiCol_ResizeGripHovered],
	//style.Colors[ImGuiCol_ResizeGripActive],
	//style.Colors[ImGuiCol_CloseButton],
	//style.Colors[ImGuiCol_CloseButtonHovered],
	//style.Colors[ImGuiCol_CloseButtonActive],
	//style.Colors[ImGuiCol_PlotLines],
	//style.Colors[ImGuiCol_PlotLinesHovered],
	//style.Colors[ImGuiCol_PlotHistogram],
	//style.Colors[ImGuiCol_PlotHistogramHovered],
	//style.Colors[ImGuiCol_TextSelectedBg],
	//style.Colors[ImGuiCol_ModalWindowDarkening],  // darken entire screen when a modal window is active
}

void EditorStyleSheet::pushFramePadding() const
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.f, 3.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3.f, 3.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 3.f));
}

void EditorStyleSheet::popFramePadding() const
{
	ImGui::PopStyleVar(3);
}

void EditorStyleSheet::pushMenuStyle() const
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.f, 3.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3.f, 3.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 3.f));
}

void EditorStyleSheet::popMenuStyle() const
{
	ImGui::PopStyleVar(3);
}

void EditorStyleSheet::pushFloatingWindowStyle() const
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_windowRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, m_childWindowRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_frameRounding);
}

void EditorStyleSheet::popFloatingWindowStyle() const
{
	ImGui::PopStyleVar(6);
}

void EditorStyleSheet::pushBackgroundWindowStyle() const
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, m_childWindowRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_frameRounding);
}

void EditorStyleSheet::popBackgroundWindowStyle() const
{
	ImGui::PopStyleVar(6);
}

//// END : Editor style sheet
///////////////////////////////////////////////////////////
