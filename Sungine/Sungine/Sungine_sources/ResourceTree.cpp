

#include  <fstream>

#include "ResourceTree.h"
#include "imgui/imgui.h"
#include "imgui_extension.h"
#include "EditorGUI.h"
#include "Factories.h"
#include "project.h"
#include "Editor.h"


/////////// RESSOURCE FOLDER /////////////////

void ResourceFolder::fillDatasFromExplorerFolder(const FileHandler::Path& folderPath)
{
	std::vector<std::string> dirNames;
	FileHandler::getAllDirNames(folderPath, dirNames);

	for (auto& dirName : dirNames)
	{
		int subFolderIdx = 0;
		if (addSubFolder(dirName, &subFolderIdx))
		{
			m_subFoldersContainer[subFolderIdx].fillDatasFromExplorerFolder(FileHandler::Path(folderPath, dirName));
		}
	}

	std::vector<std::string> fileNames;
	FileHandler::getAllFileNames(folderPath, fileNames);
	std::string outExtention;

	for (auto& fileNameAndExtention : fileNames)
	{
		//We only add files that engine understand
		FileHandler::getExtentionFromExtendedFilename(fileNameAndExtention, outExtention);
		if (FileHandler::getFileTypeFromExtention(outExtention) != FileHandler::FileType::NONE)
		{
			addFile( fileNameAndExtention );
		}
	}
}

bool ResourceFolder::moveSubFolderToNewLocation(const std::string& subFolderName, ResourceFolder& newLocation)
{
	ResourceFolder* subFolder = getSubFolder(subFolderName);

	if (subFolder != nullptr)
	{
		subFolder->moveTo(newLocation);
		removeSubFolder(subFolderName);
	}

	return true;
}

bool ResourceFolder::copySubFolderToNewLocation(const std::string& subFolderName, ResourceFolder& newLocation)
{
	ResourceFolder* subFolder = getSubFolder(subFolderName);

	if (subFolder != nullptr)
	{
		subFolder->copyTo(newLocation);
	}

	return true;
}

bool ResourceFolder::moveTo(ResourceFolder& newLocation)
{
	copyTo(newLocation); //it's like a copy / paste, ...

	// ... but we delete all files and folder from the old folder (we delete it because it's a "move to")
	removeAllFiles();
	removeAllSubFolders();

	return true;
}

bool ResourceFolder::copyTo(ResourceFolder& newLocation)
{
	int newLocationIdx = 0;
	//add a new empty sub folder
	newLocation.addSubFolder(m_name, &newLocationIdx);
	ResourceFolder* newFolder = newLocation.getSubFolder(newLocationIdx);

	//transfers folder's files in the new empty folder
	for (auto& file : getFiles())
	{
		newFolder->addFile(file.getPath());
	}

	//transfers sub folders (use same finction recursivly)
	for (auto& subFolder : getSubFolders())
	{
		subFolder.moveTo(*newLocation.getSubFolder(newLocationIdx));
	}

	return true;
}

/////////// RESSOURCE TREE /////////////////

ResourceTree::ResourceTree(const FileHandler::Path& assetResourcePath)
	: ResourceFolder(FileHandler::Path("assets"))
{
	fillDatasFromExplorerFolder(assetResourcePath);
	resolvePointersLoadingInFactories();
}

ResourceFile * ResourceTree::getSelectedResource()
{
	ResourceFolder* folder = m_selectedFileFolderPath == getPath() ? this : getSubFolder(m_selectedFileFolderPath);
	if (folder != nullptr)
		return folder->getFile(m_selectedFileKey);
	else
		return nullptr;
}

const FileHandler::Path & ResourceTree::getSelectedFileFolderPath() const
{
	return m_selectedFileFolderPath;
}

const ResourceFileKey & ResourceTree::getSelectedFileKey() const
{
	return m_selectedFileKey;
}

void ResourceTree::setSelectedFileFolderPath(const FileHandler::Path & resourceFolderPath)
{
	m_selectedFileFolderPath = resourceFolderPath;
}

void ResourceTree::setSelectedFileKey(const ResourceFileKey & resourceKey)
{
	m_selectedFileKey = resourceKey;
}

void ResourceTree::deleteSubFolderFrom(const std::string& folderName, ResourceFolder& folderFrom)
{
	assert(folderFrom.getSubFolder(folderName) != nullptr);
	if (folderFrom.getSubFolder(folderName) == nullptr)
		return;

	ResourceFolder* folderToDelete = folderFrom.getSubFolder(folderName);
	FileHandler::Path folderToDeletePath(/*Project::getPath().toString() + "/" + */folderToDelete->getPath().toString());

	//delete files
	std::vector<ResourceFile>& files = folderToDelete->getFiles();
	const int filesSize = files.size();
	for (int i = 0; i < filesSize; i++)
	{
		deleteResourceFrom(files[0], *folderToDelete);
	}

	//delete sub folders
	std::vector<ResourceFolder>& subFolders = folderToDelete->getSubFolders();
	const int subFolderSize = subFolders.size();
	for (int i = 0; i < subFolderSize; i++)
	{
		deleteSubFolderFrom(subFolders[0].getName(), *folderToDelete);
	}

	//delete current directory
	folderFrom.removeSubFolder(folderName);
	FileHandler::removeDirectory(folderToDeletePath);
}

void ResourceTree::deleteResourceFrom(const ResourceFile& resourceFileToDelete, ResourceFolder& folderFrom)
{
	assert(folderFrom.hasFile(resourceFileToDelete.getKey()));
	if (!folderFrom.hasFile(resourceFileToDelete.getKey()))
		return;

	FileHandler::CompletePath fileToDeletePath(/*Project::getPath().toString() + "/" +*/ resourceFileToDelete.getPath().toString());

	folderFrom.removeFile(resourceFileToDelete.getKey());
	FileHandler::deleteFile(fileToDeletePath);
}

void ResourceTree::moveResourceTo(const ResourceFile& resourceFileToMove, ResourceFolder& folderFrom, ResourceFolder& folderTo)
{
	assert(folderFrom.hasFile(resourceFileToMove.getKey()));
	if (!folderFrom.hasFile(resourceFileToMove.getKey()))
		return;
	assert(!folderTo.hasFile(resourceFileToMove.getKey()));
	if (folderTo.hasFile(resourceFileToMove.getKey()))
		return;

	const FileHandler::CompletePath newResourcePath(folderTo.getPath(), resourceFileToMove.getPath().getFilenameWithExtention());

	//change paths to make them begin from root application folder instead of project folder
	const FileHandler::CompletePath from_(/*Project::getPath().toString() + "/" + */resourceFileToMove.getPath().toString());
	const FileHandler::Path to_(/*Project::getPath().toString() + "/" + */newResourcePath.getPath().toString());

	folderTo.moveFileFrom(resourceFileToMove.getPath(), newResourcePath, folderFrom);

	FileHandler::copyPastFile(from_, to_); //NOT_SAFE
	FileHandler::deleteFile(from_);
}

void ResourceTree::copyResourceTo(const ResourceFile& resourceFileToMove, ResourceFolder& folderFrom, ResourceFolder& folderTo)
{
	assert(folderFrom.hasFile(resourceFileToMove.getKey()));
	if (!folderFrom.hasFile(resourceFileToMove.getKey()))
		return;
	assert(!folderTo.hasFile(resourceFileToMove.getKey()));
	if (folderTo.hasFile(resourceFileToMove.getKey()))
		return;

	const FileHandler::CompletePath newResourcePath(folderTo.getPath(), resourceFileToMove.getPath().getFilenameWithExtention());

	//change paths to make them begin from root application folder instead of project folder
	const FileHandler::CompletePath from_(/*Project::getPath().toString() + "/" + */resourceFileToMove.getPath().toString());
	const FileHandler::Path to_(/*Project::getPath().toString() + "/" +*/ newResourcePath.getPath().toString());

	folderTo.addFile(newResourcePath);

	FileHandler::copyPastFile(from_, to_); //NOT_SAFE
}

void ResourceTree::addNewShaderProgramTo(const std::string& shaderProgramName, ResourceFolder& folderTo)
{
	assert(!folderTo.hasFile(ResourceFileKey(ResourceType::SHADER_PROGRAM, shaderProgramName)));
	if (folderTo.hasFile(ResourceFileKey(ResourceType::SHADER_PROGRAM, shaderProgramName)))
		return;

	//We create and save the new resource
	const FileHandler::CompletePath resourceCompletePath(folderTo.getPath().toString(), shaderProgramName, ".glProg");

	//create new instance
	const FileHandler::CompletePath resourceFilePath(/*Project::getPath().toString() + "/" + */folderTo.getPath().toString(), shaderProgramName, ".glProg");
	ShaderProgram* newShaderProgram = new ShaderProgram(resourceCompletePath, Rendering::MaterialType::DEFAULT);
	newShaderProgram->setPipelineType(Rendering::PipelineType::DEFERRED_PIPILINE);
	newShaderProgram->save(/*resourceFilePath*/);

	folderTo.addFile<ShaderProgram>(resourceCompletePath, newShaderProgram);
}

void ResourceTree::addNewMaterialTo(const std::string& newMaterialName, ResourceFile& shaderProgramFile, ResourceFolder& folderTo)
{
	assert(shaderProgramFile.getType() == ResourceType::SHADER_PROGRAM);

	assert(!folderTo.hasFile(ResourceFileKey(ResourceType::MATERIAL, newMaterialName)));
	if (folderTo.hasFile(ResourceFileKey(ResourceType::MATERIAL, newMaterialName)))
		return;

	//We create and save the new resource
	const FileHandler::CompletePath resourceCompletePath(folderTo.getPath().toString(), newMaterialName, ".mat");

	//create new instance
	const FileHandler::CompletePath resourceFilePath(/*Project::getPath().toString() + "/" + */folderTo.getPath().toString(), newMaterialName, ".mat");
	Material* newMaterial = new Material(*static_cast<ShaderProgram*>(shaderProgramFile.getPointedResource()), resourceCompletePath);//static_cast<ShaderProgram*>(shaderProgramFile.getPointedResource())->makeNewMaterialInstance(resourceCompletePath); //getProgramFactory().getDefault(materialModelName)->makeNewMaterialInstance(resourceCompletePath);
	newMaterial->save(/*resourceFilePath*/);

	folderTo.addFile<Material>(resourceCompletePath, newMaterial);
}

void ResourceTree::addNewCubeTextureTo(const std::string& textureName, ResourceFolder& folderTo)
{
	//We create and save the new resource
	const FileHandler::CompletePath resourceCompletePath(folderTo.getPath().toString(), textureName, ".ctx");

	//create new instance
	const FileHandler::CompletePath resourceFilePath(/*Project::getPath().toString() + "/" + */folderTo.getPath().toString(), textureName, ".ctx");
	CubeTexture* newCubeTexture = new CubeTexture(resourceCompletePath);
	newCubeTexture->save(/*resourceFilePath*/);

	folderTo.addFile<CubeTexture>(resourceCompletePath, newCubeTexture);
}

void ResourceTree::addNewAnimationStateMachineTo(const std::string & resourceName, ResourceFolder & folderTo)
{
	//We create and save the new resource
	const FileHandler::CompletePath resourceCompletePath(folderTo.getPath().toString(), resourceName, ".asm");

	//create new instance
	const FileHandler::CompletePath resourceFilePath(/*Project::getPath().toString() + "/" + */folderTo.getPath().toString(), resourceName, ".ctx");
	Sungine::Animation::AnimationStateMachine* newAnimationStateMachine = new Sungine::Animation::AnimationStateMachine(resourceCompletePath);
	newAnimationStateMachine->save(/*resourceFilePath*/);

	folderTo.addFile<Sungine::Animation::AnimationStateMachine>(resourceCompletePath, newAnimationStateMachine);
}

void ResourceTree::addSubFolderTo(const std::string& folderName, ResourceFolder& folderTo)
{
	assert(folderTo.getSubFolder(folderName) == nullptr);
	if (folderTo.getSubFolder(folderName) != nullptr)
		return;

	folderTo.addSubFolder(folderName);

	const FileHandler::Path folderParentPath(/*Project::getPath().toString() + "/" +*/ folderTo.getPath().toString() + "/");
	FileHandler::addDirectory(folderName, folderParentPath);
}

void ResourceTree::moveSubFolderTo(const std::string& folderName, ResourceFolder& folderFrom, ResourceFolder& folderTo)
{
	assert(folderTo.getSubFolder(folderName) == nullptr);
	if (folderTo.getSubFolder(folderName) != nullptr)
		return;
	assert(folderFrom.getSubFolder(folderName) != nullptr);
	if (folderFrom.getSubFolder(folderName) == nullptr)
		return;

	ResourceFolder* folderToMove = folderFrom.getSubFolder(folderName);

	//create new folder in destination folder
	addSubFolderTo(folderName, folderTo);
	ResourceFolder* targetedFolder = folderTo.getSubFolder(folderName);
	assert(targetedFolder != nullptr);
	if (targetedFolder == nullptr)
		return;

	//move files
	std::vector<ResourceFile>& files = folderToMove->getFiles();
	const int filesSize = files.size();
	for (int i = 0; i < filesSize; i++)
	{
		moveResourceTo(files[i], *folderToMove, *targetedFolder);
	}

	//move sub folders
	std::vector<ResourceFolder>& subFolders = folderToMove->getSubFolders();
	const int subFolderSize = subFolders.size();
	for (int i = 0; i < subFolderSize; i++)
	{
		moveSubFolderTo(subFolders[i].getName(), *folderToMove, *targetedFolder);
	}

	//delete the folder to move
	deleteSubFolderFrom(folderName, folderFrom);
}

void ResourceTree::copySubFolderTo(const std::string& folderName, ResourceFolder& folderFrom, ResourceFolder& folderTo)
{
	assert(folderTo.getSubFolder(folderName) == nullptr);
	if (folderTo.getSubFolder(folderName) != nullptr)
		return;
	assert(folderFrom.getSubFolder(folderName) != nullptr);
	if (folderFrom.getSubFolder(folderName) == nullptr)
		return;

	ResourceFolder* folderToMove = folderFrom.getSubFolder(folderName);

	//create new folder in destination folder
	addSubFolderTo(folderName, folderTo);
	ResourceFolder* targetedFolder = folderTo.getSubFolder(folderName);
	assert(targetedFolder != nullptr);
	if (targetedFolder == nullptr)
		return;

	//copy files
	std::vector<ResourceFile>& files = folderToMove->getFiles();
	const int filesSize = files.size();
	for (int i = 0; i < filesSize; i++)
	{
		copyResourceTo(files[i], *folderToMove, *targetedFolder);
	}

	//copy sub folders
	std::vector<ResourceFolder>& subFolders = folderToMove->getSubFolders();
	const int subFolderSize = subFolders.size();
	for (int i = 0; i < subFolderSize; i++)
	{
		copySubFolderTo(subFolders[i].getName(), *folderToMove, *targetedFolder);
	}
}

void ResourceTree::renameSubFolderIn(const std::string& folderName, const std::string& newFolderName, ResourceFolder& parentFolder)
{
	assert(FileHandler::isValidFileOrDirectoryName(newFolderName));
		if (!FileHandler::isValidFileOrDirectoryName(newFolderName))
			return;

	if (folderName == newFolderName)
		return;

	assert(parentFolder.getSubFolder(folderName) != nullptr);
	if (parentFolder.getSubFolder(folderName) == nullptr)
		return;

	ResourceFolder* folderToRename = parentFolder.getSubFolder(folderName);
	assert(folderToRename != nullptr);
		if (folderToRename == nullptr)
			return;

	FileHandler::Path folderPath(/*Project::getPath().toString() + "/" + */folderToRename->getPath().toString());

	folderToRename->rename(newFolderName);
	FileHandler::renameDirectory(folderPath, newFolderName);
}

void ResourceTree::renameResourceIn(ResourceFile& fileToRename, const std::string& newFileName, ResourceFolder& parentFolder)
{
	assert(FileHandler::isValidFileOrDirectoryName(newFileName));
	if (!FileHandler::isValidFileOrDirectoryName(newFileName))
		return;

	if (fileToRename.getName() == newFileName)
		return;

	assert(parentFolder.getFile(fileToRename.getKey()) != nullptr);
	if (parentFolder.getFile(fileToRename.getKey()) == nullptr)
		return;

	FileHandler::CompletePath filePath(/*Project::getPath().toString() + "/" +*/ fileToRename.getPath().toString());

	fileToRename.rename(newFileName);
	FileHandler::renameFile(filePath, newFileName);
}

void ResourceTree::addExternalResourceTo(const FileHandler::CompletePath& resourcePath, ResourceFolder& folderTo)
{
	assert(!folderTo.hasFile(ResourceFileKey(resourcePath)));
	if (folderTo.hasFile(ResourceFileKey(resourcePath)))
		return;

	FileHandler::CompletePath newfilePath(/*Project::getPath().toString() + "/" +*/ folderTo.getPath().toString(), resourcePath.getFilenameWithExtention());
	
	folderTo.addFile(resourcePath.getFilenameWithExtention());
	FileHandler::copyPastFile(resourcePath, newfilePath);
}

/////////// RESOURCE TREE VIEW /////////////

//
//void ResourceTreeWindow::save() const
//{
//	//save this window : 
//	Json::Value root;
//	
//	saveRecursivly(root);
//
//	std::ofstream streamResources;
//	streamResources.open(resourcesPath);
//	if (!streamResources.is_open())
//	{
//		std::cout << "error, can't save tree window at path : " << resourcesPath << std::endl;
//		return;
//	}
//	streamResources << root;
//}
//
//void ResourceTreeWindow::saveRecursivly(Json::Value& root, std::vector<const std::string&>& path) const
//{
//	for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//	{
//		path.push_back(itFolder->getName());
//		saveRecursivly(root, path);
//		path.pop_back();
//	}
//
//	for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
//	{
//		for (int i = 0; i < path.size(); i++)
//		{
//			root[itFile->getPath()]["path"][i] = toJsonValue<std::string>(path[i]);
//		}
//		root[itFile->getPath()]["name"] = toJsonValue<std::string>(path[i]);
//	}
//}
//
//void ResourceTreeWindow::load()
//{
//	//load this window : 
//	std::string resourcePath = m_path + "/resources.txt";
//
//	std::ifstream streamResources;
//	streamResources.open(resourcePath);
//	if (!streamResources.is_open())
//	{
//		std::cout << "error, can't load ResourceTree window at path : " << resourcePath << std::endl;
//		return;
//	}
//	Json::Value root;
//	streamResources >> root.get(;
//
//	loadRecursivly(root);
//}

ResourceTreeView::ResourceTreeView(const std::string& name, ResourceTree* model, Editor* editorPtr)
	: EditorFrame(name)
	, m_model(model)
	, m_folderWeRightClicOn(nullptr)
	, m_editorPtr(editorPtr)
{
	
	//std::vector<const std::string&> resourcePath;
	////Meshes :
	//for (auto& it = getMeshFactory().begin(); it != getMeshFactory().end(); it++)
	//{
	//	const std::string& resourceName = it->first;
	//	ResourceFile resource(resourceName, ResourceType::MESH);
	//	if (containsFileRecursivly(resourceName, &resourcePath))
	//	{

	//	}
	//}

	/*m_model->addSubFolder("toto");
	m_model->getSubFolder("toto").addFile(ResourceFile("FileToto01", ResourceType::MESH));
	m_model->getSubFolder("toto").addFile(ResourceFile("FileToto02", ResourceType::MESH));
	m_model->getSubFolder("toto").addFile(ResourceFile("FileToto03", ResourceType::MESH));

	m_model->addSubFolder("tutu");
	m_model->getSubFolder("tutu").addFile(ResourceFile("FileTutu01", ResourceType::MESH));
	m_model->getSubFolder("tutu").addFile(ResourceFile("FileTutu02", ResourceType::MESH));

	m_model->addFile(ResourceFile("File01", ResourceType::MESH));*/
}

ResourceTreeView::~ResourceTreeView()
{

}

void ResourceTreeView::displayFiles(ResourceFolder* parentFolder, ResourceFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback)
{

	for (int fileIdx = 0; fileIdx < currentFolder.fileCount(); fileIdx++)
	{
		ResourceFile& currentFile = currentFolder.getFile(fileIdx);

		//TODO : couleur à changer en fonction du type de resource.
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

		bool isItemHovered = false;
		if (!currentFile.isBeingRenamed())
		{
			
			bool isActive = (currentFolder.getPath() == m_model->getSelectedFileFolderPath() && m_model->getSelectedFileKey() == currentFile.getKey());
			if (ImGui::RadioButton(currentFile.getName().c_str(), isActive))
			{
				m_model->setSelectedFileFolderPath(currentFolder.getPath());
				m_model->setSelectedFileKey(currentFile.getKey());
				m_editorPtr->onResourceSelected();
			}
			isItemHovered = ImGui::IsItemHovered();
			ImGui::SameLine();
			currentFile.drawIconeInResourceTree();

			if (isItemHovered)
			{
				if (ImGui::IsMouseClicked(1))
				{
					if (outOpenModaleCallback != nullptr)
					{
						outOpenModaleCallback->shouldOpen = true;
						outOpenModaleCallback->modaleName = "resourceFileContextMenu";
					}
					m_fileWeRightClicOn = &currentFile;
					m_folderWeRightClicOn = &currentFolder;
				}
				else
				{
					currentFile.drawUIOnHovered();
				}
			}
		}
		else
		{
			if (currentFile.drawRenamingInputText())
				currentFile.endRenamingResource();
		}
		ImGui::PopStyleColor(4);

		//rename
		if (isItemHovered && ImGui::IsMouseDoubleClicked(0))
		{
			currentFile.begingRenamingResource();
		}

		//files drag and drop
		if (!currentFile.isBeingRenamed() && isItemHovered && ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
		{
			DragAndDropManager::beginDragAndDrop(std::make_shared<ResourceDragAndDropOperation>(&currentFile, &currentFolder));
		}
	}
}



void ResourceTreeView::displayFoldersRecusivly(ResourceFolder* parentFolder, ResourceFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback, DropCallback* outDropCallback)
{

	int colorStyleModifierCount = 0;
	if (DragAndDropManager::isDragAndDropping() && (DragAndDropManager::getOperationType() | (EditorDragAndDropType::ResourceDragAndDrop | EditorDragAndDropType::ResourceFolderDragAndDrop)))
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 255, 0, 255));
		colorStyleModifierCount++;
	}

	bool nodeOpened = ImGui::TreeNode(currentFolder.getName().c_str());
	auto rectMax = ImGui::GetItemRectMax();
	auto rectMin = ImGui::GetItemRectMin();
	if (nodeOpened)
	{

		//current folder drag and drop
		if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
		{
			DragAndDropManager::beginDragAndDrop(std::make_shared<ResourceFolderDragAndDropOperation>(&currentFolder, parentFolder, m_model));
		}

		//recursivity
		for (int subFolderIdx = 0; subFolderIdx < currentFolder.subFolderCount(); subFolderIdx++)
		{
			ResourceFolder* itFolder = currentFolder.getSubFolder(subFolderIdx);

			displayFoldersRecusivly(&currentFolder, *itFolder, outOpenModaleCallback, outDropCallback);

		}
		displayFiles(parentFolder, currentFolder, outOpenModaleCallback);

		ImGui::TreePop();
	}
	if (ImGui::IsMouseHoveringRect(rectMin, rectMax) && DragAndDropManager::isDragAndDropping())
	{
		ImGui::GetWindowDrawList()->AddRect(ImVec2(rectMin.x - 2, rectMin.y - 2), ImVec2(rectMax.x + 2, rectMax.y + 2), ImColor(255, 255, 0, 255), 5.f);
	}
	if (ImGui::IsMouseHoveringRect(rectMin, rectMax) && ImGui::IsMouseReleased(0))
	{
		if (outDropCallback != nullptr)
		{
			outDropCallback->currentFolder = &currentFolder;
			outDropCallback->dropContext = EditorDropContext::DropIntoFileOrFolder;
		}
		//DragAndDropManager::dropDraggedItem(&currentFolder, EditorDropContext::DropIntoFileOrFolder);
	}

	//right clic menu open ? 
	if (ImGui::IsMouseHoveringRect(rectMin, rectMax) && ImGui::IsMouseClicked(1))
	{
		if (outOpenModaleCallback != nullptr)
		{
			outOpenModaleCallback->shouldOpen = true;
			outOpenModaleCallback->modaleName = "resourceFolderContextMenu";
		}
		//ImGui::OpenPopupEx("resourceFolderContextMenu", true);
		m_folderWeRightClicOn = &currentFolder;
	}

	ImGui::PopStyleColor(colorStyleModifierCount);
}

void ResourceTreeView::displayModales()
{
	std::string popupToOpen = "";
	bool needOpenPopup = false;

	//right clic on file menu display :
	if (ImGui::BeginPopup("resourceFileContextMenu"))
	{
		if (ImGui::Button("Rename file."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("RenameFileModale");
			popupToOpen = "RenameFileModale";
			needOpenPopup = true;
		}
		else if (ImGui::Button("Copy file."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr && m_fileWeRightClicOn != nullptr)
			{
				m_fileWaitingPastPath = m_fileWeRightClicOn->getPath();
				m_shouldMoveFileOrFolder = false;
				m_isMovingItemFolder = false;
			}
		}
		else if (ImGui::Button("Move file."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr && m_fileWeRightClicOn != nullptr)
			{
				m_fileWaitingPastPath = m_fileWeRightClicOn->getPath();
				m_shouldMoveFileOrFolder = true;
				m_isMovingItemFolder = false;
			}
		}
		else if (ImGui::Button("Delete file."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr && m_fileWeRightClicOn != nullptr)
				ResourceTree::deleteResourceFrom(*m_fileWeRightClicOn, *m_folderWeRightClicOn);
		}

		// Show per resource right click mouse UI
		if (m_fileWeRightClicOn != nullptr)
		{
			needOpenPopup = m_fileWeRightClicOn->getPointedResource()->drawRightClicContextMenu(popupToOpen);
		}

		ImGui::EndPopup();
	}

	if (needOpenPopup)
	{
		ImGui::Ext::openStackingPopUp(popupToOpen.c_str());
		needOpenPopup = false;
	}

	//right clic on folder menu display :
	if (ImGui::BeginPopup("resourceFolderContextMenu"))
	{
		if (ImGui::Button("Rename folder."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("RenameFolderModale");
			needOpenPopup = true;
			popupToOpen = "RenameFolderModale";
		}
		else if (ImGui::Button("Add folder."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("AddFolderModale");
			needOpenPopup = true;
			popupToOpen = "AddFolderModale";
		}
		else if (ImGui::Button("Copy folder."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr)
			{
				m_folderWaitingPastPath = m_folderWeRightClicOn->getPath();
				m_shouldMoveFileOrFolder = false;
				m_isMovingItemFolder = true;
			}
		}
		else if (ImGui::Button("Move folder."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr)
			{
				m_folderWaitingPastPath = m_folderWeRightClicOn->getPath();
				m_shouldMoveFileOrFolder = true;
				m_isMovingItemFolder = true;
			}
		}
		else if (m_isMovingItemFolder == true && m_folderWeRightClicOn != nullptr && ImGui::Button("Past folder."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr)
			{
				ResourceFolder* folderToMoveOrCopy = m_model->getSubFolder(m_folderWaitingPastPath);
				ResourceFolder* parentFolder = m_model->getSubFolder(folderToMoveOrCopy->getParentFolderPath());
				assert(parentFolder != nullptr);

				if (m_shouldMoveFileOrFolder)
					ResourceTree::moveSubFolderTo(folderToMoveOrCopy->getName(), *parentFolder, *m_folderWeRightClicOn);
				else
					ResourceTree::copySubFolderTo(folderToMoveOrCopy->getName(), *parentFolder, *m_folderWeRightClicOn);
			}
		}
		else if (m_isMovingItemFolder == false && m_fileWeRightClicOn != nullptr && ImGui::Button("Past file."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr && m_fileWeRightClicOn != nullptr)
			{
				ResourceFile* fileToMoveOrCopy = m_model->getFile(m_fileWaitingPastPath);
				ResourceFolder* parentFolder = m_model->getSubFolder(fileToMoveOrCopy->getParentFolderPath());
				assert(parentFolder != nullptr);

				if (m_shouldMoveFileOrFolder)
					ResourceTree::moveResourceTo(*fileToMoveOrCopy, *parentFolder, *m_folderWeRightClicOn);
				else
					ResourceTree::copyResourceTo(*fileToMoveOrCopy, *parentFolder, *m_folderWeRightClicOn);
			}
		}
		else if (ImGui::Button("Delete folder."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr)
			{
				ResourceFolder* parentFolder = m_model->getSubFolder(m_folderWeRightClicOn->getParentFolderPath());
				assert(parentFolder != nullptr);

				ResourceTree::deleteSubFolderFrom(m_folderWeRightClicOn->getName(), *parentFolder);
			}
		}
		else if (ImGui::Button("Add resource."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("AddResourcePopUp");
			needOpenPopup = true;
			popupToOpen = "AddResourcePopUp";
		}

		ImGui::EndPopup();
	}

	if (needOpenPopup)
	{
		ImGui::Ext::openStackingPopUp(popupToOpen.c_str());
		needOpenPopup = false;
	}

	//pop up to rename file :
	if (ImGui::BeginPopup("RenameFileModale"))
	{
		popUpToRenameFile();
	}

	//pop up to rename folder :
	if (ImGui::BeginPopup("RenameFolderModale"))
	{
		popUpToRenameFolder();
	}

	//pop up to add resource :
	if (ImGui::BeginPopup("AddResourcePopUp"))
	{
		if (ImGui::Button("ShaderProgram."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("AddShaderProgramPopUp");
			needOpenPopup = true;
			popupToOpen = "AddShaderProgramPopUp";
		}
		else if (ImGui::Button("CubeTexture."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("AddCubeTexturePopUp");
			needOpenPopup = true;
			popupToOpen = "AddCubeTexturePopUp";
		}
		else if (ImGui::Button("CubeTexture."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("AddCubeTexturePopUp");
			needOpenPopup = true;
			popupToOpen = "AddAnimationStateMachinePopUp";
		}

		ImGui::EndPopup();
	}

	if (needOpenPopup)
	{
		ImGui::Ext::openStackingPopUp(popupToOpen.c_str());
		needOpenPopup = false;
	}

	//PopUp to choose a material :
	if (ImGui::BeginPopup("AddMaterialPopUp"))
	{
		popUpToAddMaterial();
	}

	if (ImGui::BeginPopup("AddShaderProgramPopUp"))
	{
		popUpToAddShaderProgram();
	}

	//PopUp to add new cubeTexture :
	if (ImGui::BeginPopup("AddCubeTexturePopUp"))
	{
		popUpToAddCubeTexture();
	}

	if (ImGui::BeginPopup("AddAnimationStateMachinePopUp"))
	{
		popUpToAddAnimationStateMachine();
	}

	//Modale to add new folder :
	if (ImGui::BeginPopup("AddFolderModale"))
	{
		popUpToAddFolder();
	}
}

void ResourceTreeView::popUpToRenameFile()
{
	m_uiString.resize(100);
	ImGui::InputText("##newName", &m_uiString[0], 100);
	m_uiString = m_uiString.substr(0, m_uiString.find_first_of('\0'));
	assert(m_fileWeRightClicOn != nullptr);

	ResourceFolder* parentFolder = m_model->getSubFolder(m_fileWeRightClicOn->getParentFolderPath());
	assert(parentFolder != nullptr);

	if (!parentFolder->hasFile(ResourceFileKey(m_fileWeRightClicOn->getType(), m_uiString)))
	{
		if (FileHandler::isValidFileOrDirectoryName(m_uiString))
		{
			ImGui::SameLine();
			if (ImGui::Button("Validate##validateName") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
			{
				ResourceTree::renameResourceIn(*m_fileWeRightClicOn, m_uiString, *parentFolder);

				m_fileWeRightClicOn = nullptr;
				ImGui::CloseCurrentPopup();
			}
		}
		else
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Invalid file name.");
	}
	else
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "A file with the same name already exists in this folder.");
	}
	ImGui::EndPopup();
}

void ResourceTreeView::popUpToRenameFolder()
{
	m_uiString.resize(100);
	ImGui::InputText("##newName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);

	ResourceFolder* parentFolder = m_model->getSubFolder(m_folderWeRightClicOn->getParentFolderPath());
	assert(parentFolder != nullptr);

	if (!parentFolder->hasSubFolder(m_uiString))
	{
		if (FileHandler::isValidFileOrDirectoryName(m_uiString))
		{
			ImGui::SameLine();
			if (ImGui::Button("Validate##validateName") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
			{
				ResourceTree::renameSubFolderIn(m_folderWeRightClicOn->getName(), m_uiString, *parentFolder);

				m_folderWeRightClicOn = nullptr;
				ImGui::CloseCurrentPopup();
			}
		}
		else
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Invalid folder name.");
	}
	else
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "A folder with the same name already exists in this folder.");
	}
	ImGui::EndPopup();
}

void ResourceTreeView::popUpToAddFolder()
{
	m_uiString.resize(100);
	ImGui::InputText("##folderName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasSubFolder(m_uiString))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddFolder") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
				ResourceTree::addSubFolderTo(m_uiString, *m_folderWeRightClicOn);
			m_folderWeRightClicOn = nullptr;
			ImGui::CloseCurrentPopup();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "A folder with the same name already exists.");
	}
	ImGui::EndPopup();
}

void ResourceTreeView::popUpToChooseMaterial()
{
	bool shouldEndPopup = true;
	int tmpProgramIdx = 0;
	/*
	for (auto& it = getProgramFactory().resourceBegin(); it != getProgramFactory().resourceEnd(); it++)
	{
		const std::string matName = it->first.getFilename();
		const std::string btnLabel = matName + "##" +std::to_string(tmpProgramIdx++);

		if (ImGui::Button(btnLabel.data()))
		{
			m_chooseMaterialName = matName;
			ImGui::EndPopup();
			ImGui::OpenPopupEx("AddMaterialPopUp", true);
			shouldEndPopup = false;
		}
		//else
		//	ImGui::EndPopup();
	}

	for (auto& it = getProgramFactory().defaultResourceBegin(); it != getProgramFactory().defaultResourceEnd(); it++)
	{
		const std::string matName = it->first;
		const std::string btnLabel = matName + "##" + std::to_string(tmpProgramIdx++);

		if (ImGui::Button(btnLabel.data()))
		{
			m_chooseMaterialName = matName;
			ImGui::EndPopup();
			ImGui::OpenPopupEx("AddMaterialPopUp", true);
			shouldEndPopup = false;
		}
		//else
		//	ImGui::EndPopup();
	}*/


	for (auto& it = getProgramFactory().resourceBegin(); it != getProgramFactory().resourceEnd(); it++)
	{
		const std::string matName = it->first.getFilename();
		const std::string btnLabel = matName + "##" + std::to_string(tmpProgramIdx++);

		if (ImGui::Button(btnLabel.data()))
		{
			//m_chooseMaterialName = matName;
			ImGui::EndPopup();
			ImGui::Ext::openStackingPopUp("AddShaderProgramPopUp");
			shouldEndPopup = false;
		}
		//else
		//	ImGui::EndPopup();
	}

	if(shouldEndPopup)
		ImGui::EndPopup();
}

void ResourceTreeView::popUpToAddShaderProgram()
{
	//assert(!m_chooseShderProgram.empty());

	m_uiString.resize(100);
	ImGui::InputText("##shaderProgramName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(ResourceFileKey(ResourceType::MATERIAL, m_uiString)))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddShaderProgram") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				ResourceTree::addNewShaderProgramTo(m_uiString, *m_folderWeRightClicOn);
			}
			m_folderWeRightClicOn = nullptr;
			ImGui::CloseCurrentPopup();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "A file with the same name already exists.");
	}
	ImGui::EndPopup();
}

void ResourceTreeView::popUpToAddMaterial()
{
	//assert(!m_chooseShderProgram.empty());

	m_uiString.resize(100);
	ImGui::InputText("##materialName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(ResourceFileKey(ResourceType::MATERIAL, m_uiString)))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddMaterial") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				ResourceTree::addNewMaterialTo(m_uiString, *m_fileWeRightClicOn, *m_folderWeRightClicOn);
			}
			m_folderWeRightClicOn = nullptr;
			ImGui::CloseCurrentPopup();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "A file with the same name already exists.");
	}
	ImGui::EndPopup();
}

void ResourceTreeView::popUpToAddCubeTexture()
{
	m_uiString.resize(100);
	ImGui::InputText("##fileName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(ResourceFileKey(ResourceType::CUBE_TEXTURE, m_uiString)))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddFile") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				ResourceTree::addNewCubeTextureTo(m_uiString, *m_folderWeRightClicOn);
			}
			m_folderWeRightClicOn = nullptr;
			ImGui::CloseCurrentPopup();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "A file with the same name already exists.");
	}
	ImGui::EndPopup();
}

void ResourceTreeView::popUpToAddAnimationStateMachine()
{
	m_uiString.resize(100);
	ImGui::InputText("##fileName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(ResourceFileKey(ResourceType::ANIMATION_STATE_MACHINE, m_uiString)))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddFile") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				ResourceTree::addNewAnimationStateMachineTo(m_uiString, *m_folderWeRightClicOn);
			}
			m_folderWeRightClicOn = nullptr;
			ImGui::CloseCurrentPopup();
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(255, 0, 0, 255), "A file with the same name already exists.");
	}
	ImGui::EndPopup();
}

void ResourceTreeView::drawContent(Project& project, EditorModal* parentModale)
{
	DropCallback dropCallback(nullptr, EditorDropContext::DropIntoFileOrFolder);
	OpenModaleCallback openModaleCallback;
	displayFoldersRecusivly(nullptr, *m_model, &openModaleCallback, &dropCallback);

	openModaleCallback.openCallbackIfNeeded();
	displayModales();

	//asynchronous drop
	if (dropCallback.currentFolder != nullptr)
	{
		DragAndDropManager::dropDraggedItem(dropCallback.currentFolder, EditorDropContext::DropIntoFileOrFolder);
	}
	//displayFoldersRecusivly(m_model, m_model->getSubFolders(), m_model->getFiles());
}

void ResourceTreeView::setModel(ResourceTree* model)
{
	m_model = model;
}

/*
void ResourceTreeView::displayFoldersRecusivly(ResourceFolder* parentFolder, std::vector<ResourceFolder>& foldersToDisplay, std::vector<ResourceFile>& filesToDisplay)
{

int colorStyleModifierCount = 0;
if (DragAndDropManager::isDragAndDropping() && (DragAndDropManager::getOperationType() | (EditorDragAndDropType::ResourceDragAndDrop | EditorDragAndDropType::ResourceFolderDragAndDrop)))
{
ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 255, 0, 255));
colorStyleModifierCount++;
}

for (int subFolderIdx = 0; subFolderIdx < foldersToDisplay.size(); subFolderIdx++)
{
ResourceFolder& itFolder = foldersToDisplay[subFolderIdx];

bool nodeOpened = ImGui::TreeNode(itFolder.getName().c_str());
auto rectMax = ImGui::GetItemRectMax();
auto rectMin = ImGui::GetItemRectMin();
if (nodeOpened)
{
//current folder drag and drop
if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
{
DragAndDropManager::beginDragAndDrop(std::make_shared<ResourceFolderDragAndDropOperation>(&itFolder, parentFolder, m_model));
}

displayFoldersRecusivly(&itFolder, itFolder.getSubFolders(), itFolder.getFiles());

ImGui::TreePop();
}
if (ImGui::IsMouseHoveringBox(rectMin, rectMax) && DragAndDropManager::isDragAndDropping())
{
ImGui::GetWindowDrawList()->AddRect(ImVec2(rectMin.x - 2, rectMin.y - 2), ImVec2(rectMax.x + 2, rectMax.y + 2), ImColor(255, 255, 0, 255), 5.f);
}
if (ImGui::IsMouseHoveringBox(rectMin, rectMax) && ImGui::IsMouseReleased(0))
{
DragAndDropManager::dropDraggedItem(&itFolder, EditorDropContext::DropIntoFileOrFolder);
}
//right clic menu open ?
bool shouldOpenPopup = false;
if (ImGui::IsMouseHoveringBox(rectMin, rectMax) && ImGui::IsMouseClicked(1))
{
ImGui::OpenPopupEx("resourceFolderContextMenu", false);
m_folderWeRightClicOn = &itFolder;
}
}

//right clic menu display :
if (ImGui::BeginPopup("resourceFolderContextMenu"))
{
if (ImGui::Button("Add folder."))
{
ImGui::EndPopup();
ImGui::OpenPopupEx("AddFolderModale", true);
}
else if (ImGui::Button("Add resource."))
{
ImGui::EndPopup();
ImGui::OpenPopupEx("AddResourcePopUp", true);
}
else
ImGui::EndPopup();
}

//pop up to add resource :
if (ImGui::BeginPopup("AddResourcePopUp"))
{
if (ImGui::Button("Material."))
{
ImGui::EndPopup();
ImGui::OpenPopupEx("ChooseMaterialPopUp", true);
}
else if (ImGui::Button("CubeTexture."))
{
ImGui::EndPopup();
ImGui::OpenPopupEx("AddCubeTexturePopUp", true);
}
else
ImGui::EndPopup();
}

//PopUp to choose a material :
if (ImGui::BeginPopup("ChooseMaterialPopUp"))
{
popUpToChooseMaterial();
}

//PopUp to add new cubeTexture :
if (ImGui::BeginPopup("AddCubeTexturePopUp"))
{
popUpToAddCubeTexture();
}

//Modale to add new folder :
if (ImGui::BeginPopup("AddFolderModale"))
{
m_uiString.resize(100);
ImGui::InputText("##folderName", &m_uiString[0], 100);
assert(m_folderWeRightClicOn != nullptr);
if (!m_folderWeRightClicOn->hasSubFolder(m_uiString))
{
ImGui::SameLine();
if (ImGui::Button("Validate##AddFolder") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
{
if(m_folderWeRightClicOn != nullptr)
m_folderWeRightClicOn->addSubFolder(m_uiString);
m_folderWeRightClicOn = nullptr;
ImGui::CloseCurrentPopup();
}
}
else
{
ImGui::TextColored(ImVec4(255, 0, 0, 255), "A folder with the same name already exists.");
}
ImGui::EndPopup();
}


for (int fileIdx = 0; fileIdx < filesToDisplay.size(); fileIdx++)
{
ResourceFile& currentFile = filesToDisplay[fileIdx];

//TODO : couleur à changer en fonction du type de resource.
ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
if(!currentFile.isBeingRenamed())
ImGui::Button(currentFile.getName().c_str());
else
{
if (currentFile.drawRenamingInputText())
currentFile.endRenamingResource();
}
ImGui::PopStyleColor(4);

//rename
if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
{
currentFile.begingRenamingResource();
}

//files drag and drop
if (!currentFile.isBeingRenamed() && ImGui::IsItemHovered() && ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
{
DragAndDropManager::beginDragAndDrop(std::make_shared<ResourceDragAndDropOperation>(&currentFile, parentFolder));
}
}

ImGui::PopStyleColor(colorStyleModifierCount);
}
*/

//
//void ResourceTreeWindow::addFolder(const std::string& folderName)
//{
//	m_resourceFolders.push_back(ResourceFolder(folderName));
//}
//
//void ResourceTreeWindow::removeFolder(const std::string& folderName)
//{
//	for (auto& it = m_resourceFolders.begin(); it != m_resourceFolders.end(); it++)
//	{
//		if (it->getName() == folderName)
//		{
//			m_resourceFolders.erase(it);
//			return;
//		}
//	}
//}
//
//ResourceFolder& ResourceTreeWindow::getFolder(const std::string& folderName)
//{
//	for (auto& folder : m_resourceFolders)
//	{
//		if (folder.getName() == folderName)
//			return folder;
//	}
//}
//
//void ResourceTreeWindow::addFileToFolder(ResourceFile file, const std::string& folderName)
//{
//	for (auto& folder : m_resourceFolders)
//	{
//		if (folder.getName() == folderName)
//			folder.addFile(file);
//	}
//}
//
//void ResourceTreeWindow::addFileToFolder(ResourceFile file, size_t folderIdx)
//{
//	assert(folderIdx >= 0 && folderIdx < m_resourceFolders.size());
//
//	m_resourceFolders[folderIdx].addFile(file);
//}

//TODO

void ResourceFile::drawInInspector(Scene & scene, const std::vector<IDrawableInInspector*>& selection)
{
	assert(false && "multiple edition of resource file isn't implemented.");
}

void ResourceFile::drawInInspector(Scene & scene)
{
	assert(m_pointedResource != nullptr);
	m_pointedResource->drawInInspector(scene);
}

void ResourceFile::drawIconeInResourceTree()
{
	if (m_pointedResource != nullptr)
		m_pointedResource->drawIconeInResourceTree();
}

void ResourceFile::drawUIOnHovered()
{
	if (m_pointedResource != nullptr)
		m_pointedResource->drawUIOnHovered();
}
