#pragma once


#include <memory>
#include "EditorNodes.h"
#include "ResourceTree.h"

//forwards
class Editor;
class EditorModal;
class Inspector;
class SceneHierarchy;
class DebugDrawRenderer;
class Viewport;
class CameraEditor;
class EditorWindowManager;
class WorldPropertiesTool;
class AnimationStateMachine;


class SaveSceneAsEditorFrame : public EditorFrame
{
private:
	EditorWindowManager* m_windowManagerRef;
	char m_savePath[60];

public:
	SaveSceneAsEditorFrame(EditorWindowManager* windowManager);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

class LoadSceneEditorFrame : public EditorFrame
{
private:
	EditorWindowManager* m_windowManagerRef;
	bool m_needToSaveScene;
	Editor* m_editorPtr;

public:
	LoadSceneEditorFrame(EditorWindowManager* windowManager, Editor* editorPtr);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

class NewSceneEditorFrame : public EditorFrame
{
private:
	EditorWindowManager* m_windowManagerRef;
	char m_newPath[60];
	bool m_needToSaveScene;

public:
	NewSceneEditorFrame(EditorWindowManager* windowManager);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class DroppedFileEditorFrame : public EditorFrame
{
private:
	ResourceTreeView m_treeView;
	//static DroppedFileEditorWindow* modalRef;
	Editor* m_editorRef;

public:
	DroppedFileEditorFrame(Editor* editorRef);

	//static void openPopUp(Editor& editor);
	//static void closePopUp(Editor& editor);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class ViewportEditorFrame : public EditorFrame
{
private:
	std::weak_ptr<Viewport> m_viewport;
	Editor* m_editorRef;

public:
	ViewportEditorFrame(const std::string& name, std::shared_ptr<Viewport> model, Editor* editorRef);
	void drawContent(Project& project, EditorModal* parentWindow) override;
	void onFrameMoved() override;
	void onFrameResized() override;
};

/////////////////////////////////////////

//class TerrainToolEditorFrame : public EditorFrame
//{
//public:
//	TerrainToolEditorFrame(const std::string& name);
//	void drawContent(Project& project, EditorModal* parentWindow) override;
//};
//
///////////////////////////////////////////
//
//class SkyboxToolEditorFrame : public EditorFrame
//{
//public:
//	SkyboxToolEditorFrame(const std::string& name);
//	void drawContent(Project& project, EditorModal* parentWindow) override;
//};

/////////////////////////////////////////

//class SceneManagerEditorFrame : public EditorFrame
//{
//public:
//	SceneManagerEditorFrame(const std::string& name);
//	void drawContent(Project& project, EditorModal* parentWindow) override;
//};

/////////////////////////////////////////

class FactoriesDebugEditorFrame : public EditorFrame
{
private:
	bool m_textureFactoryVisible;
	bool m_cubeTextureFactoryVisible;
	bool m_meshFactoryVisible;
	bool m_programFactoryVisible;
	bool m_materialFactoryVisible;
	bool m_skeletalAnimationFactoryVisible;

public:
	FactoriesDebugEditorFrame(const std::string& name);
	void hideAllDebugViews();
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class InspectorEditorFrame : public EditorFrame
{
private:
	std::weak_ptr<Inspector> m_inspector;

public:
	InspectorEditorFrame(const std::string& name, std::shared_ptr<Inspector> model);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class DebugRenderEditorFrame : public EditorFrame
{
private:
	std::weak_ptr<DebugDrawRenderer> m_debugDrawRenderer;
public:
	DebugRenderEditorFrame(const std::string& name, std::shared_ptr<DebugDrawRenderer> model);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class SceneHierarchyEditorFrame : public EditorFrame
{
private:
	std::weak_ptr<SceneHierarchy> m_sceneHierarchy;

public:
	SceneHierarchyEditorFrame(const std::string& name, std::shared_ptr<SceneHierarchy> model);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class WorldPropertiesEditorFrame : public EditorFrame
{
private:
	std::weak_ptr<WorldPropertiesTool> m_worldPropertiesTool;

public:
	WorldPropertiesEditorFrame(const std::string& name, std::shared_ptr<WorldPropertiesTool> model);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class EditorCameraEditorFrame : public EditorFrame
{
private:
	CameraEditor* m_editorCamera;

public:
	EditorCameraEditorFrame(const std::string& name, CameraEditor* camera);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class SceneEditorFrame : public EditorFrame
{
private:
	Scene* m_scene;

public:
	SceneEditorFrame(const std::string& name, CameraEditor* camera);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class MVSEditorFrame : public EditorFrame
{
private:
	MVS::NodeManager* m_content;

public:
	MVSEditorFrame(MVS::NodeManager* content);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};

/////////////////////////////////////////

class ASMEditorFrame : public EditorFrame
{
private:
	Sungine::Animation::AnimationStateMachine* m_content;

public:
	ASMEditorFrame(Sungine::Animation::AnimationStateMachine* content);
	void drawContent(Project& project, EditorModal* parentWindow) override;
};
