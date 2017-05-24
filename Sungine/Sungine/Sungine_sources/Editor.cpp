

#include "Editor.h"
//forwards :
#include "Scene.h"
#include "SceneAccessor.h"
#include "Application.h"
#include "Factories.h"
#include "InputHandler.h"
#include "Project.h"
#include "EditorGUI.h"
#include "EditorFrames.h"
#include "FrameBuffer.h"
#include "Renderer.h"

////////////////////////////////////////// GUI STATES

const float GUIStates::MOUSE_PAN_SPEED = 0.001f;
const float GUIStates::MOUSE_ZOOM_SPEED = 0.05f;
const float GUIStates::MOUSE_TURN_SPEED = 0.005f;

void init_gui_states(GUIStates & guiStates)
{
	guiStates.panLock = false;
	guiStates.turnLock = false;
	guiStates.zoomLock = false;
	guiStates.lockPositionX = 0;
	guiStates.lockPositionY = 0;
	guiStates.camera = 0;
	guiStates.time = 0.0;
	guiStates.playing = false;
}




/////////////////////////////////// EDITOR

Editor::Editor() : m_isGizmoVisible(true), m_isMovingGizmo(false), m_isUIVisible(true), m_multipleEditing(false)
, m_isPlaying(false), m_isOwningPlayer(true)
, m_currentSelectionType(SelectionType::ENTITY)
{
	m_savePath[0] = '\0';
	m_loadPath[0] = '\0';

	m_gizmo = new Gizmo(getMaterialFactory().getDefault("wireframe"), this);

	m_camera = new CameraEditor();
	m_camera->setFPSMode(true);
	//camera_defaults(*m_camera);

	init_gui_states(m_guiStates);

	//Models : 
	m_resourceTree = std::make_shared<ResourceTree>(Project::getAssetsFolderPath());
	m_inspector = std::make_shared<Inspector>(this);
	m_sceneHierarchy = std::make_shared<SceneHierarchy>(this);
	m_worldPropertiesTool = std::make_shared<WorldPropertiesTool>();
	m_debugDrawRenderer = std::make_shared<DebugDrawRenderer>();
	m_viewport = std::make_shared<Viewport>();

	//Main window : 
	//m_windowManager.getBackgroundWindow()->setNode(std::make_shared<EditorNode>(std::make_shared<EditorNodeUniqueDisplay>(), std::make_shared<EditorNode>(std::make_shared<ViewportEditorFrame>("Viewport")))); //Viewport
	m_windowManager.setBackgroundWindow(std::make_shared<EditorBackgroundWindow>(std::make_shared<ViewportEditorFrame>("Viewport", m_viewport, this)));

	//Create the style sheet
	m_styleSheet = std::make_shared<EditorStyleSheet>();
	//Apply default style sheet
	m_styleSheet->applyDefaultStyleSheet();
}

const EditorStyleSheet & Editor::getStyleSheet() const
{
	return *m_styleSheet;
}

DebugDrawRenderer& Editor::getDebugDrawRenderer() const
{
	return *m_debugDrawRenderer;
}

RenderTarget & Editor::getViewportRenderTarget() const
{
	return m_viewport->getRenderTarget();
}

float Editor::getMenuTopOffset() const
{
	return m_menuTopOffset;
}

void Editor::changeCurrentSelected(Entity* entity)
{
	//clearSelectedComponents();

	m_gizmo->setTarget(nullptr);
	m_currentEntitiesSelected.clear();
	addCurrentSelected(entity);
}

void Editor::changeCurrentSelected(std::vector<Entity*> entities)
{
	//clearSelectedComponents();

	m_gizmo->setTarget(nullptr);
	m_currentEntitiesSelected.clear();
	for (auto& e : entities)
	{
		addCurrentSelected(e);
	}
}

void Editor::addCurrentSelected(Entity * entity)
{
	if (entity == nullptr)
		return;

	m_currentEntitiesSelected.push_back(entity);

	if (m_gizmo != nullptr)
	{
		if(m_currentEntitiesSelected.size() == 1)
			m_gizmo->setTarget(m_currentEntitiesSelected.front()); // set a unique target
		else
			m_gizmo->setTargets(m_currentEntitiesSelected); //set multiple targets
	}

	m_currentSelectionType = SelectionType::ENTITY;
	//refreshSelectedComponents(false);
}

void Editor::removeCurrentSelected(Entity * entity)
{
	if (entity == nullptr)
		return;

	auto findIt = std::find(m_currentEntitiesSelected.begin(), m_currentEntitiesSelected.end(), entity);

	if (findIt != m_currentEntitiesSelected.end())
	{
		m_currentEntitiesSelected.erase(findIt);
	}

	if (m_gizmo != nullptr)
	{
		if (m_currentEntitiesSelected.size() == 1)
			m_gizmo->setTarget(m_currentEntitiesSelected.front()); // set a unique target
		else
			m_gizmo->setTargets(m_currentEntitiesSelected); //set multiple targets
	}

	m_currentSelectionType = SelectionType::ENTITY;
	//refreshSelectedComponents(true);
}

void Editor::toggleCurrentSelected(Entity* entity)
{
	if (entity == nullptr)
		return;

	auto findIt = std::find(m_currentEntitiesSelected.begin(), m_currentEntitiesSelected.end(), entity);

	if (findIt != m_currentEntitiesSelected.end())
		removeCurrentSelected(entity);
	else
		addCurrentSelected(entity);
}

const std::vector<Entity*>& Editor::getCurrentSelection() const
{
	return m_currentEntitiesSelected;
}

void Editor::getCurrentDrawableSelection(std::vector<IDrawableInInspector*>& drawableSelection) const
{
	if (m_currentSelectionType == SelectionType::ENTITY)
	{
		for (auto& entity : m_currentEntitiesSelected)
			drawableSelection.push_back(entity);
	}
	else if (m_currentSelectionType == SelectionType::RESOURCE)
	{
		drawableSelection.push_back(m_resourceTree->getSelectedResource());
	}
}

void Editor::renderGizmo(BaseCamera& camera, RenderTarget& renderTarget)
{
	glDisable(GL_DEPTH_TEST);

	renderTarget.bindFramebuffer();

	if (!m_isGizmoVisible)
		return;

	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();
	glm::mat4 projectionMatrix = camera.getProjectionMatrix(); // glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 viewMatrix = camera.getViewMatrix(); // glm::lookAt(m_camera->eye, m_camera->o, m_camera->up);

	m_gizmo->render(projectionMatrix, viewMatrix);

	renderTarget.unbindFramebuffer();

	glEnable(GL_DEPTH_TEST);

	CHECK_GL_ERROR("Render error into Editor::renderGizmo().");
}

void Editor::onResourceSelected()
{
	m_currentSelectionType = SelectionType::RESOURCE;
}

void Editor::onEntitySelected()
{
	m_currentSelectionType = SelectionType::ENTITY;
}

void Editor::deselectAll()
{
	m_currentEntitiesSelected.clear();
}

//void Editor::clearSelectedComponents()
//{
//	m_pointLights.clear();
//	m_directionlLights.clear();
//	m_spotLights.clear();
//	m_meshRenderers.clear();
//	m_colliders.clear();
//}
//
//void Editor::refreshSelectedComponents(bool clearComponentLists)
//{
//	if (clearComponentLists)
//	{
//		m_pointLights.clear();
//		m_directionlLights.clear();
//		m_spotLights.clear();
//		m_meshRenderers.clear();
//		m_colliders.clear();
//	}
//
//	for (auto& selected : m_currentSelected)
//	{
//		PointLight* pointLight = static_cast<PointLight*>(selected->getComponent(Component::POINT_LIGHT));
//		DirectionalLight* directionalLight = static_cast<DirectionalLight*>(selected->getComponent(Component::DIRECTIONAL_LIGHT));
//		SpotLight* spotLight = static_cast<SpotLight*>(selected->getComponent(Component::SPOT_LIGHT));
//		MeshRenderer* meshRenderer = static_cast<MeshRenderer*>(selected->getComponent(Component::MESH_RENDERER));
//		Collider* collider = static_cast<Collider*>(selected->getComponent(Component::COLLIDER));
//
//		if (pointLight != nullptr)
//		{
//			m_pointLights.push_back(pointLight);
//		}
//		if (directionalLight != nullptr)
//		{
//			m_directionlLights.push_back(directionalLight);
//		}
//		if (spotLight != nullptr)
//		{
//			m_spotLights.push_back(spotLight);
//		}
//		if (meshRenderer != nullptr)
//		{
//			m_meshRenderers.push_back(meshRenderer);
//		}
//		if (collider != nullptr)
//		{
//			m_colliders.push_back(collider);
//		}
//	}
//}

void Editor::displayMenuBar(Project& project)
{
	m_styleSheet->pushMenuStyle();

	Scene& scene = *project.getActiveScene();

	m_saveWindowOpen = false;
	m_loadWindowOpen = false;

	if (ImGui::BeginMainMenuBar())
	{
		drawMenuEntry_options(project);
		drawMenuEntry_visibilities(scene);
		drawMenuEntry_windows();
		drawMenuEntry_addEntity(scene);
		//drawMenuEntry_camera();
		drawMenuEntry_playModes(project);

		ImGui::SameLine();
		ImGui::Text("                     Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		m_menuTopOffset = ImGui::GetWindowHeight();

		ImGui::EndMainMenuBar();
	}

	m_styleSheet->popMenuStyle();
}

void Editor::drawMenuEntry_options(Project& project)
{
	if (ImGui::BeginMenu("options"))
	{
		if (ImGui::Selectable("save"))
		{
			//if (project.getName() == "")
			// Has the scene already been saved, ot is it a new scene ?
			if (!project.activeSceneExists())
			{
				// If it is a new scene, we give it a name.
				//m_saveWindowOpen = true; // Open popup to save scene
				m_windowManager.addModal(std::make_shared<SaveSceneAsEditorFrame>(&m_windowManager));
				project.saveResources(); // Save resources
			}
			else
			{
				// Otherwise, we juste save the scene.
				project.save(); // Save active scene + resources
			}
			//ImGui::OpenPopup("save window");
		}
		if (ImGui::Selectable("save as"))
		{
			//m_saveWindowOpen = true; // Open popup to save scene
			m_windowManager.addModal(std::make_shared<SaveSceneAsEditorFrame>(&m_windowManager));
			project.saveResources(); // Save resources
		}
		if (ImGui::Selectable("load"))
		{
			//m_loadWindowOpen = true;
			m_windowManager.addModal(std::make_shared<LoadSceneEditorFrame>(&m_windowManager, this));
			m_needToSaveScene = true; // Tell the system that we may want to save the scene.
			//ImGui::OpenPopup("load window");
		}
		if (ImGui::Selectable("new"))
		{
			//m_loadWindowOpen = true;
			m_windowManager.addModal(std::make_shared<NewSceneEditorFrame>(&m_windowManager));
			m_needToSaveScene = true; // Tell the system that we may want to save the scene.
									  //ImGui::OpenPopup("load window");
		}

		ImGui::EndMenu();
	}
}

void Editor::drawMenuEntry_visibilities(Scene& scene)
{
	if (ImGui::BeginMenu("toggle visibility"))
	{
		if (ImGui::RadioButton("colliders visibility", scene.getAreCollidersVisible()))
		{
			scene.toggleColliderVisibility();
		}

		if (ImGui::RadioButton("debug deferred visibility", scene.getIsDebugDeferredVisible()))
		{
			scene.toggleDebugDeferredVisibility();
		}

		if (ImGui::RadioButton("debug physic visibility", scene.getIsDebugPhysicVisible()))
		{
			scene.toggleDebugPhysicVisibility();
		}

		if (ImGui::RadioButton("gizmo visibility", m_isGizmoVisible))
		{
			toggleGizmoVisibility();
		}

		if (ImGui::RadioButton("light boundingBox visibility", scene.getAreLightsBoundingBoxVisible()))
		{
			scene.toggleLightsBoundingBoxVisibility();
		}

		if (ImGui::RadioButton("octrees visibility", scene.getAreOctreesVisible()))
		{
			scene.toggleOctreesVisibility();
		}

		ImGui::EndMenu();
	}

}

void Editor::drawMenuEntry_playModes(Project& project)
{
	if (!m_isPlaying)
	{
		if (ImGui::Button("play"))
			launchGameInEditMode(project);
	}
	else {
		if (ImGui::Button("stop"))
		{
			stopGameInEditMode(project);
		}
		ImGui::SameLine();
		if (m_isOwningPlayer)
		{
			if (ImGui::Button("eject"))
			{
				ejectPlayerFromPawn();
			}
		}
		else
		{
			if (ImGui::Button("possess"))
			{
				possessPawn();
			}
		}
	}
}
//
//void Editor::drawMenuEntry_camera()
//{
//	if (ImGui::BeginMenu("camera mode"))
//	{
//		if (ImGui::RadioButton("editor camera", !m_cameraFPS))
//		{
//			if (m_cameraFPS)
//			{
//				//CameraEditor* newCam = new CameraEditor();
//				////newCam->switchFromCameraFPS(*m_camera); //set up the camera
//				//delete m_camera;
//				//m_camera = newCam;
//				//toogleCamera(*m_camera);
//				m_camera->setFPSMode(false);
//				m_cameraFPS = false;
//			}
//		}
//		if (ImGui::RadioButton("FPS camera", m_cameraFPS))
//		{
//			if (!m_cameraFPS)
//			{
//				//CameraFPS* newCam = new CameraFPS();
//				////newCam->switchFromCameraEditor(*m_camera); //set up the camera
//				//delete m_camera;
//				//m_camera = newCam;
//				//toogleCamera(*m_camera);
//				m_camera->setFPSMode(true);
//				m_cameraFPS = true;
//			}
//		}
//		if (ImGui::RadioButton("hide cursor", m_hideCursorWhenMovingCamera))
//		{
//			m_hideCursorWhenMovingCamera = !m_hideCursorWhenMovingCamera;
//		}
//		ImGui::SliderFloat("camera base speed", &m_cameraBaseSpeed, 0.01f, 1.f);
//		ImGui::SliderFloat("camera boost speed", &m_cameraBoostSpeed, 0.01f, 1.f);
//		float tmpFloat = m_camera->getFOV();
//		if (ImGui::SliderFloat("camera fov", &(tmpFloat), 0.f, glm::pi<float>()))
//			m_camera->setFOV(tmpFloat);
//
//		tmpFloat = m_camera->getNear();
//		if (ImGui::SliderFloat("camera near", &(tmpFloat), 0.001f, 5.f))
//			m_camera->setNear(tmpFloat);
//
//		tmpFloat = m_camera->getFar();
//		if (ImGui::SliderFloat("camera far", &(tmpFloat), 0.01f, 1000.f))
//			m_camera->setFar(tmpFloat);
//
//		tmpFloat = m_camera->getAspect();
//		if (ImGui::SliderFloat("camera aspect", &(tmpFloat), 0.01f, 10.f))
//			m_camera->setAspect(tmpFloat);
//
//		ImGui::EndMenu();
//	}
//}

void Editor::drawMenuEntry_addEntity(Scene& scene)
{
	const glm::vec3& cameraPosition = m_camera->getCameraPosition();
	const glm::vec3& cameraForward = m_camera->getCameraForward();
	glm::vec3 newEntityPosition = cameraForward;
	newEntityPosition *= 3.f;
	newEntityPosition += cameraPosition;

	if (ImGui::BeginMenu("Add default entities"))
	{
		if (ImGui::Button("add empty entity"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			newEntity->add(newCollider);

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}
		ImGui::SameLine();
		if (ImGui::Button("add pointLight"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto light = new PointLight();
			light->setBoundingBoxVisual(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			newEntity->add(newCollider).add(light);
			newEntity->setName("point light");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}
		ImGui::SameLine();
		if (ImGui::Button("add directionalLight"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto light = new DirectionalLight();
			newEntity->add(newCollider).add(light);
			newEntity->setName("directional light");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}
		ImGui::SameLine();
		if (ImGui::Button("add spotLight"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto light = new SpotLight();
			light->setBoundingBoxVisual(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			newEntity->add(newCollider).add(light);
			newEntity->setName("spot light");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}

		if (ImGui::Button("add cube"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto meshRenderer = new MeshRenderer(getMeshFactory().getDefault("cube"), getMaterialFactory().getDefault("brick"));
			newEntity->add(newCollider).add(meshRenderer);
			newEntity->setName("cube");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}

		if (ImGui::Button("add Camera"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto camera = new Camera();
			newEntity->add(newCollider).add(camera);
			newEntity->setName("camera");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}

		if (ImGui::Button("add flag"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto flag = new Physic::Flag(getMaterialFactory().getDefault("defaultLit"), 10);
			newEntity->add(newCollider).add(flag);
			newEntity->setName("flag");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}

		if (ImGui::Button("add path point"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto pathPoint = new PathPoint();
			newEntity->add(newCollider).add(pathPoint);
			newEntity->setName("path point");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}

		if (ImGui::Button("add wind zone"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto windZone = new Physic::WindZone();
			newEntity->add(newCollider).add(windZone);
			newEntity->setName("wind zone");

			newEntity->setTranslation(newEntityPosition);
			changeCurrentSelected(newEntity);
		}

		if (ImGui::Button("add billboard"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto billboard = new Billboard();
			newEntity->add(newCollider).add(billboard);
			newEntity->setName("billboard");
		}

		if (ImGui::Button("add particle emitter"))
		{
			auto newEntity = new Entity(&scene);
			auto newCollider = new BoxCollider(getMeshFactory().getDefault("cubeWireframe"), getMaterialFactory().getDefault("wireframe"));
			auto particleEmitter = new Physic::ParticleEmitter();
			newEntity->add(newCollider).add(particleEmitter);
			newEntity->setName("particle emitter");
		}


		ImGui::EndMenu();
	}
}

void Editor::drawMenuEntry_windows()
{
	if (ImGui::BeginMenu("Windows"))
	{

		if (!m_windowManager.isFrameOpen("Viewport") && ImGui::Button("Viewport"))
		{
			m_windowManager.addWindow(std::make_shared<ViewportEditorFrame>("Viewport", m_viewport, this));
		}
		if (!m_windowManager.isFrameOpen("SceneHierarchy") && ImGui::Button("SceneHierarchy"))
		{
			m_windowManager.addWindow(std::make_shared<SceneHierarchyEditorFrame>("SceneHierarchy", m_sceneHierarchy));
		}
		if (!m_windowManager.isFrameOpen("WorldProperties") && ImGui::Button("WorldProperties"))
		{
			m_windowManager.addWindow(std::make_shared<WorldPropertiesEditorFrame>("WorldProperties", m_worldPropertiesTool));
		}
		if (!m_windowManager.isFrameOpen("Inspector") && ImGui::Button("Inspector"))
		{
			m_windowManager.addWindow(std::make_shared<InspectorEditorFrame>("Inspector", m_inspector));
		}
		if (!m_windowManager.isFrameOpen("Resource tree") && ImGui::Button("Resource tree"))
		{
			m_windowManager.addWindow(std::make_shared<ResourceTreeView>("Resource tree", m_resourceTree.get(), this));
		}
		//if (!m_windowManager.isFrameOpen("Terrain tool") && ImGui::Button("Terrain tool"))
		//{
		//	m_windowManager.addWindow(std::make_shared<TerrainToolEditorFrame>("Terrain tool"));
		//}
		//if (!m_windowManager.isFrameOpen("Skybox tool") && ImGui::Button("Skybox tool"))
		//{
		//	m_windowManager.addWindow(std::make_shared<SkyboxToolEditorFrame>("Skybox tool"));
		//}
		//if (!m_windowManager.isFrameOpen("Scene manager") && ImGui::Button("Scene manager"))
		//{
		//	m_windowManager.addWindow(std::make_shared<SceneManagerEditorFrame>("Scene manager"));
		//}
		if (!m_windowManager.isFrameOpen("Debug renderer") && ImGui::Button("Debug renderer"))
		{
			m_windowManager.addWindow(std::make_shared<DebugRenderEditorFrame>("Debug renderer", m_debugDrawRenderer));
		}
		if (!m_windowManager.isFrameOpen("Factories debuger") && ImGui::Button("Factories debuger"))
		{
			m_windowManager.addWindow(std::make_shared<FactoriesDebugEditorFrame>("Factories debuger"));
		}
		if (!m_windowManager.isFrameOpen("Camera Editor") && ImGui::Button("Camera Editor"))
		{
			m_windowManager.addWindow(std::make_shared<EditorCameraEditorFrame>("Camera Editor", m_camera));
		}
		ImGui::EndMenu();
	}
}

void Editor::launchGameInEditMode(Project& project)
{
	if (project.getActiveSceneStatus() == Project::SceneStatus::TEMPORARY) {
		std::cout << "can't play the scene in a temporary scene, please save your scene." << std::endl;
		return;
	}

	changeCurrentSelected(nullptr);
	//toggleDebugVisibility(*project.getActiveScene());

	//save the current scene : 
	project.saveActiveScene();
	//setPlaying mode to true : 
	m_isPlaying = true;
	possessPawn();
}

void  Editor::stopGameInEditMode(Project& project)
{
	changeCurrentSelected(nullptr);
	//toggleDebugVisibility(*project.getActiveScene());

	//setPlaying mode to false : 
	m_isPlaying = false;
	//reload the current scene : 
	project.reloadActiveScene();
	ejectPlayerFromPawn();
}

ResourceTree* Editor::getResourceTree() const
{
	return m_resourceTree.get();
}

EditorWindowManager* Editor::getWindowManager()
{
	return &m_windowManager;
}

//////////////////////////////////////////////////////////
////////////// BEGIN : MODALS HANDLING 

void Editor::displayModals(Project& project)
{
	//modal windows : 
	if (m_saveWindowOpen)
		ImGui::OpenPopup("save window");
	if (m_loadWindowOpen)
		ImGui::OpenPopup("load window");

	//auto& modalIter = m_editorModals.begin();
	//while (modalIter != m_editorModals.end())
	//{
	//	(*modalIter)->drawAsModal();

	//	//close modal ?
	//	if ((*modalIter)->shouldCloseModale())
	//	{
	//		modalIter = m_editorModals.erase(modalIter);
	//	}
	//	else
	//		modalIter++;
	//}

	m_windowManager.displayModals(project, *this);

	////load : 
	//bool loadModalWindowOpen = true;
	//if (ImGui::BeginPopupModal("load window", &loadModalWindowOpen))
	//{
	//	ImGui::Text("Would you like to save the current scene ?");
	//	if (ImGui::Button("Yes"))
	//	{

	//		project.saveActiveScene();
	//		m_needToSaveScene = false;
	//	}
	//	ImGui::SameLine();
	//	if (ImGui::Button("No"))
	//	{
	//		m_needToSaveScene = false;
	//	}

	//	std::vector<std::string> outSceneNames;
	//	FileHandler::getAllFileNames(project.getScenesFolderPath(), outSceneNames);
	//	for (int i = 0; i < outSceneNames.size(); i++)
	//	{
	//		if (ImGui::Button(outSceneNames[i].c_str()))
	//		{
	//			project.loadScene(outSceneNames[i]);
	//			ImGui::CloseCurrentPopup();
	//		}
	//	}

	//	ImGui::EndPopup();

	//	//ImGui::InputText("project name", &m_loadPath[0], 60);
	//	//if (ImGui::Button("load"))
	//	//{
	//	//	std::string loadPath = ("save/" + std::string(m_loadPath));

	//	//	//Verify the validity of path :
	//	//	std::vector<std::string> dirNames;
	//	//	FileHandler::getAllDirNames(FileHandler::Path("save/"), dirNames);
	//	//	bool dirAlreadyExists = (std::find(dirNames.begin(), dirNames.end(), std::string(m_loadPath)) != dirNames.end());

	//	//	if (m_loadPath != "" && dirAlreadyExists)
	//	//	{
	//	//		project.open(m_loadPath, loadPath);
	//	//		//scene.clear(); //clear the previous scene
	//	//		//scene.load(loadPath);
	//	//		changeCurrentSelected(nullptr);
	//	//	}
	//	//	else
	//	//	{
	//	//		std::cout << "can't load project with name : " << m_loadPath << " no project found." << std::endl;
	//	//	}
	//	//	ImGui::CloseCurrentPopup();
	//	//}

	//	//ImGui::EndPopup();
	//}

	////save
	//bool saveModalWindowOpen = true;
	//if (ImGui::BeginPopupModal("save window", &saveModalWindowOpen))
	//{
	//	ImGui::InputText("set scene name", m_savePath, 60);
	//	FileHandler::CompletePath scenePath(project.getScenesFolderPath().toString() + m_savePath + ".json");
	//	if (!FileHandler::fileExists(scenePath))
	//	{
	//		if (ImGui::Button("save"))
	//		{
	//			project.saveAsActiveScene(scenePath);
	//		}
	//	}
	//	else
	//	{
	//		ImGui::Text("A scene with the same name already exists.");
	//	}

	//	//ImGui::InputText("set project name", m_savePath, 60);
	//	//if (ImGui::Button("save"))
	//	//{
	//	//	std::string savePath = ("save/" + std::string(m_savePath));

	//	//	//Verify the validity of path :
	//	//	std::vector<std::string> dirNames;
	//	//	FileHandler::getAllDirNames(FileHandler::Path("save/"), dirNames);
	//	//	bool dirAlreadyExists = (std::find(dirNames.begin(), dirNames.end(), std::string(m_savePath)) != dirNames.end());

	//	//	if (m_savePath != "" && !dirAlreadyExists)
	//	//	{
	//	//		project.setName(m_savePath);
	//	//		project.setPath(savePath);
	//	//		project.save();
	//	//	}
	//	//	else
	//	//	{
	//	//		std::cout << "can't save project with name : " << m_savePath << " a project with the same name was found." << std::endl;
	//	//	}

	//	//	//scene.save(savePath);
	//	//	ImGui::CloseCurrentPopup();
	//	//}

	//	//ImGui::EndPopup();
	//}
}

//void Editor::addModal(std::shared_ptr<EditorWindow> modal)
//{
//	m_editorModals.push_back(modal);
//}
//
//void Editor::removeModal(EditorWindow* modal)
//{
//	auto found = std::find_if(m_editorModals.begin(), m_editorModals.end(), [modal](const std::shared_ptr<EditorWindow>& item) { return item.get() == modal; });
//	if(found != m_editorModals.end())
//		m_editorModals.erase(found);
//}
//
//void Editor::displayTopLeftWindow(Project& project)
//{
//	Scene& scene = *project.getActiveScene();
//
//	int entityId = 0;
//
//	if (!m_currentSelected.empty())
//	{
//		//can't add or remove components in multiple editing, only change components parameters
//		if (ImGui::RadioButton("multiple editing", m_multipleEditing))
//		{
//			m_multipleEditing = !m_multipleEditing;
//
//			if (m_multipleEditing)
//			{
//				refreshSelectedComponents(true);
//			}
//		}
//
//		if (!m_multipleEditing)
//		{
//			for (auto& selected : m_currentSelected)
//			{
//				ImGui::PushID(entityId);
//				if (ImGui::CollapsingHeader(("entity " + patch::to_string(entityId)).c_str())) {
//					selected->drawUI(scene);
//				}
//				ImGui::PopID();
//
//				entityId++;
//			}
//		}
//		else
//		{
//			m_inspector.drawUI(m_currentSelected);
//			m_inspector.drawUI(m_pointLights);
//			m_inspector.drawUI(m_directionlLights);
//			m_inspector.drawUI(m_spotLights);
//			m_inspector.drawUI(m_meshRenderers);
//		}
//	}
//}

////////////// END : MODALS HANDLING 
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
////////////// BEGIN : DROPPEDFILES HANDLING 

void Editor::onFilesDropped(int count, const char** paths)
{
	for (int i = 0; i < count; i++)
	{
		if(FileHandler::getFileTypeFromExtention(FileHandler::getFileExtention(paths[i])) != FileHandler::FileType::NONE)
			m_droppedFiles.push_back(FileHandler::CompletePath(paths[i]));
	}

	m_windowManager.addModal(std::make_shared<DroppedFileEditorFrame>(this));

	//DroppedFileEditorWindow::openPopUp(*this);
}

size_t Editor::getDroppedFilesCount() const
{
	return m_droppedFiles.size();
}

const FileHandler::CompletePath& Editor::getDroppedFilesPath(int idx) const
{
	return m_droppedFiles[idx];
}

void Editor::clearDroppedFiles()
{
	m_droppedFiles.clear();
}

void Editor::removeDroppedFile(int idx)
{
	if (idx >= 0 && idx < m_droppedFiles.size())
		m_droppedFiles.erase(m_droppedFiles.begin() + idx);
}

void Editor::removeDroppedFile(const FileHandler::CompletePath& filePath)
{
	for (int i = 0; i < m_droppedFiles.size(); i++)
	{
		if (m_droppedFiles[i] == filePath)
		{
			m_droppedFiles.erase(m_droppedFiles.begin() + i);
			return;
		}
	}
}

////////////// END : DROPPEDFILES HANDLING 
//////////////////////////////////////////////////////////
//
//void Editor::displayBottomLeftWindow(Project& project)
//{
//	Scene& scene = *project.getActiveScene();
//
//	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.8, 0.8, 0.2) );
//	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2, 0.2, 0.2, 0.2));
//	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8, 0.8, 0.8, 0.2));
//
//	auto entities = scene.getEntities();
//
//	int entityId = 0;
//	bool setParenting = false;
//	Entity* parentToAttachSelected = nullptr;
//	for (auto& entity : entities)
//	{
//		if (!entity->hasParent())
//		{
//			displayTreeEntityNode(entity, entityId, setParenting, parentToAttachSelected);
//		}
//	}
//
//	if (setParenting)
//	{
//		if (parentToAttachSelected->getIsSelected())
//			parentToAttachSelected->setParent(nullptr);
//		else
//		{
//			for (int i = 0; i < m_currentSelected.size(); i++)
//			{
//				m_currentSelected[i]->setParent(parentToAttachSelected);
//			}
//		}
//	}
//
//	ImGui::PopStyleColor();
//	ImGui::PopStyleColor();
//	ImGui::PopStyleColor();
//}
//
//void Editor::displayBottomWindow(Project& project)
//{
//	Scene& scene = *project.getActiveScene();
//
//	ImGui::BeginChild("choose tool", ImVec2(200, ImGui::GetWindowHeight()));
//	if (ImGui::RadioButton("terrain tool", m_terrainToolVisible))
//	{
//		hideAllToolsUI();
//		m_terrainToolVisible = true;
//	}
//	if (ImGui::RadioButton("skybox tool", m_skyboxToolVisible))
//	{
//		hideAllToolsUI();
//		m_skyboxToolVisible = true;
//	}
//	if (ImGui::RadioButton("texture factory", m_textureFactoryVisible))
//	{
//		hideAllToolsUI();
//		m_textureFactoryVisible = true;
//	}
//	if (ImGui::RadioButton("cube texture factory", m_cubeTextureFactoryVisible))
//	{
//		hideAllToolsUI();
//		m_cubeTextureFactoryVisible = true;
//	}
//	if (ImGui::RadioButton("mesh factory", m_meshFactoryVisible))
//	{
//		hideAllToolsUI();
//		m_meshFactoryVisible = true;
//	}
//	if (ImGui::RadioButton("program factory", m_programFactoryVisible))
//	{
//		hideAllToolsUI();
//		m_programFactoryVisible = true;
//	}
//	if (ImGui::RadioButton("material factory", m_materialFactoryVisible))
//	{
//		hideAllToolsUI();
//		m_materialFactoryVisible = true;
//	}
//	if (ImGui::RadioButton("skeletal animation factory", m_skeletalAnimationFactoryVisible))
//	{
//		hideAllToolsUI();
//		m_skeletalAnimationFactoryVisible = true;
//	}
//	if (ImGui::RadioButton("scene manager", m_sceneManagerVisible))
//	{
//		hideAllToolsUI();
//		m_sceneManagerVisible = true;
//	}
//	ImGui::EndChild();
//	
//	ImGui::SameLine();
//
//	if (m_terrainToolVisible)
//	{
//		ImGui::BeginChild("Terrain tool");
//		scene.getTerrain().drawUI();
//		ImGui::End();
//	}
//
//	if (m_skyboxToolVisible)
//	{
//		ImGui::BeginChild("Skybox tool");
//		scene.getSkybox().drawUI();
//		ImGui::End();
//	}
//	//%NOCOMMIT%
///*
//	if (m_textureFactoryVisible)
//	{
//		ImGui::BeginChild("Texture factory");
//		getTextureFactory().drawUI();
//		ImGui::End();
//	}
//
//	if (m_cubeTextureFactoryVisible)
//	{
//		ImGui::BeginChild("Cube Texture factory");
//		getCubeTextureFactory().drawUI();
//		ImGui::End();
//	}
//
//	if (m_meshFactoryVisible)
//	{
//		ImGui::BeginChild("Mesh factory");
//		getMeshFactory().drawUI();
//		ImGui::End();
//	}
//
//	if (m_programFactoryVisible)
//	{
//		ImGui::BeginChild("Program factory");
//		getProgramFactory().drawUI();
//		ImGui::End();
//	}
//
//	if (m_materialFactoryVisible)
//	{
//		ImGui::BeginChild("Material factory");
//		getMaterialFactory().drawUI();
//		ImGui::End();
//	}
//
//	if (m_skeletalAnimationFactoryVisible)
//	{
//		ImGui::BeginChild("SkeletalAnimation factory");
//		getSkeletalAnimationFactory().drawUI();
//		ImGui::End();
//	}*/
//
//	if (m_sceneManagerVisible)
//	{
//		ImGui::BeginChild("Scene manager");
//		project.drawUI();
//		ImGui::End();
//	}
//}

//void Editor::updatePanelSize(float topLeftWidth, float topLeftHeight, float bottomHeight)
//{
//	m_windowRect = glm::vec4(m_windowDecal.x, m_windowDecal.y, Application::get().getWindowWidth(), Application::get().getWindowHeight());
//	m_topLeftPanelRect = glm::vec4(m_windowRect.x, m_windowRect.y, topLeftWidth, topLeftHeight);
//	m_bottomLeftPanelRect = glm::vec4(m_windowRect.x, m_topLeftPanelRect.w, m_topLeftPanelRect.z, m_windowRect.w - m_topLeftPanelRect.w);
//	m_bottomPanelRect = glm::vec4(m_windowRect.x + m_topLeftPanelRect.z, m_windowRect.w - bottomHeight, m_windowRect.z - m_topLeftPanelRect.z, bottomHeight);
//}

void Editor::onResizeWindow()
{
	//m_windowRect.z = Application::get().getWindowWidth();
	//m_windowRect.w = Application::get().getWindowHeight();
	//updatePanelSize(m_topLeftPanelRect.z, m_topLeftPanelRect.w, m_bottomPanelRect.w);

	m_windowManager.onScreenResized();
}

void Editor::renderUI(Project& project)
{
	//We assure we are rendering the HUD in the default framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Scene& scene = *project.getActiveScene();
	m_inspector->setScene(&scene);
	m_sceneHierarchy->setScene(&scene);
	m_worldPropertiesTool->setScene(&scene);

	if (!m_isUIVisible)
		return;

	displayMenuBar(project);

	EditorNode::s_shouldDrawMouseCursorWithImGui = false;
	displayBackgroundWindow(project);
	displayFloatingWindows(project);

	displayModals(project);

	//%NOCOMMIT% test only
	ImGui::ShowTestWindow();

	//asynchonous commands : 
	m_windowManager.update();

	//We synchronize the render size of the editor viewport
	//Scene* activeScene = project.getActiveScene();
	//const glm::vec2 viewportDelta(m_viewport->getSize() - activeScene->getRenderer().getViewportRenderSize());
	//const bool isDraggingFrame = (DragAndDropManager::isDragAndDropping() && DragAndDropManager::getOperationType() == EditorDragAndDropType::EditorFrameDragAndDrop);
	//if ((std::abs(viewportDelta.x) > 2.f || std::abs(viewportDelta.y) > 2.f)
	//	&& !isDraggingFrame)
	//{
	//	// Resize render targets :
	//	activeScene->getRenderer().onResizeViewport(m_viewport->getSize());
	//	scene.onViewportResized(m_viewport->getSize());
	//	m_camera->onViewportResized(m_viewport->getSize());
	//}

}

void Editor::displayFloatingWindows(Project& project)
{
	//for (int i = 0; i < m_editorWindows.size(); i++)
	//{
	//	m_editorWindows[i]->drawAsWindow();
	//}
	m_styleSheet->pushFloatingWindowStyle();
	m_windowManager.displayFloatingWindows(project, *this);
	m_styleSheet->popFloatingWindowStyle();
}

void Editor::displayBackgroundWindow(Project& project)
{
	m_styleSheet->pushBackgroundWindowStyle();

	m_windowManager.displayBackgroundWindows(project, *this);

	m_styleSheet->popBackgroundWindowStyle();

	//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1, 0.1, 0.1, 255));

	////top left window :
	//ImGui::SetNextWindowSize(ImVec2(m_topLeftPanelRect.z, m_windowRect.w));
	//ImGui::SetNextWindowContentSize(ImVec2(m_topLeftPanelRect.z, m_windowRect.w));
	//ImGui::SetNextWindowPos(ImVec2(m_topLeftPanelRect.x, m_topLeftPanelRect.y));
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	//ImGui::Begin("leftWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders);
	//ImGui::BeginChild("leftWindowContent", ImVec2(m_topLeftPanelRect.z - 30, m_windowRect.w));

	//ImGui::SetNextWindowContentWidth(m_topLeftPanelRect.z - 30);
	//ImGui::BeginChild("topLeftWindowContent", ImVec2(m_topLeftPanelRect.z - 30, m_topLeftPanelRect.w - 16.f));
	//displayTopLeftWindow(project);
	//ImGui::EndChild();


	////ImGui::InvisibleButton("hSplitter0", ImVec2(m_topLeftPanelRect.z, 8.f));
	////if (ImGui::IsItemActive())
	////{
	////	m_topLeftPanelRect.w += ImGui::GetIO().MouseDelta.y;
	////	if (m_topLeftPanelRect.w < 10) m_topLeftPanelRect.w = 10;
	////	else if (m_topLeftPanelRect.w > m_windowRect.w - 20) m_topLeftPanelRect.w = m_topLeftPanelRect.w - 20;
	////	updatePanelSize(m_topLeftPanelRect.z, m_topLeftPanelRect.w, m_bottomPanelRect.w);
	////}
	//ImGui::Separator();
	//ImGui::InvisibleButton("hSplitter1", ImVec2(m_topLeftPanelRect.z, 8.f));
	//if (ImGui::IsItemActive())
	//{
	//	m_topLeftPanelRect.w += ImGui::GetIO().MouseDelta.y;
	//	if (m_topLeftPanelRect.w < 10) m_topLeftPanelRect.w = 10;
	//	else if (m_topLeftPanelRect.w > m_windowRect.w - 20) m_topLeftPanelRect.w = m_topLeftPanelRect.w - 20;
	//	updatePanelSize(m_topLeftPanelRect.z, m_topLeftPanelRect.w, m_bottomPanelRect.w);
	//}
	//ImGui::Separator();

	//ImGui::BeginChild("bottomLeftWindowContent", ImVec2(m_topLeftPanelRect.z - 30, m_bottomLeftPanelRect.w - 16.f));
	//displayBottomLeftWindow(project);
	//ImGui::EndChild();

	//ImGui::EndChild();
	//ImGui::SameLine();

	//ImGui::InvisibleButton("vSplitter", ImVec2(20.f, m_windowRect.w));
	//if (ImGui::IsItemActive())
	//{
	//	m_topLeftPanelRect.z += ImGui::GetIO().MouseDelta.x;
	//	if (m_topLeftPanelRect.z < 10) m_topLeftPanelRect.z = 10;
	//	else if (m_topLeftPanelRect.z > m_windowRect.z - 10) m_topLeftPanelRect.z = m_windowRect.z - 10;
	//	updatePanelSize(m_topLeftPanelRect.z, m_topLeftPanelRect.w, m_bottomPanelRect.w);
	//}

	//ImGui::End();
	//ImGui::PopStyleVar();

	////bottom window :
	//ImGui::SetNextWindowSize(ImVec2(m_bottomPanelRect.z, m_bottomPanelRect.w));
	//ImGui::SetNextWindowContentSize(ImVec2(m_bottomPanelRect.z, m_bottomPanelRect.w));
	//ImGui::SetNextWindowPos(ImVec2(m_bottomPanelRect.x, m_bottomPanelRect.y));
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	//ImGui::Begin("bottomWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_ShowBorders);

	//ImGui::InvisibleButton("hsplitter", ImVec2(m_bottomPanelRect.z, 20.f));
	////ImGui::Separator();
	//if (ImGui::IsItemActive())
	//{
	//	m_bottomPanelRect.w -= ImGui::GetIO().MouseDelta.y;
	//	if (m_bottomPanelRect.w < 10) m_bottomPanelRect.w = 10;
	//	else if (m_bottomPanelRect.w > m_windowRect.w - 20) m_bottomPanelRect.w = m_bottomPanelRect.w - 20;
	//	updatePanelSize(m_topLeftPanelRect.z, m_topLeftPanelRect.w, m_bottomPanelRect.w);
	//}

	//ImGui::BeginChild("bottomWindowContent");
	//displayBottomWindow(project);
	//ImGui::EndChild();

	//ImGui::End();
	//ImGui::PopStyleVar();

	//ImGui::PopStyleVar();
	//ImGui::PopStyleColor();

	////ImGui::BeginChild("bottomWindow", ImVec2(screenWidth, screenHeight - m_leftPanelHeight));
	////ImGui::EndChild();
}

bool Editor::testGizmoIntersection(const Ray & ray)
{

	m_gizmoTranslationDirection = m_gizmo->checkIntersection(ray, m_gizmoTranslationAnchor);
	if (m_gizmoTranslationDirection == Gizmo::GIZMO_ARROW_NONE)
		return false;
	else
		return true;
}

void Editor::beginMoveGizmo()
{
	m_isMovingGizmo = true;
}

bool Editor::isMovingGizmo()
{
	return m_isMovingGizmo;
}

void Editor::endMoveGizmo()
{
	m_isMovingGizmo = false;
}

void Editor::moveGizmo(const Ray & ray)
{
	float t;
	glm::vec3 normal;
	glm::vec3 normalX(1, 0, 0);
	glm::vec3 normalY(0, 1, 0);
	glm::vec3 normalZ(0, 0, 1);

	glm::vec3 direction;
	glm::vec3 directionX(1, 0, 0);
	glm::vec3 directionY(0, 1, 0);
	glm::vec3 directionZ(0, 0, 1);

	glm::vec3 tmpOldTranslationAnchor(m_gizmoTranslationAnchor);

	if (m_gizmoTranslationDirection == Gizmo::GIZMO_ARROW_X)
	{
		direction = directionX;

		if (glm::dot(normalY, ray.getDirection()) < 0.0001f)
			normal = normalZ;
		else
			normal = normalY;
			
		if (ray.intersectPlane(m_gizmoTranslationAnchor, normal, &t))
		{
			m_gizmoTranslationAnchor = ray.at(t);

			glm::vec3 deltaTranslation = m_gizmoTranslationAnchor - tmpOldTranslationAnchor;

			deltaTranslation = dot(deltaTranslation, direction) * direction;

			m_gizmo->translate(deltaTranslation);
		}
	}
	else if (m_gizmoTranslationDirection == Gizmo::GIZMO_ARROW_Y)
	{
		direction = directionY;

		if (glm::dot(normalZ, ray.getDirection()) < 0.0001f)
			normal = normalX;
		else
			normal = normalZ;

		if (ray.intersectPlane(m_gizmoTranslationAnchor, normal, &t))
		{
			m_gizmoTranslationAnchor = ray.at(t);

			glm::vec3 deltaTranslation = m_gizmoTranslationAnchor - tmpOldTranslationAnchor;

			deltaTranslation = dot(deltaTranslation, direction) * direction;

			m_gizmo->translate(deltaTranslation);
		}
	}
	else
	{
		direction = directionZ;

		if (glm::dot(normalX, ray.getDirection()) < 0.0001f)
			normal = normalY;
		else
			normal = normalX;

		if (ray.intersectPlane(m_gizmoTranslationAnchor, normal, &t))
		{
			m_gizmoTranslationAnchor = ray.at(t);

			glm::vec3 deltaTranslation = m_gizmoTranslationAnchor - tmpOldTranslationAnchor;

			deltaTranslation = dot(deltaTranslation, direction) * direction;

			m_gizmo->translate(deltaTranslation);
		}
	}
}

Entity* Editor::duplicateSelected()
{
	if (m_currentEntitiesSelected.empty())
		return nullptr;

	std::vector<Entity*> newEntities;

	for(auto selected : m_currentEntitiesSelected)
		newEntities.push_back( new Entity(*selected) ); //copy the entity

	changeCurrentSelected(newEntities); //change selection, to select the copy

	return newEntities.front();
}

void Editor::deleteSelected(Scene& scene)
{
	if (m_currentEntitiesSelected.empty())
		return;

	for (int i = 0; i < m_currentEntitiesSelected.size(); i++)
	{
		scene.getAccessor().removeFromScene(m_currentEntitiesSelected[i]);
		delete m_currentEntitiesSelected[i];
		m_currentEntitiesSelected[i] = nullptr;
	}

	changeCurrentSelected(nullptr);

}

void Editor::toggleUIVisibility()
{
	m_isUIVisible = !m_isUIVisible;
}

void Editor::toggleGizmoVisibility()
{
	m_isGizmoVisible = !m_isGizmoVisible;
}

void Editor::toggleDebugVisibility(Scene& scene)
{
	toggleUIVisibility();

	scene.setAreCollidersVisible(m_isUIVisible);
	scene.setIsDebugDeferredVisible(m_isUIVisible);
	scene.setAreLightsBoundingBoxVisible(m_isUIVisible);
	scene.setIsDebugPhysicVisible(m_isUIVisible);
	m_isGizmoVisible = m_isUIVisible;

}

void Editor::toggleLightsBoundingBoxVisibility(Scene& scene)
{
	scene.toggleLightsBoundingBoxVisibility();
}

void Editor::updateGuiStates(GLFWwindow* window)
{
	ImGuiIO& io = ImGui::GetIO();
	m_guiStates.mouseOverUI = io.WantCaptureMouse;
	m_guiStates.UICaptureKeyboard = io.WantCaptureKeyboard;

	// Mouse states
	m_guiStates.leftButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	m_guiStates.rightButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	m_guiStates.middleButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);

	m_guiStates.altPressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT);
	m_guiStates.shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
	m_guiStates.ctrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);

	m_guiStates.leftPressed = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
	m_guiStates.rightPressed = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
	m_guiStates.forwardPressed = (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS);
	m_guiStates.backwardPressed = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);

	if (m_guiStates.leftButton == GLFW_PRESS)
		m_guiStates.turnLock = true;
	else
		m_guiStates.turnLock = false;

	if (m_guiStates.rightButton == GLFW_PRESS)
		m_guiStates.zoomLock = true;
	else
		m_guiStates.zoomLock = false;

	if (m_guiStates.middleButton == GLFW_PRESS || (m_guiStates.leftButton == GLFW_PRESS && m_guiStates.altPressed))
		m_guiStates.panLock = true;
	else
		m_guiStates.panLock = false;
}

void Editor::updateCameraMovement_editor(GLFWwindow* window)
{
	// Camera movements
	if(glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL && !m_hideCursorWhenMovingCamera)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (!m_guiStates.altPressed && (m_guiStates.leftButton == GLFW_PRESS || m_guiStates.rightButton == GLFW_PRESS || m_guiStates.middleButton == GLFW_PRESS))
	{
		double x; double y;
		glfwGetCursorPos(window, &x, &y);
		m_guiStates.lockPositionX = x;
		m_guiStates.lockPositionY = y;
	}
	if (m_guiStates.altPressed == GLFW_PRESS || m_guiStates.shiftPressed == GLFW_PRESS)
	{
		if(m_hideCursorWhenMovingCamera)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		double mousex; double mousey;
		glfwGetCursorPos(window, &mousex, &mousey);
		int diffLockPositionX = mousex - m_guiStates.lockPositionX;
		int diffLockPositionY = mousey - m_guiStates.lockPositionY;

		if (m_guiStates.altPressed == GLFW_PRESS && m_guiStates.shiftPressed == GLFW_RELEASE)
		{
			if (m_guiStates.zoomLock)
			{
				float zoomDir = 0.0;
				if (diffLockPositionX > 0)
					zoomDir = -1.f;
				else if (diffLockPositionX < 0)
					zoomDir = 1.f;
				m_camera->translateLocal(glm::vec3(0, 0, GUIStates::MOUSE_ZOOM_SPEED * zoomDir));
				//camera_zoom(*m_camera, zoomDir * GUIStates::MOUSE_ZOOM_SPEED);
			}
			else if (m_guiStates.turnLock)
			{
				m_camera->rotate(diffLockPositionX * GUIStates::MOUSE_TURN_SPEED, diffLockPositionY * GUIStates::MOUSE_TURN_SPEED);

				//camera_turn(*m_camera, diffLockPositionY * GUIStates::MOUSE_TURN_SPEED,
				//	diffLockPositionX * GUIStates::MOUSE_TURN_SPEED);

			}
		}
		if (m_guiStates.altPressed == GLFW_PRESS && m_guiStates.shiftPressed == GLFW_PRESS)
		{
			if (m_guiStates.panLock)
			{
				m_camera->translateLocal( glm::vec3(diffLockPositionX * GUIStates::MOUSE_PAN_SPEED, diffLockPositionY * GUIStates::MOUSE_PAN_SPEED, 0));

/*				camera_pan(*m_camera, diffLockPositionX * GUIStates::MOUSE_PAN_SPEED,
					diffLockPositionY * GUIStates::MOUSE_PAN_SPEED)*/;
			}
		}

		m_guiStates.lockPositionX = mousex;
		m_guiStates.lockPositionY = mousey;
	}
}

void Editor::updateCameraMovement_fps(GLFWwindow* window)
{
	if (!m_guiStates.rightButton)
	{
		double x; double y;
		glfwGetCursorPos(window, &x, &y);
		m_guiStates.lockPositionX = x;
		m_guiStates.lockPositionY = y;
	}
	else
	{
		if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL && !m_hideCursorWhenMovingCamera)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		double mousex; double mousey;
		glfwGetCursorPos(window, &mousex, &mousey);
		int diffLockPositionX = mousex - m_guiStates.lockPositionX;
		int diffLockPositionY = mousey - m_guiStates.lockPositionY;

		m_camera->rotate(diffLockPositionX * GUIStates::MOUSE_TURN_SPEED, diffLockPositionY * GUIStates::MOUSE_TURN_SPEED);

		//camera_rotate(*m_camera, diffLockPositionY * GUIStates::MOUSE_TURN_SPEED,
		//	diffLockPositionX * GUIStates::MOUSE_TURN_SPEED);

		glfwSetCursorPos(window, m_guiStates.lockPositionX, m_guiStates.lockPositionY);
	}

	if (m_guiStates.altPressed || m_guiStates.ctrlPressed)
		return;

	if (!m_guiStates.UICaptureKeyboard)
	{
		float cameraSpeed = m_camera->getCameraBaseSpeed();
		if (m_guiStates.shiftPressed)
			cameraSpeed = m_camera->getCameraBoostSpeed();

		glm::vec3 translateDirection = glm::vec3(0,0,0);

		if (InputHandler::getKey(GLFW_KEY_Q, InputHandler::FOCUSING_EDITOR))
			translateDirection += glm::vec3(-1, 0, 0);
		if (InputHandler::getKey(GLFW_KEY_D, InputHandler::FOCUSING_EDITOR))
			translateDirection += glm::vec3(1, 0, 0);
		if (InputHandler::getKey(GLFW_KEY_Z, InputHandler::FOCUSING_EDITOR))
			translateDirection += glm::vec3(0, 0, 1);
		if (InputHandler::getKey(GLFW_KEY_S, InputHandler::FOCUSING_EDITOR))
			translateDirection += glm::vec3(0, 0, -1);

		if (translateDirection != glm::vec3(0, 0, 0))
		{
			translateDirection = glm::normalize(translateDirection) * cameraSpeed;
			m_camera->translateLocal(translateDirection);
		}


		//if (m_guiStates.leftPressed)
		//	camera_translate(*m_camera, -cameraSpeed, 0, 0);
		//if (m_guiStates.rightPressed)
		//	camera_translate(*m_camera, cameraSpeed, 0, 0);
		//if (m_guiStates.forwardPressed)
		//	camera_translate(*m_camera, 0, 0, cameraSpeed);
		//if (m_guiStates.backwardPressed)
		//	camera_translate(*m_camera, 0, 0, -cameraSpeed);
	}
}

bool Editor::getIsPlaying() const
{
	return m_isPlaying;
}

bool Editor::getIsOwningPlayer() const
{
	return m_isOwningPlayer;
}

void Editor::possessPawn()
{
	m_isOwningPlayer = true;
	InputHandler::setFocusState( (InputHandler::getFocusState() | InputHandler::FOCUSING_GAME) & ~InputHandler::FOCUSING_EDITOR);
}

void Editor::ejectPlayerFromPawn()
{
	m_isOwningPlayer = false;
	InputHandler::setFocusState((InputHandler::getFocusState() | InputHandler::FOCUSING_EDITOR) & ~InputHandler::FOCUSING_GAME);
}

void Editor::update(/*Camera & camera*/ Scene& scene, GLFWwindow* window)
{

	//const float viewportRenderWidth = m_viewport->getRenderSize().x;
	//const float viewportRenderHeight = m_viewport->getRenderSize().y;
	const float viewportWidth = m_viewport->getSize().x;
	const float viewportHeight = m_viewport->getSize().y;
	const float viewportPosX= m_viewport->getPosition().x;
	const float viewportPosY = m_viewport->getPosition().y;
	const bool isViewportHovered = m_viewport->getIsHovered();

	//Terrain tool
	/*if (m_windowManager.isFrameOpen("Terrain tool"))
	{
		if (isViewportHovered 
			&& InputHandler::getMouseButton(GLFW_MOUSE_BUTTON_1, InputHandler::FOCUSING_EDITOR)
			&& !m_guiStates.altPressed 
			&& !m_guiStates.ctrlPressed
			&& !m_guiStates.shiftPressed)
		{
			glm::vec3 origin = m_camera->getCameraPosition();
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);
			mouseX = (mouseX - viewportPosX);
			mouseY = (mouseY - viewportPosY);
			glm::vec3 direction = screenToWorld(mouseX, mouseY, viewportWidth, viewportHeight, *m_camera);

			Ray ray(origin, direction, 1000.f);
			CollisionInfo collisionInfo;

			if (ray.intersectTerrain(scene.getTerrain(), collisionInfo))
			{
				if(scene.getTerrain().getCurrentTerrainTool() == Terrain::DRAW_MATERIAL)
					scene.getTerrain().drawMaterialOnTerrain(collisionInfo.point);
				else if(scene.getTerrain().getCurrentTerrainTool() == Terrain::DRAW_GRASS)
					scene.getTerrain().drawGrassOnTerrain(collisionInfo.point);
			}
		}
	}*/

	//update gizmo
	float distanceToCamera = glm::length(m_camera->getCameraPosition() - m_gizmo->getPosition());
	m_gizmo->setScale(distanceToCamera*0.1f);

	updateGuiStates(window);

	//camera movements : 
	

		if (!m_camera->getFPSMode())
			updateCameraMovement_editor(window);
		else
			updateCameraMovement_fps(window);

	if (!m_guiStates.UICaptureKeyboard)
	{
		// ui visibility : 
		if (InputHandler::getKeyDown(GLFW_KEY_TAB, InputHandler::FOCUSING_EDITOR) && m_guiStates.ctrlPressed)
		{
			this->toggleDebugVisibility(scene);
		}

		//entity copy / past : 
		if (InputHandler::getKeyDown(GLFW_KEY_D, InputHandler::FOCUSING_EDITOR) && m_guiStates.ctrlPressed)
		{
			this->duplicateSelected();
		}

		//delete selected : 
		if (InputHandler::getKeyDown(GLFW_KEY_DELETE, InputHandler::FOCUSING_EDITOR))
		{
			this->deleteSelected(scene);
		}

	}

	if (isViewportHovered)
	{
		//object picking : 
		if (!m_guiStates.altPressed 
			&& !m_guiStates.ctrlPressed
			&& InputHandler::getMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT, InputHandler::FOCUSING_EDITOR))
		{

			glm::vec3 origin = m_camera->getCameraPosition();
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);
			mouseX = (mouseX - viewportPosX);
			mouseY = (mouseY - viewportPosY);
			glm::vec3 direction = screenToWorld(mouseX, mouseY, viewportWidth, viewportHeight, *m_camera);
			//direction = direction - origin;
			//direction = glm::normalize(direction);

			Ray ray(origin, direction, 1000.f);

			// intersection with gizmo
			if (this->testGizmoIntersection(ray))
			{
				this->beginMoveGizmo();
			}
			//intersection with a collider in the scene
			else
			{
				auto entities = scene.getEntities();
				float distanceToIntersection = 0;
				float minDistanceToIntersection = 0;
				Entity* selectedEntity = nullptr;
				for (int i = 0, intersectedCount = 0; i < entities.size(); i++)
				{
					Collider* collider = static_cast<Collider*>(entities[i]->getComponent(Component::ComponentType::COLLIDER));
					if (entities[i]->getComponent(Component::ComponentType::COLLIDER) != nullptr)
					{
						if (ray.intersect(*collider, &distanceToIntersection))
						{
							if (intersectedCount == 0 || distanceToIntersection < minDistanceToIntersection)
							{
								selectedEntity = entities[i];
								minDistanceToIntersection = distanceToIntersection;
							}
							intersectedCount++;

							//std::cout << "intersect a cube !!!" << std::endl;
							//ray.debugLog();
						}
					}
				}

				if (selectedEntity != nullptr)
				{
					if (!m_guiStates.shiftPressed)
						this->changeCurrentSelected(selectedEntity);
					else
						this->toggleCurrentSelected(selectedEntity);
				}
			}

		}

	}

	if (InputHandler::getMouseButtonUp(GLFW_MOUSE_BUTTON_LEFT, InputHandler::FOCUSING_EDITOR))
	{
		if (this->isMovingGizmo())
			this->endMoveGizmo();
	}
	if (InputHandler::getMouseButton(GLFW_MOUSE_BUTTON_LEFT, InputHandler::FOCUSING_EDITOR))
	{
		if (this->isMovingGizmo())
		{
			glm::vec3 origin = m_camera->getCameraPosition();
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);
			mouseX = (mouseX - viewportPosX);
			mouseY = (mouseY - viewportPosY);
			glm::vec3 direction = screenToWorld(mouseX, mouseY, viewportWidth, viewportHeight, *m_camera);
			Ray ray(origin, direction, 1000.f);

			this->moveGizmo(ray);
		}
	}
	
}

CameraEditor& Editor::getCamera()
{
	return *m_camera;
}

