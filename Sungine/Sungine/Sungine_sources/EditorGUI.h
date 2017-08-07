#pragma once

#include <string>

#include "imgui/imgui.h"

#include "imgui_extension.h"
#include "ISingleton.h"
#include "GUI.h"
#include "Utils.h"
#include "ResourcePointer.h"
#include "FileHandler.h"

//forwards : 
class ResourceFolder;
class ResourceFile;
class ResourceTree;

class Editor;


///////////////////////////////////////////
//// BEGIN : Drag and drop

enum EditorDragAndDropType
{
	AssetDragAndDrop = 1<<0,
	AssetFolderDragAndDrop = 1<<1,
	DroppedFileDragAndDrop = 1<<2,
	EditorFrameDragAndDrop = 1<<3
};
ENUM_MASK_IMPLEMENTATION(EditorDragAndDropType);

enum EditorDropContext
{
	DropIntoFileOrFolder = 1 << 0,
	DropIntoResourceField = 1 << 1,
	DropIntoEditorWindow = 1 << 2
};
ENUM_MASK_IMPLEMENTATION(EditorDropContext);


class DroppedFileDragAndDropOperation : public DragAndDropOperation
{
private:
	FileHandler::CompletePath m_resourcePath;

public:
	DroppedFileDragAndDropOperation(const FileHandler::CompletePath& resourcePath);
	virtual void dragOperation() override;
	virtual void dropOperation(void* customData, int dropContext) override;
	virtual void cancelOperation() override;
	virtual void updateOperation() override;
	virtual bool canDropInto(void* customData, int dropContext) override;
};

class AssetDragAndDropOperation : public DragAndDropOperation
{
private:
	AssetFolder* m_folderAssetBelongsTo;
	const AssetFile* m_assetDragged;
public:
	AssetDragAndDropOperation(const AssetFile* asset, AssetFolder* assetFolder);
	virtual void dragOperation() override;
	virtual void dropOperation(void* customData, int dropContext) override;
	virtual void cancelOperation() override;
	virtual void updateOperation() override;
	virtual bool canDropInto(void* customData, int dropContext) override;
};

class AssetFolderDragAndDropOperation : public DragAndDropOperation
{
private:
	AssetTree* m_assetTree;
	AssetFolder* m_parentFolder;
	const AssetFolder* m_folderDragged;
public:
	AssetFolderDragAndDropOperation(const AssetFolder* folder, AssetFolder* parentFolder, AssetTree* assetTree);
	virtual void dragOperation() override;
	virtual void dropOperation(void* customData, int dropContext) override;
	virtual void cancelOperation() override;
	virtual void updateOperation() override;
	virtual bool canDropInto(void* customData, int dropContext) override;
};

class EditorFrameDragAndDropOperation : public DragAndDropOperation
{
private:
	//Don't use it directly ! otherwise, use a shared_from_this. We only need this to get the node of the window.
	int m_currentDraggedWindowId;
	Editor* m_editorPtr;

public:
	EditorFrameDragAndDropOperation(int currentDraggedWindowId, Editor* editor);
	virtual void dragOperation() override;
	virtual void dropOperation(void* customData, int dropContext) override;
	virtual void cancelOperation() override;
	virtual void updateOperation() override;
	virtual bool canDropInto(void* customData, int dropContext) override;
};


//// END : Drag and drop
///////////////////////////////////////////

///////////////////////////////////////////
//// BEGIN : Editor style sheet

class EditorStyleSheet
{
private:
	ImColor m_mainBgColor;
	ImColor m_mainBgTitleColor;
	ImColor m_mainBgChildColor;
	ImColor m_mainBorderColor;
	ImColor m_mainTextColor;
	float m_windowRounding;
	float m_frameRounding;
	float m_childWindowRounding;
	ImColor m_mainColor;

public:
	EditorStyleSheet();
	const ImColor& getMainColor() const;
	void applyDefaultStyleSheet() const;
	void pushFramePadding() const;
	void popFramePadding() const;
	void pushMenuStyle() const;
	void popMenuStyle() const;
	void pushFloatingWindowStyle() const;
	void popFloatingWindowStyle() const;
	void pushBackgroundWindowStyle() const;
	void popBackgroundWindowStyle() const;
};



//// END : Editor style sheet
///////////////////////////////////////////



///////////////////////////////////////////
//// BEGIN : Editor fields

namespace EditorGUI {

	enum FieldDisplayType {
		DEFAULT = 0,
		INPUT,
		SLIDER,
		COLOR
	};

	static std::vector<std::string> LiteralFieldDisplayType = {
		"default",
		"input",
		"slider",
		"color",
	};


	template<typename T>
	bool ResourceField(const std::string& label, ResourcePtr<T>& resourcePtr);
	//{
	//	const int bufSize = 100;
	//	std::string currentResourceName(resourcePtr.isValid() ? resourcePtr->getCompletePath().getFilename() : "INVALID");
	//	currentResourceName.reserve(bufSize);

	//	ResourceType resourceType = getResourceType<T>();
	//	bool canDropIntoField = DragAndDropManager::canDropInto(&resourceType, EditorDropContext::DropIntoResourceField);
	//	bool isTextEdited = false;
	//	bool needClearPtr = false;

	//	int colStyleCount = 0;
	//	if (canDropIntoField)
	//	{
	//		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 255, 0, 255));
	//		colStyleCount++;
	//	}

	//	bool enterPressed = ImGui::InputText(label.c_str(), &currentResourceName[0], bufSize, ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_ReadOnly);
	//	isTextEdited = (enterPressed || ImGui::IsKeyPressed(GLFW_KEY_TAB) || (!ImGui::IsItemHovered() && ImGui::IsMouseClickedAnyButton()));
	//	ImGui::SameLine();
	//	needClearPtr = ImGui::SmallButton(std::string("<##" + label).data());


	//	//borders if can drop here : 
	//	if (ImGui::IsItemHovered() && canDropIntoField)
	//	{
	//		ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 2.f);
	//	}

	//	//drop resource : 
	//	FileHandler::CompletePath droppedResourcePath;
	//	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
	//	{
	//		DragAndDropManager::dropDraggedItem(&droppedResourcePath, (EditorDropContext::DropIntoResourceField));
	//		isTextEdited = true;
	//	}

	//	ImGui::PopStyleColor(colStyleCount);

	//	if (needClearPtr)
	//		resourcePtr.reset();
	//	else if (isTextEdited)
	//	{
	//		if (getResourceFactory<T>().contains(droppedResourcePath))
	//			resourcePtr = getResourceFactory<T>().get(droppedResourcePath);
	//		else
	//			resourcePtr.reset();
	//	}

	//	return isTextEdited;
	//}
	
	//Value fields : 
	template<typename T>
	bool ValueField(const std::string& label, T& value, FieldDisplayType displayType)
	{
		assert(0 && "invalid value type form this field.");
		return false;
	}

	template<typename T>
	bool ValueField(const std::string& label, T& value, const T& minValue, const T& maxValue, FieldDisplayType displayType)
	{
		assert(0 && "invalid value type form this field.");
		return false;
	}

	//resource field 
	//template<typename U>
	//bool ValueField(const std::string& label, ResourcePtr<U> resourcePtr)
	//{
	//	return ResourceField(label, resourcePtr);
	//}

	//template<>
	//bool ValueField<Texture>(const std::string& label, ResourcePtr<Texture> resourcePtr)
	//{
	//	return ResourceField(label, resourcePtr);
	//}

	//template<>
	//bool ValueField<Texture>(const std::string& label, ResourcePtr<Texture> resourcePtr)
	//{
	//	return ResourceField(label, resourcePtr);
	//}

	//string field
	bool ValueField(const std::string& label, std::string& outString, char* buf, int bufSize);


	//drag field

	//float
	template<>
	bool ValueField<float>(const std::string& label, float& value, const float& minValue, const float& maxValue, FieldDisplayType displayType);

	//input field

	//float 
	template<>
	bool ValueField<float>(const std::string& label, float& value, FieldDisplayType displayType);

	template<>
	bool ValueField<glm::vec2>(const std::string& label, glm::vec2& value, FieldDisplayType displayType);

	template<>
	bool ValueField<glm::vec3>(const std::string& label, glm::vec3& value, FieldDisplayType displayType);

	template<>
	bool ValueField<glm::vec4>(const std::string& label, glm::vec4& value, FieldDisplayType displayType);

	//int 
	template<>
	bool ValueField<int>(const std::string& label, int& value, FieldDisplayType displayType);

	template<>
	bool ValueField<glm::ivec2>(const std::string& label, glm::ivec2& value, FieldDisplayType displayType);

	template<>
	bool ValueField<glm::ivec3>(const std::string& label, glm::ivec3& value, FieldDisplayType displayType);

	template<>
	bool ValueField<glm::ivec4>(const std::string& label, glm::ivec4& value, FieldDisplayType displayType);

}

//// END : Editor fields
///////////////////////////////////////////