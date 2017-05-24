#pragma once

#include <map>
#include <string>
#include <memory>

#include "glew/glew.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include "GLFW/glfw3.h"

#include "FileHandler.h"
#include "IAsynchronousCommand.h"

//forwards : 
class Renderer;
class Scene;
struct GLFWwindow;
class Project;

class ProjectAsynchronousLoadScene final : public IAsynchronousCommand
{
private:
	Project* m_project;
	std::string m_sceneName;
public:
	ProjectAsynchronousLoadScene(Project* project, const std::string& sceneName);
	void execute() override;
};

class ProjectAsynchronousCreateAndSwitchScene final : public IAsynchronousCommand
{
private:
	Project* m_project;
public:
	ProjectAsynchronousCreateAndSwitchScene(Project* project);
	void execute() override;
};

class Project
{
public : enum SceneStatus {DEFAULT, EDITED, TEMPORARY};

private:
	// mapping scenes name <-> scenes path 
	std::map<std::string, FileHandler::CompletePath> m_scenes;
	std::map<std::string, SceneStatus> m_scenesStatus;
	std::string m_activeSceneName;
	Scene* m_activeScene;

	Renderer* m_renderer;

	//meta : 
	static FileHandler::Path m_projectPath;
	static FileHandler::Path m_assetFolderPath;
	static FileHandler::Path m_shaderFolderPath;
	static FileHandler::Path m_scenesFolderPath;
	static FileHandler::Path m_projectInfosFolderPath;
	static FileHandler::Path m_engineResourcesFolderPath;

	//For ui : 
	char m_newSceneName[30];

	std::vector<std::unique_ptr<IAsynchronousCommand>> m_commandsForLateUpdate;

public:
	Project();
	~Project();

	//create and init default assets for the project.
	void init();
	//deallocate project memory.
	void clear();
	void open(const FileHandler::Path& projectAbsolutePath);
	void open();

	// Return true if the active scene exists in your hard drive i.e : it has already been saved at learst one time.
	bool activeSceneExists();
	void saveProjectInfos();
	// Save all scenes and resources.
	void save();
	// Save only resources.
	void saveResources();
	//load a project (scenes and resources).
	void load();
	//load project resources.
	void loadResources();

	void playEdit();
	void play();
	void edit();
	void exitApplication();

	Scene* getActiveScene() const;
	Project::SceneStatus getActiveSceneStatus() const;

	// Load the scene with given name (search in m_scenesFolderPath/)
	void loadScene(const std::string& sceneName);

	//void addDefaultScene(const std::string& sceneName);
	//void addSceneFromActive(const std::string& sceneName);

	// Save the active scene. The scene must have a valid name !
	void saveActiveScene();
	// Set scene name, then save the scene at given path.
	void saveAsActiveScene(const FileHandler::CompletePath& scenePath);
	// Delete the active scene, then recreate it and reload it. 
	void reloadActiveScene();
	// Delete and recreate a scene, set it as the active scene.
	void createAndSwitchToNewScene();

	// Asynchronous version of createAndSwitchToNewScene().
	void createAndSwitchToNewSceneAsynchrone();
	// Asynchronous version of loadScene().
	void loadSceneAsynchrone(const std::string& sceneName);
	// Update which append at the very end of the frame, will deals with the asynchronous commands.
	void lateUpdate();
	// Load the default scene. You must call it on an empty scene.
	void loadDefaultScene(Scene* scene);

	//void drawUI();
	
	//accessing global meta data about project.
	static void setName(const std::string& name);
	static std::string getName();
	static void setPath(const FileHandler::Path& path);
	static const FileHandler::Path& getAbsolutePath();
	static const FileHandler::Path& getAssetsFolderPath();
	static const FileHandler::Path& getShaderFolderPath();
	static const FileHandler::Path& getScenesFolderPath();
	static const FileHandler::Path& getEngineResourcesFolderPath();

	static FileHandler::CompletePath getAbsolutePathFromRelativePath(const FileHandler::CompletePath& relativeCompletePath);
	static FileHandler::Path getAbsolutePathFromRelativePath(const FileHandler::Path& relativePath);

	static bool isPathPointingInsideProjectFolder(const FileHandler::Path& path);
	static bool isPathPointingInsideProjectFolder(const FileHandler::CompletePath& completePath);

private :
	GLFWwindow* Project::initGLFW(int width, int height);
	void initGlew();
	void initImGUI();
	void setupWindow(GLFWwindow* window);
	//void onWindowResize(GLFWwindow* window, int width, int height);
	void initDefaultAssets();
	void initProject();
};

