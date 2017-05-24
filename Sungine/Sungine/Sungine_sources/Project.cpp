

#include "Project.h"
#include "Application.h"
#include "InputHandler.h"
#include "Factories.h"
#include "Scene.h"
#include "Editor.h"
#include "FileHandler.h"
//forwards : 
#include "DebugDrawer.h"
#include "Renderer.h"

ProjectAsynchronousLoadScene::ProjectAsynchronousLoadScene(Project* project, const std::string& sceneName) 
	: m_project(project)
	, m_sceneName(sceneName)
{

}

void ProjectAsynchronousLoadScene::execute()
{
	m_project->loadScene(m_sceneName);
}

ProjectAsynchronousCreateAndSwitchScene::ProjectAsynchronousCreateAndSwitchScene(Project* project)
	: m_project(project)
{

}
void ProjectAsynchronousCreateAndSwitchScene::execute()
{
	m_project->createAndSwitchToNewScene();
}

/////////////////////////////////////

void onWindowResize(GLFWwindow* window, int width, int height)
{
	Application::get().setWindowResize(true);
	Application::get().setWindowWidth(width);
	Application::get().setWindowHeight(height);
}

void onFilesDrop(GLFWwindow* window, int count, const char** paths)
{
	Editor::instance().onFilesDropped(count, paths);
}

/////////////////////////////////////

//statics : 
FileHandler::Path Project::m_projectPath = FileHandler::Path();
FileHandler::Path Project::m_assetFolderPath = FileHandler::Path();
FileHandler::Path Project::m_shaderFolderPath = FileHandler::Path();
FileHandler::Path Project::m_scenesFolderPath = FileHandler::Path();
FileHandler::Path Project::m_projectInfosFolderPath = FileHandler::Path();
FileHandler::Path Project::m_engineResourcesFolderPath = FileHandler::Path();

/////////////////////////////////////

Project::Project() : m_activeSceneName(""), m_renderer(nullptr), m_activeScene(nullptr)
{
	m_newSceneName[0] = '\0';
}

Project::~Project()
{
	clear();
}

void Project::init()
{
	//int width = 1024, height= 768;
	int width = 1024, height = 680;
	float widthf = (float)width, heightf = (float)height;
	double t;
	float fps = 0.f;

	///////////////////// SET APPLICATION GLOBAL PARAMETERS /////////////////////
	Application::get().setWindowWidth(width);
	Application::get().setWindowHeight(height);

	GLFWwindow* window = initGLFW(width, height);
	Application::get().setActiveWindow(window);
	InputHandler::attachToWindow(window);
	initGlew();
	setupWindow(window);

	//initProject(); //TODO 10

	//add default assets to factories : 
	//initDefaultAssets();

	// Init viewer structures
	//Camera camera;
	//camera_defaults(camera);
	//GUIStates guiStates;
	//init_gui_states(guiStates);



	//// create and initialize our light manager
	//LightManager* lightManager = new LightManager();

	//// renderer : 
	//m_renderer = new Renderer(lightManager, "aogl.vert", "aogl_gPass.frag", "aogl_lightPass.vert", "aogl_lightPass_pointLight.frag", "aogl_lightPass_directionalLight.frag", "aogl_lightPass_spotLight.frag"); // call lightManager.init()
	//m_renderer->initPostProcessQuad("blit.vert", "blit.frag");
	//m_renderer->initialyzeShadowMapping("shadowPass.vert", "shadowPass.frag", "shadowPassOmni.vert", "shadowPassOmni.frag", "shadowPassOmni.geom");

	// Our scene : 
	//Scene scene(renderer);
}

void Project::clear()
{
	m_activeSceneName = "";

	//clear scene :
	if(m_activeScene != nullptr)
		delete m_activeScene;
	m_activeScene = nullptr;
	
	m_scenes.clear();
	m_scenesStatus.clear();

	//clear systems : 
	if(m_renderer != nullptr)
		delete m_renderer;

	//clear all resources : 
	clearAllResourceFactories();
}

void Project::open(const FileHandler::Path & projectAbsolutePath)
{
	clear(); //clear the current project (scenes + resources + systems)

	m_projectPath = projectAbsolutePath;
	m_assetFolderPath = FileHandler::Path("assets/");
	assert(FileHandler::directoryExists(m_assetFolderPath));
	m_shaderFolderPath = FileHandler::Path("engineResources/shaders/");
	assert(FileHandler::directoryExists(m_shaderFolderPath));
	m_scenesFolderPath = FileHandler::Path("scenes/");
	assert(FileHandler::directoryExists(m_scenesFolderPath));
	m_projectInfosFolderPath = FileHandler::Path("projectInfos/");
	assert(FileHandler::directoryExists(m_projectInfosFolderPath));
	m_engineResourcesFolderPath = FileHandler::Path("engineResources/");
	assert(FileHandler::directoryExists(m_engineResourcesFolderPath));

	//we have to set projectPath before calling initProject
	initProject(); //init systems and resources

	//load the project.
	load();

	//create a default sceen if there are no scene in the project.
	if (m_scenes.size() == 0)
	{
		m_activeScene = new Scene(m_renderer);
		loadDefaultScene(m_activeScene);
	}
}

void Project::open()
{
	clear(); //clear the current project (scenes + resources + systems)

	m_assetFolderPath = FileHandler::Path("assets/");
	assert(FileHandler::directoryExists(m_assetFolderPath));
	m_shaderFolderPath = FileHandler::Path("engineResources/shaders/");
	assert(FileHandler::directoryExists(m_shaderFolderPath));
	m_scenesFolderPath = FileHandler::Path("scenes/");
	assert(FileHandler::directoryExists(m_scenesFolderPath));
	m_projectInfosFolderPath = FileHandler::Path("projectInfos/");
	assert(FileHandler::directoryExists(m_projectInfosFolderPath));
	m_engineResourcesFolderPath = FileHandler::Path("engineResources/");
	assert(FileHandler::directoryExists(m_engineResourcesFolderPath));

	//we have to set projectPath before calling initProject
	initProject(); //init systems and resources

	//load the project.
	load();

	//create a default sceen if there are no scene in the project.
	if (m_scenes.size() == 0)
	{
		m_activeScene = new Scene(m_renderer);
		loadDefaultScene(m_activeScene);
	}
}


bool Project::activeSceneExists()
{
	if (m_activeScene == nullptr || m_activeScene->getName() == "")
		return false;
	
	FileHandler::CompletePath activeScenePath(m_scenesFolderPath.toString() + "/" + m_activeScene->getName() + ".json");
	return FileHandler::fileExists(activeScenePath);
}

void Project::saveProjectInfos()
{
	// TODO

	//Json::Value rootProject;
	//rootProject["sceneCount"] = m_scenes.size();
	//int sceneIdx = 0;
	//for (auto& it = m_scenes.begin(); it != m_scenes.end(); it++)
	//{
	//	rootProject["sceneInfos"][sceneIdx]["name"] = it->first;
	//	rootProject["sceneInfos"][sceneIdx]["path"] = it->second.toString();
	//	rootProject["sceneInfos"][sceneIdx]["status"] = m_scenesStatus[it->first];
	//	sceneIdx++;
	//}
	//rootProject["activeSceneName"] = m_activeSceneName;

	//std::ofstream streamProject;
	//streamProject.open(m_projectPath.toString() + "/projectInfos.txt");
	//if (!streamProject.is_open())
	//{
	//	std::cout << "error, can't save project infos at path : " << m_projectPath.toString() << std::endl;
	//	return;
	//}
	//streamProject << rootProject;
}


void Project::save()
{
	//Save active scene :
	saveActiveScene();
	//Save resources :
	saveResources();

	m_scenesStatus[m_activeSceneName] = SceneStatus::EDITED;
}

void Project::saveResources()
{
	FileHandler::CompletePath resourcesPath(m_projectInfosFolderPath.toString() + "/resources.json");

	//save resources : 
	Json::Value rootResources;
	getMeshFactory().save(rootResources["meshFactory"]);
	getMaterialFactory().save(rootResources["materialFactory"]);
	getTextureFactory().save(rootResources["textureFactory"]);
	getCubeTextureFactory().save(rootResources["cubeTextureFactory"]);

	std::ofstream streamResources;
	streamResources.open(resourcesPath.toString());
	if (!streamResources.is_open())
	{
		std::cout << "error, can't save resources at path : " << resourcesPath.toString() << std::endl;
		return;
	}
	streamResources << rootResources;
}

void Project::loadResources()
{
	//load resources : 
	FileHandler::CompletePath resourcePath(m_projectInfosFolderPath.toString() + "/resources.txt");

	std::ifstream streamResources;
	streamResources.open(resourcePath.toString());
	if (!streamResources.is_open())
	{
		std::cout << "error, can't load project resources at path : " << resourcePath.toString() << std::endl;
		return;
	}
	Json::Value rootResources;
	streamResources >> rootResources;

	loadResourcesInAllFactories(rootResources);
}

void Project::load()
{
	//load project infos : 
	// TODO
	//FileHandler::CompletePath projectPath(m_projectPath, "projectInfos", ".txt");
	//std::ifstream streamProject;
	//streamProject.open(projectPath.toString());
	//if (!streamProject.is_open())
	//{
	//	std::cout << "error, can't load project infos at path : " << projectPath.toString() << std::endl;
	//	return;
	//}

	// Load resources :
	loadResources();

	//load scenes : 
	// TODO

	//Json::Value rootProject;
	//streamProject >> rootProject;

	//int sceneCount = rootProject.get("sceneCount", 0).asInt();
	//for (int i = 0; i < sceneCount; i++)
	//{
	//	std::string sceneName = rootProject["sceneInfos"][i]["name"].asString();
	//	FileHandler::CompletePath scenePath( m_projectPath.toString() + "scenes/", sceneName, ".txt");
	//	SceneStatus sceneStatus = (SceneStatus)rootProject["sceneInfos"][i]["status"].asInt();
	//	m_scenes[sceneName] = scenePath;
	//	m_scenesStatus[sceneName] = sceneStatus;
	//}

	////load the active scene : 
	//m_activeSceneName = rootProject.get("activeSceneName", "default").asString();
	//if (m_activeScene != nullptr)
	//	delete m_activeScene;
	//m_activeScene = new Scene(m_renderer, m_activeSceneName);
	//m_activeScene->load(m_scenes[m_activeSceneName]);

}

void Project::playEdit()
{
	//TODO
}

void Project::play()
{
	//TODO
}

void Project::edit()
{
	//Create the editor : 
	Editor& editor = Editor::instance();

	GLFWwindow* window = Application::get().getActiveWindow();
	Scene* scene = getActiveScene();

	glfwSetDropCallback(window, onFilesDrop);

	float t = 0;

	//main loop
	do
	{
		scene = getActiveScene();

		t = glfwGetTime();
		ImGui_ImplGlfwGL3_NewFrame();

		//get main camera : 
		CameraEditor& currentEditorCamera = editor.getCamera();

		scene->clearReflectivePlanes();
		scene->setupReflectivePlanes();
		if (!editor.getIsPlaying())
			scene->setupReflectivePlanes(currentEditorCamera.getObjectID(), currentEditorCamera);

		scene->computeCulling();
		//add culling for editor camera if we use it
		if(!editor.getIsPlaying())
			scene->computeCullingForSingleCamera(currentEditorCamera);

		//Physics : 
		scene->updatePhysic(Application::get().getFixedDeltaTime(), currentEditorCamera, editor.getIsPlaying());

		//check if window has been resized by user
		if (Application::get().getWindowResize())
		{
			//TODO : resize only if full screen
			//if (editor.getIsPlaying())
			//	scene->getRenderer().onResizeViewport( glm::vec2(Application::get().getWindowWidth(), Application::get().getWindowHeight()) );

			scene->getRenderer().onResizeWindow(glm::vec2(Application::get().getWindowWidth(), Application::get().getWindowHeight()));
			editor.onResizeWindow();

			Application::get().setWindowResize(false);
		}

		//rendering :
		// TODO : Symplify this
		///////////////////////////////////////////////
		//BEGIN : RENDERING THE SCENE
		//renderer.render(camera, entities);
		if (editor.getIsPlaying())
		{
			scene->render(editor.getViewportRenderTarget());
		}
		else
		{
			scene->renderForEditor(editor.getCamera(), editor.getViewportRenderTarget(), editor.getDebugDrawRenderer());
			CHECK_GL_ERROR("gl error in renderer.");
			//scene->renderPaths(currentCamera);
			//CHECK_GL_ERROR("");
			//scene->renderColliders(currentCamera);
			//CHECK_GL_ERROR("");
			//scene->renderDebugLights(currentCamera);
			//CHECK_GL_ERROR("");
			//scene->renderDebugOctrees(currentCamera);
			//CHECK_GL_ERROR("");
			//scene->renderDebugPhysic(currentCamera);
			//CHECK_GL_ERROR("");

			DebugDrawer::render(currentEditorCamera, editor.getViewportRenderTarget());
			DebugDrawer::clear();

			editor.renderGizmo(currentEditorCamera, editor.getViewportRenderTarget());
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // TODO : movethis ?
		//END : RENDERING THE SCENE
		///////////////////////////////////////////////

		//Update behaviours :
		if (editor.getIsPlaying())
		{
			scene->updateControllers(Application::get().getFixedDeltaTime());
			scene->updateAnimations(Application::get().getTime());
			scene->updateBehaviours();
		}


		//update editor : 
		editor.update(*scene, window);

		//synchronize input handler : 
		InputHandler::pullInputsFromWindow();

#if 1
		/*
		ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("aogl");
		ImGui::SliderFloat("Material Specular Power", &(brickMaterial.specularPower), 0.0f, 100.f);
		lightManager.drawUI();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		*/

		///////////////////////////////////////////////
		//BEGIN : RENDERING THE HUD

		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		editor.renderUI(*this);

		DragAndDropManager::updateDragAndDrop(); // Must be called before ImGui::Render()

		ImGui::Render();

		glDisable(GL_BLEND);

		//END : RENDERING THE HUD
		///////////////////////////////////////////////
#endif

		///////////////////////////////////////////////
		//BEGIN : LATE UPDATES
		lateUpdate();
		//END : LATE UPDATES
		///////////////////////////////////////////////


		// Check for errors
		CHECK_GL_ERROR("End loop");

		glfwSwapBuffers(window);
		glfwPollEvents();

		double newTime = glfwGetTime();
		Application::get().setDeltaTime( newTime - t );
		Application::get().setFPS( 1.f / (newTime - t) );

	} // Check if the ESC key was pressed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);

}

void Project::exitApplication()
{
	// Close OpenGL window and terminate GLFW
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

Scene* Project::getActiveScene() const
{
	return m_activeScene;
}

Project::SceneStatus Project::getActiveSceneStatus() const
{
	auto findIt = m_scenesStatus.find(m_activeSceneName);
	if (findIt == m_scenesStatus.end())
		return Project::SceneStatus::TEMPORARY;
	else
		return findIt->second;
}

void Project::loadScene(const std::string& sceneName)
{
	FileHandler::CompletePath newScenePath(m_scenesFolderPath.toString() + "/" + sceneName + ".json");

	//m_activeScene->clear();
	delete m_activeScene;
	m_activeScene = new Scene(m_renderer, sceneName);
	m_activeScene->load(newScenePath);
	m_activeSceneName = m_activeScene->getName();

	//if (m_scenes.find(sceneName) == m_scenes.end())
	//	return;

	//if (m_activeScene != nullptr)
	//{
	//	if (m_scenes.find(m_activeSceneName) != m_scenes.end())
	//	{
	//		FileHandler::CompletePath activeScenePath = m_scenes[m_activeSceneName];// m_path + "/scenes/" + m_activeSceneName + ".txt";
	//		addDirectories( FileHandler::Path(m_projectPath.toString() + "scenes/"));
	//		m_activeScene->save(activeScenePath);
	//	}

	//	//m_activeScene->clear();
	//	delete m_activeScene;
	//}

	//m_activeScene = new Scene(m_renderer, sceneName);
	//if (m_scenesStatus[sceneName] == SceneStatus::DEFAULT) {
	//	loadDefaultScene(m_activeScene);
	//}
	//else {
	//	m_activeScene->load(m_scenes[sceneName]);
	//}
	//m_activeSceneName = sceneName;

	//saveProjectInfos();
}

//void Project::addDefaultScene(const std::string& sceneName)
//{
//	if (m_scenes.find(sceneName) != m_scenes.end())
//		return;
//
//	FileHandler::CompletePath scenePath(FileHandler::Path(m_projectPath.toString() + "scenes/"), sceneName, ".txt");
//	m_scenes[sceneName] = scenePath;
//	m_scenesStatus[sceneName] = SceneStatus::DEFAULT;
//
//	//saveProjectInfos(); TODO
//}
//
//void Project::addSceneFromActive(const std::string& sceneName)
//{
//	if (m_scenes.find(sceneName) != m_scenes.end())
//		return;
//
//	FileHandler::CompletePath scenePath(FileHandler::Path(m_projectPath.toString() + "scenes/"), sceneName, ".txt");
//	m_scenes[sceneName] = scenePath;
//	m_scenesStatus[sceneName] = SceneStatus::EDITED;
//
//	if (m_activeScene != nullptr)
//		m_activeScene->save(scenePath);
//
//	saveProjectInfos();
//
//
//	//if (m_activeScene != nullptr)
//	//{
//	//	std::string activeScenePath = m_scenes[m_activeSceneName]; // m_path + "/scenes/" + m_activeSceneName + ".txt";
//	//	addDirectories(m_path + "/scenes/");
//	//	m_activeScene->save(activeScenePath);
//	//	
//	//	m_activeScene->clear();
//	//	//delete m_activeScene;
//	//}
//
//	////m_activeScene = new Scene(m_renderer, sceneName);
//	//loadDefaultScene(m_activeScene);
//	//m_activeSceneName = sceneName;
//}

void Project::saveActiveScene()
{
	assert(m_activeScene->getName() != "");

	FileHandler::CompletePath activeScenePath(m_scenesFolderPath.toString() + "/" + m_activeScene->getName() + ".json");
	assert(FileHandler::fileExists(activeScenePath));
	if (!FileHandler::fileExists(activeScenePath))
		return;

	m_activeScene->setName(activeScenePath.getFilename());
	m_activeScene->save(activeScenePath);

	//if (m_activeScene != nullptr && m_scenes.find(m_activeSceneName) != m_scenes.end()) {
	//	m_activeScene->save(m_scenes[m_activeSceneName]);
	//	m_scenesStatus[m_activeSceneName] = SceneStatus::EDITED;
	//}
	//else {
	//	std::cout << "can't save the scene, besause the active scene is a temporary scene." << std::endl;
	//}

	//saveProjectInfos();
}

void Project::saveAsActiveScene(const FileHandler::CompletePath & scenePath)
{
	m_activeScene->setName(scenePath.getFilename());
	m_activeScene->save(scenePath);
}

void Project::reloadActiveScene()
{
	if (m_activeScene == nullptr)
		return;

	delete m_activeScene;
	m_activeScene = new Scene(m_renderer, m_activeSceneName);

	if (m_activeScene != nullptr) {
		m_activeScene->load(m_scenes[m_activeSceneName]);
	}
}

void Project::createAndSwitchToNewScene()
{
	if (m_activeScene != nullptr)
		delete m_activeScene;
	m_activeScene = new Scene(m_renderer);
	loadDefaultScene(m_activeScene);
}

void Project::createAndSwitchToNewSceneAsynchrone()
{
	m_commandsForLateUpdate.push_back(std::make_unique<ProjectAsynchronousCreateAndSwitchScene>(this));
}

void Project::loadSceneAsynchrone(const std::string & sceneName)
{
	m_commandsForLateUpdate.push_back(std::make_unique<ProjectAsynchronousLoadScene>(this, sceneName));
}

void Project::lateUpdate()
{
	for (auto& command : m_commandsForLateUpdate)
	{
		command->execute();
	}

	m_commandsForLateUpdate.clear();
}

void Project::loadDefaultScene(Scene* scene)
{

	// populate the scene :

	// mesh renderer for colliders : 
	MeshRenderer cubeWireFrameRenderer;
	cubeWireFrameRenderer.setMesh( getMeshFactory().getDefault("cubeWireframe") );
	cubeWireFrameRenderer.setMaterial( getMaterialFactory().getDefault("wireframe"), 0);

	//int r = 5;
	//float omega = 0;
	//for (int i = 0; i < 100; i++)
	//{
	//	Entity* newEntity = new Entity(&scene);
	//	BoxCollider* boxColliderLight = new BoxCollider(&cubeWireFrameRenderer);
	//	PointLight* pointLight = new PointLight(10, glm::vec3(rand() % 255 / 255.f, rand() % 255 / 255.f, rand() % 255 / 255.f), glm::vec3(0,0,0));
	//	newEntity->add(boxColliderLight).add(pointLight);
	//	newEntity->setTranslation(glm::vec3(r*std::cosf(omega), 2.f, r*std::sinf(omega)));
	//	
	//	scene.add(newEntity);

	//	omega += 0.4f;

	//	if(i % 10 == 0)
	//		r += 5;
	//}

	// an entity with a light : 
	Entity* newEntity = new Entity(scene);
	BoxCollider* boxColliderLight = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
	//SpotLight* spotLight = new SpotLight(10, glm::vec3(rand() % 255 / 255.f, rand() % 255 / 255.f, rand() % 255 / 255.f), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
	PointLight* pointLight = new PointLight(10, glm::vec3(rand() % 255 / 255.f, rand() % 255 / 255.f, rand() % 255 / 255.f), glm::vec3(0, 0, 0));
	pointLight->setBoundingBoxVisual(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
	newEntity->add(boxColliderLight).add(pointLight);
	newEntity->setTranslation(glm::vec3(0, 1.5, 0));
	newEntity->setName("point light");


	//renderers : 
	MeshRenderer* cubeRenderer01 = new MeshRenderer(getMeshFactory().getDefault("cube"), getMaterialFactory().getDefault("brick"));
	MeshRenderer* cubeRenderer02 = new MeshRenderer(getMeshFactory().getDefault("cube"), getMaterialFactory().getDefault("brick"));

	//MeshRenderer cubeRenderer02;
	//cubeRenderer02.mesh = &cube;
	//cubeRenderer02.material = &brickMaterial;

	MeshRenderer* planeRenderer = new MeshRenderer(getMeshFactory().getDefault("plane"), getMaterialFactory().getDefault("brick"));

	//colliders : 
	BoxCollider* boxCollider01 = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
	BoxCollider* boxCollider02 = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));

	//entities : 

	//cube entity 01
	Entity* entity_cube01 = new Entity(scene);
	entity_cube01->add(cubeRenderer01);
	entity_cube01->add(boxCollider01);
	entity_cube01->setName("cube");
	entity_cube01->setTranslation(glm::vec3(3, 0, 0));
	//cube entity 02
	Entity* entity_cube02 = new Entity(scene);
	entity_cube02->add(cubeRenderer02);
	entity_cube02->add(boxCollider02);
	entity_cube02->setName("cube");
	entity_cube02->setTranslation(glm::vec3(3, -2, 0));
	entity_cube02->setScale(glm::vec3(10, 1, 10));
	/*
	Entity* entity_cube03= new Entity(&scene);
	entity_cube03->add(cubeRenderer03);
	entity_cube03->add(boxCollider03);
	entity_cube03->setScale(glm::vec3(10, 1, 10));
	entity_cube03->setTranslation(glm::vec3(4, 2, 0));
	entity_cube03->setRotation( glm::quat( glm::vec3(0, 0, -glm::pi<float>()*0.5f) ));
	//cube entity 04
	Entity* entity_cube04 = new Entity(&scene);
	entity_cube04->add(cubeRenderer04);
	entity_cube04->add(boxCollider04);
	entity_cube04->setTranslation(glm::vec3(0, -2, 0));
	entity_cube04->setScale(glm::vec3(10, 1, 10));
	entity_cube04->setTranslation(glm::vec3(0, 2, 4));
	entity_cube04->setRotation(glm::quat(glm::vec3(-glm::pi<float>()*0.5f, 0, 0)));
	*/

	//flage entity : 
	Physic::Flag* flag = new Physic::Flag(getMaterialFactory().getDefault("brick"));

	Entity* entity_flag = new Entity(scene);
	entity_flag->add(new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe")));
	entity_flag->add(flag);
	entity_flag->setName("flag");
	entity_flag->endCreation();
}

void Project::setName(const std::string & name)
{
	assert(false);
	//TODO
}

std::string Project::getName()
{
	assert(false);
	//TODO
	return "";
}

void Project::setPath(const FileHandler::Path& path)
{
	assert(false);
	//TODO
	//m_projectPath = path;
}

const FileHandler::Path& Project::getAbsolutePath()
{
	return m_projectPath;
}

const FileHandler::Path& Project::getAssetsFolderPath()
{
	return m_assetFolderPath;
}

const FileHandler::Path& Project::getShaderFolderPath()
{
	return m_shaderFolderPath;
}

const FileHandler::Path & Project::getScenesFolderPath()
{
	return m_scenesFolderPath;
}

const FileHandler::Path & Project::getEngineResourcesFolderPath()
{
	return m_engineResourcesFolderPath;
}

FileHandler::CompletePath Project::getAbsolutePathFromRelativePath(const FileHandler::CompletePath& relativeCompletePath)
{
	return relativeCompletePath; // FileHandler::CompletePath(getPath().toString() + relativeCompletePath.toString());
}

FileHandler::Path Project::getAbsolutePathFromRelativePath(const FileHandler::Path& relativePath)
{
	return relativePath; // FileHandler::Path(getPath().toString() + relativePath.toString());
}

bool Project::isPathPointingInsideProjectFolder(const FileHandler::Path& path)
{
	return false; // path.toString().find(getPath().toString()) != std::string::npos;
}

bool Project::isPathPointingInsideProjectFolder(const FileHandler::CompletePath& completePath)
{
	return false; // completePath.toString().find(getPath().toString()) != std::string::npos;
}
//
//void Project::drawUI()
//{
//	if (m_scenes.find(m_activeSceneName) == m_scenes.end()) {
//		ImGui::Text("Temporary scene, please add a new scene to the project if you want to save your scene.");
//	}
//	else {
//		ImGui::Text(("active scene : " + m_activeSceneName).c_str());
//		ImGui::SameLine();
//		if (ImGui::Button("save")) {
//			saveActiveScene();
//		}
//	}
//
//
//	ImGui::InputText("new scene", m_newSceneName, 30);
//	ImGui::SameLine();
//	if (ImGui::Button("add blank")) {
//		addDefaultScene(m_newSceneName);
//	}
//	ImGui::SameLine();
//	if(ImGui::Button("add from active")) {
//		addSceneFromActive(m_newSceneName);
//	}
//
//	for (auto& it = m_scenes.begin(); it != m_scenes.end(); it++)
//	{
//		ImGui::PushID(it->first.c_str());
//		ImGui::Text(it->first.c_str());
//		ImGui::SameLine();
//		if (ImGui::Button("load"))
//			loadScene(it->first);
//		ImGui::PopID();
//	}
//}

GLFWwindow* Project::initGLFW(int width, int height)
{

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#if defined(__APPLE__)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	int const DPI = 2; // For retina screens only
#else
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	int const DPI = 1;
# endif

	// Open a window and create its OpenGL context
	GLFWwindow* window = glfwCreateWindow(width / DPI, height / DPI, "aogl", 0, 0);
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	return window;
}

void Project::initGlew()
{
	// Init glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}
}

void Project::initImGUI()
{
	//TODO
}

void Project::setupWindow(GLFWwindow* window)
{
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Enable vertical sync (on cards that support it)
	glfwSwapInterval(1);
	GLenum glerr = GL_NO_ERROR;
	glerr = glGetError();

	ImGui_ImplGlfwGL3_Init(window, true);

	//set the resize window callback 
	glfwSetWindowSizeCallback(window, onWindowResize);
}

void Project::initDefaultAssets()
{
	//Important : We init all default resources
	initAllResourceFactories();


	Texture* diffuseTexture = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/spnza_bricks_a_diff.tga"));
	Texture* specularTexture = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/spnza_bricks_a_spec.tga"));
	Texture* bumpTexture = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/spnza_bricks_a_normal.png"));

	Texture* grassTextureDiffuse = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/grass/grass01.png"));
	grassTextureDiffuse->textureWrapping_u = GL_CLAMP_TO_EDGE;
	grassTextureDiffuse->textureWrapping_v = GL_CLAMP_TO_EDGE;

	Texture* texSkyRight = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/skyboxes/right.png"));
	Texture* texSkyLeft = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/skyboxes/left.png"));
	Texture* texSkyTop = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/skyboxes/top.png"));
	Texture* texSkyBottom = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/skyboxes/top.png"));
	Texture* texSkyFront = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/skyboxes/front.png"));
	Texture* texSkyBack = new Texture(FileHandler::CompletePath(Project::getEngineResourcesFolderPath().toString() + "textures/skyboxes/back.png"));

	getTextureFactory().addDefaultResource("skyRight", texSkyRight);
	getTextureFactory().addDefaultResource("skyLeft", texSkyLeft);
	getTextureFactory().addDefaultResource("skyTop", texSkyTop);
	getTextureFactory().addDefaultResource("skyBottom", texSkyBottom);
	getTextureFactory().addDefaultResource("skyFront", texSkyFront);
	getTextureFactory().addDefaultResource("skyBack", texSkyBack);

	std::vector<ResourcePtr<Texture>> skyboxTextureBundle = {
		getTextureFactory().getDefault("skyRight"),
		getTextureFactory().getDefault("skyLeft"),
		getTextureFactory().getDefault("skyTop"),
		getTextureFactory().getDefault("skyBottom"),
		getTextureFactory().getDefault("skyFront"),
		getTextureFactory().getDefault("skyBack")
	};

	CubeTexture* defaultSkybox = new CubeTexture(skyboxTextureBundle);

	//force texture initialisation
	diffuseTexture->initGL();
	specularTexture->initGL();
	bumpTexture->initGL();
	grassTextureDiffuse->initGL();

	//////////////////// BEGIN RESSOURCES : 
	//the order between resource initialization and factories initialisation is important, indeed it's the factory which set set name of the different ressources when they are added to the factories.
	// So initialyzing materials before TextureFectory initialysation will create materials with wrong texture and mesh names. 

	////////// INITIALYZE DEFAULT TEXTURES IN FACTORY : 
	getTextureFactory().addDefaultResource("brickDiffuse", diffuseTexture);
	getTextureFactory().addDefaultResource("brickSpecular", specularTexture);
	getTextureFactory().addDefaultResource("brickBump", bumpTexture);
	getTextureFactory().addDefaultResource("grass01Diffuse", grassTextureDiffuse);

	// materials : 
	MaterialLit* brickMaterial = new MaterialLit(*getProgramFactory().getDefault("lit"));
	brickMaterial->setInternalData<Texture>("Diffuse", getTextureFactory().getDefault("brickDiffuse"));
	brickMaterial->setInternalData<Texture>("Bump", getTextureFactory().getDefault("brickBump"));
	brickMaterial->setInternalData<Texture>("Specular", getTextureFactory().getDefault("brickSpecular"));

	////////// INITIALYZE DEFAULT MATERIALS IN FACTORY : 
	getMaterialFactory().addDefaultResource("brick", brickMaterial);

	getCubeTextureFactory().addDefaultResource("plaineSkybox", defaultSkybox);

	///////////////////// END RESSOURCES 

}

void Project::initProject()
{
	//we need set the projectPath before to call this function.
	//assert(m_projectPath.toString() != "");

	initDefaultAssets();

	// create and initialize our light manager
	LightManager* lightManager = new LightManager();

	// renderer : 
	m_renderer = new Renderer(lightManager); // call lightManager.init()
	//m_renderer->initPostProcessQuad("blit.vert", "blit.frag");
	//m_renderer->initialyzeShadowMapping("shadowPass.vert", "shadowPass.frag", "shadowPassOmni.vert", "shadowPassOmni.frag", "shadowPassOmni.geom");
}
