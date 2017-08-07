

#include  <fstream>

#include "AssetTree.h"
#include "imgui/imgui.h"
#include "imgui_extension.h"
#include "EditorGUI.h"
#include "project.h"
#include "Editor.h"

#include "Material.h"
#include "MaterialInstance.h"
#include "AnimationStateMachine.h"


/////////// RESSOURCE FOLDER /////////////////
//
//void AssetFolder::fillDatasFromExplorerFolder(const FileHandler::Path& folderPath)
//{
//	std::vector<std::string> dirNames;
//	FileHandler::getAllDirNames(folderPath, dirNames);
//
//	for (auto& dirName : dirNames)
//	{
//		int subFolderIdx = 0;
//		if (addSubFolder(dirName, &subFolderIdx))
//		{
//			m_subFoldersContainer[subFolderIdx].fillDatasFromExplorerFolder(FileHandler::Path(folderPath, dirName));
//		}
//	}
//
//	std::vector<std::string> fileNames;
//	FileHandler::getAllFileNames(folderPath, fileNames);
//	std::string outExtention;
//
//	for (auto& fileNameAndExtention : fileNames)
//	{
//		//We only add files that engine understand
//		FileHandler::getExtentionFromExtendedFilename(fileNameAndExtention, outExtention);
//		if (FileHandler::getFileTypeFromExtention(outExtention) != FileHandler::FileType::NONE)
//		{
//			addFile(fileNameAndExtention);
//		}
//	}
//}

bool AssetFolder::moveSubFolderToNewLocation(const std::string& subFolderName, AssetFolder& newLocation)
{
	AssetFolder* subFolder = getSubFolder(subFolderName);

	if (subFolder != nullptr)
	{
		subFolder->moveTo(newLocation);
		removeSubFolder(subFolderName);
	}

	return true;
}

bool AssetFolder::copySubFolderToNewLocation(const std::string& subFolderName, AssetFolder& newLocation)
{
	AssetFolder* subFolder = getSubFolder(subFolderName);

	if (subFolder != nullptr)
	{
		subFolder->copyTo(newLocation);
	}

	return true;
}

bool AssetFolder::moveTo(AssetFolder& newLocation)
{
	copyTo(newLocation); //it's like a copy / paste, ...

	// ... but we delete all files and folder from the old folder (we delete it because it's a "move to")
	removeAllFiles();
	removeAllSubFolders();

	return true;
}

bool AssetFolder::copyTo(AssetFolder& newLocation)
{
	int newLocationIdx = 0;
	//add a new empty sub folder
	newLocation.addSubFolder(m_name, &newLocationIdx);
	AssetFolder* newFolder = newLocation.getSubFolder(newLocationIdx);

	//transfers folder's files in the new empty folder
	for (auto& file : getFiles())
	{
		newFolder->addFile(file.getPath(), file.getAssetId());
	}

	//transfers sub folders (use same finction recursivly)
	for (auto& subFolder : getSubFolders())
	{
		subFolder.moveTo(*newLocation.getSubFolder(newLocationIdx));
	}

	return true;
}

/////////// RESSOURCE TREE /////////////////

AssetTree::AssetTree(const FileHandler::Path& assetResourcePath)
	: m_assetFolder(Project::getAssetsFolderPath())
	, m_defaultAssetFolder(Project::getDefaultAssetsFolderPath())
{
	//fillDatasFromExplorerFolder(assetResourcePath);
	//resolvePointersLoadingInFactories();
}

void AssetTree::selectAsset(const FileHandler::Path& assetPath, const std::string& assetFilenameWithExtention)
{
	m_selectedFileFolderPath = assetPath;
	m_selectedFileNameWithExtention = assetFilenameWithExtention;
}

AssetFile* AssetTree::getSelectedAssetFile()
{
	if (Project::isPathPointingInsideDefaultAssetFolder(m_selectedFileFolderPath))
	{
		return m_defaultAssetFolder.getFile(FileHandler::CompletePath(m_selectedFileFolderPath, m_selectedFileNameWithExtention));
	}
	else
	{
		return m_assetFolder.getFile(FileHandler::CompletePath(m_selectedFileFolderPath, m_selectedFileNameWithExtention));
	}
}

const FileHandler::Path & AssetTree::getSelectedFileFolderPath() const
{
	return m_selectedFileFolderPath;
}

const std::string& AssetTree::getSelectedFileNameWithExtention() const
{
	return m_selectedFileNameWithExtention;
}

AssetFolder& AssetTree::getAssetFolder()
{
	return m_assetFolder;
}

AssetFolder& AssetTree::getDefaultAssetFolder()
{
	return m_defaultAssetFolder;
}

void AssetTree::deleteSubFolderFrom(const std::string& folderName, AssetFolder& folderFrom)
{
	assert(folderFrom.getSubFolder(folderName) != nullptr);
	if (folderFrom.getSubFolder(folderName) == nullptr)
		return;

	AssetFolder* folderToDelete = folderFrom.getSubFolder(folderName);
	FileHandler::Path folderToDeletePath(/*Project::getPath().toString() + "/" + */folderToDelete->getPath().toString());

	//delete files
	std::vector<AssetFile>& files = folderToDelete->getFiles();
	const int filesSize = files.size();
	for (int i = 0; i < filesSize; i++)
	{
		deleteAssetFrom(files[0], *folderToDelete);
	}

	//delete sub folders
	std::vector<AssetFolder>& subFolders = folderToDelete->getSubFolders();
	const int subFolderSize = subFolders.size();
	for (int i = 0; i < subFolderSize; i++)
	{
		deleteSubFolderFrom(subFolders[0].getName(), *folderToDelete);
	}

	//delete current directory
	folderFrom.removeSubFolder(folderName);
	FileHandler::removeDirectory(folderToDeletePath);
}

void AssetTree::deleteAssetFrom(const AssetFile& assetFileToDelete, AssetFolder& folderFrom)
{
	assert(folderFrom.hasFile(assetFileToDelete.getFilenameWithExtention()));
	if (!folderFrom.hasFile(assetFileToDelete.getFilenameWithExtention()))
		return;

	FileHandler::CompletePath fileToDeletePath(assetFileToDelete.getPath().toString());

	folderFrom.removeFile(assetFileToDelete);
	FileHandler::deleteFile(fileToDeletePath);
}

void AssetTree::moveAssetTo(const AssetFile& assetFileToMove, AssetFolder& folderFrom, AssetFolder& folderTo)
{
	assert(folderFrom.hasFile(assetFileToMove.getFilenameWithExtention()));
	if (!folderFrom.hasFile(assetFileToMove.getFilenameWithExtention()))
		return;
	assert(!folderTo.hasFile(assetFileToMove.getFilenameWithExtention()));
	if (folderTo.hasFile(assetFileToMove.getFilenameWithExtention()))
		return;

	const FileHandler::CompletePath newResourcePath(folderTo.getPath(), assetFileToMove.getPath().getFilenameWithExtention());

	//change paths to make them begin from root application folder instead of project folder
	const FileHandler::CompletePath from_(assetFileToMove.getPath().toString());
	const FileHandler::Path to_(newResourcePath.getPath().toString());

	folderTo.moveFileFrom(folderFrom, assetFileToMove);

	FileHandler::copyPastFile(from_, to_); //NOT_SAFE
	FileHandler::deleteFile(from_);
}

void AssetTree::copyAssetTo(const AssetFile& assetFileToMove, AssetFolder& folderFrom, AssetFolder& folderTo)
{
	assert(folderFrom.hasFile(assetFileToMove.getFilenameWithExtention()));
	if (!folderFrom.hasFile(assetFileToMove.getFilenameWithExtention()))
		return;
	assert(!folderTo.hasFile(assetFileToMove.getFilenameWithExtention()));
	if (folderTo.hasFile(assetFileToMove.getFilenameWithExtention()))
		return;

	const FileHandler::CompletePath newAssetPath(folderTo.getPath(), assetFileToMove.getPath().getFilenameWithExtention());

	//change paths to make them begin from root application folder instead of project folder
	const FileHandler::CompletePath from_(assetFileToMove.getPath().toString());
	const FileHandler::Path to_(newAssetPath.getPath().toString());

	folderTo.addFile(newAssetPath, assetFileToMove.getAssetId());

	FileHandler::copyPastFile(from_, to_); //NOT_SAFE
}

void AssetTree::addNewMaterialTo(const std::string& materialName, AssetFolder& folderTo)
{
	assert(!folderTo.hasFile(materialName + Material::s_extention));
	if (folderTo.hasFile(materialName + Material::s_extention))
		return;

	//create new asset
	const FileHandler::CompletePath assetCompletePath(folderTo.getPath().toString(), materialName, Material::s_extention);
	Material* newMaterial = new Material(assetCompletePath, Rendering::MaterialType::DEFAULT);
	newMaterial->setPipelineType(Rendering::PipelineType::DEFERRED_PIPILINE);
	
	AssetHandle<Material> assetHandle = AssetManager::instance().createAsset<Material>(*newMaterial, assetCompletePath);
	if (assetHandle.isValid())
	{
		folderTo.addFile(assetCompletePath, newMaterial->getAssetId());
	}
}

void AssetTree::addNewMaterialInstanceTo(const std::string& newMaterialInstanceName, AssetFile& baseMaterialFile, AssetFolder& folderTo)
{
	assert(baseMaterialFile.getType() == FileHandler::FileType::MATERIAL);

	assert(!folderTo.hasFile(newMaterialInstanceName + MaterialInstance::s_extention));
	if (folderTo.hasFile(newMaterialInstanceName + MaterialInstance::s_extention))
		return;

	//create new asset
	const FileHandler::CompletePath assetCompletePath(folderTo.getPath().toString(), newMaterialInstanceName, MaterialInstance::s_extention);
	MaterialInstance* newMaterialInstance = new MaterialInstance(AssetManager::instance().getHandle<MaterialInstance>(*baseMaterialFile.getPointedAsset()), assetCompletePath);
	
	AssetHandle<MaterialInstance> assetHandle = AssetManager::instance().createAsset<MaterialInstance>(*newMaterialInstance, assetCompletePath);
	if (assetHandle.isValid())
	{
		folderTo.addFile(assetCompletePath, newMaterialInstance->getAssetId());
	}
}

void AssetTree::addNewCubeTextureTo(const std::string& textureName, AssetFolder& folderTo)
{
	//create new asset
	const FileHandler::CompletePath assetCompletePath(folderTo.getPath().toString(), textureName, CubeTexture::s_extention);
	CubeTexture* newCubeTexture = new CubeTexture(assetCompletePath);
	
	AssetHandle<CubeTexture> assetHandle = AssetManager::instance().createAsset<CubeTexture>(*newCubeTexture, assetCompletePath);
	if (assetHandle.isValid())
	{
		folderTo.addFile(assetCompletePath, newCubeTexture->getAssetId());
	}
}

void AssetTree::addNewAnimationStateMachineTo(const std::string & resourceName, AssetFolder & folderTo)
{
	//create new asset
	const FileHandler::CompletePath assetCompletePath(folderTo.getPath().toString(), resourceName, Sungine::Animation::AnimationStateMachine::s_extention);
	Sungine::Animation::AnimationStateMachine* newAnimationStateMachine = new Sungine::Animation::AnimationStateMachine(assetCompletePath);
	
	AssetHandle<Sungine::Animation::AnimationStateMachine> assetHandle = AssetManager::instance().createAsset<Sungine::Animation::AnimationStateMachine>(*newAnimationStateMachine, assetCompletePath);
	if (assetHandle.isValid())
	{
		folderTo.addFile(assetCompletePath, newAnimationStateMachine->getAssetId());
	}
}

void AssetTree::addSubFolderTo(const std::string& folderName, AssetFolder& folderTo)
{
	assert(folderTo.getSubFolder(folderName) == nullptr);
	if (folderTo.getSubFolder(folderName) != nullptr)
		return;

	folderTo.addSubFolder(folderName);

	const FileHandler::Path folderParentPath(/*Project::getPath().toString() + "/" +*/ folderTo.getPath().toString() + "/");
	FileHandler::addDirectory(folderName, folderParentPath);
}

void AssetTree::moveSubFolderTo(const std::string& folderName, AssetFolder& folderFrom, AssetFolder& folderTo)
{
	assert(folderTo.getSubFolder(folderName) == nullptr);
	if (folderTo.getSubFolder(folderName) != nullptr)
		return;
	assert(folderFrom.getSubFolder(folderName) != nullptr);
	if (folderFrom.getSubFolder(folderName) == nullptr)
		return;

	AssetFolder* folderToMove = folderFrom.getSubFolder(folderName);

	//create new folder in destination folder
	addSubFolderTo(folderName, folderTo);
	AssetFolder* targetedFolder = folderTo.getSubFolder(folderName);
	assert(targetedFolder != nullptr);
	if (targetedFolder == nullptr)
		return;

	//move files
	std::vector<AssetFile>& files = folderToMove->getFiles();
	const int filesSize = files.size();
	for (int i = 0; i < filesSize; i++)
	{
		moveAssetTo(files[i], *folderToMove, *targetedFolder);
	}

	//move sub folders
	std::vector<AssetFolder>& subFolders = folderToMove->getSubFolders();
	const int subFolderSize = subFolders.size();
	for (int i = 0; i < subFolderSize; i++)
	{
		moveSubFolderTo(subFolders[i].getName(), *folderToMove, *targetedFolder);
	}

	//delete the folder to move
	deleteSubFolderFrom(folderName, folderFrom);
}

void AssetTree::copySubFolderTo(const std::string& folderName, AssetFolder& folderFrom, AssetFolder& folderTo)
{
	assert(folderTo.getSubFolder(folderName) == nullptr);
	if (folderTo.getSubFolder(folderName) != nullptr)
		return;
	assert(folderFrom.getSubFolder(folderName) != nullptr);
	if (folderFrom.getSubFolder(folderName) == nullptr)
		return;

	AssetFolder* folderToMove = folderFrom.getSubFolder(folderName);

	//create new folder in destination folder
	addSubFolderTo(folderName, folderTo);
	AssetFolder* targetedFolder = folderTo.getSubFolder(folderName);
	assert(targetedFolder != nullptr);
	if (targetedFolder == nullptr)
		return;

	//copy files
	std::vector<AssetFile>& files = folderToMove->getFiles();
	const int filesSize = files.size();
	for (int i = 0; i < filesSize; i++)
	{
		copyAssetTo(files[i], *folderToMove, *targetedFolder);
	}

	//copy sub folders
	std::vector<AssetFolder>& subFolders = folderToMove->getSubFolders();
	const int subFolderSize = subFolders.size();
	for (int i = 0; i < subFolderSize; i++)
	{
		copySubFolderTo(subFolders[i].getName(), *folderToMove, *targetedFolder);
	}
}

void AssetTree::renameSubFolderIn(const std::string& folderName, const std::string& newFolderName, AssetFolder& parentFolder)
{
	assert(FileHandler::isValidFileOrDirectoryName(newFolderName));
	if (!FileHandler::isValidFileOrDirectoryName(newFolderName))
		return;

	if (folderName == newFolderName)
		return;

	assert(parentFolder.getSubFolder(folderName) != nullptr);
	if (parentFolder.getSubFolder(folderName) == nullptr)
		return;

	AssetFolder* folderToRename = parentFolder.getSubFolder(folderName);
	assert(folderToRename != nullptr);
	if (folderToRename == nullptr)
		return;

	FileHandler::Path folderPath(/*Project::getPath().toString() + "/" + */folderToRename->getPath().toString());

	folderToRename->rename(newFolderName);
	FileHandler::renameDirectory(folderPath, newFolderName);
}

void AssetTree::renameAssetIn(AssetFile& fileToRename, const std::string& newFileName, AssetFolder& parentFolder)
{
	assert(FileHandler::isValidFileOrDirectoryName(newFileName));
	if (!FileHandler::isValidFileOrDirectoryName(newFileName))
		return;

	if (fileToRename.getDisplayName() == newFileName)
		return;

	assert(parentFolder.getFile(fileToRename.getFilenameWithExtention()) != nullptr);
	if (parentFolder.getFile(fileToRename.getFilenameWithExtention()) == nullptr)
		return;

	FileHandler::CompletePath filePath(fileToRename.getPath().toString());

	fileToRename.rename(newFileName);
	FileHandler::renameFile(filePath, newFileName);
}

void AssetTree::addExternalAssetTo(const FileHandler::CompletePath& assetPath, AssetFolder& folderTo)
{
	assert(!folderTo.hasFile(assetPath.getFilenameWithExtention()));
	if (folderTo.hasFile(assetPath.getFilenameWithExtention()))
		return;

	FileHandler::CompletePath newAssetPath(folderTo.getPath().toString(), assetPath.getFilenameWithExtention());

	// We first copy the file inside the engine asset folder
	FileHandler::copyPastFile(assetPath, newAssetPath);
	// Then, we load the file as a new asset
	Asset* newAsset = AssetManager::instance().loadSingleAsset(newAssetPath);
	if(newAsset != nullptr)
		folderTo.addFile(assetPath.getFilenameWithExtention(), newAsset->getAssetId());
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
//		std::cout << "error, can't load AssetTree window at path : " << resourcePath << std::endl;
//		return;
//	}
//	Json::Value root;
//	streamResources >> root.get(;
//
//	loadRecursivly(root);
//}

AssetTreeView::AssetTreeView(const std::string& name, AssetTree* model, Editor* editorPtr)
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
	//	AssetFile resource(resourceName, ResourceType::MESH);
	//	if (containsFileRecursivly(resourceName, &resourcePath))
	//	{

	//	}
	//}

	/*m_model->addSubFolder("toto");
	m_model->getSubFolder("toto").addFile(AssetFile("FileToto01", ResourceType::MESH));
	m_model->getSubFolder("toto").addFile(AssetFile("FileToto02", ResourceType::MESH));
	m_model->getSubFolder("toto").addFile(AssetFile("FileToto03", ResourceType::MESH));

	m_model->addSubFolder("tutu");
	m_model->getSubFolder("tutu").addFile(AssetFile("FileTutu01", ResourceType::MESH));
	m_model->getSubFolder("tutu").addFile(AssetFile("FileTutu02", ResourceType::MESH));

	m_model->addFile(AssetFile("File01", ResourceType::MESH));*/
}

AssetTreeView::~AssetTreeView()
{

}

void AssetTreeView::displayFiles(AssetFolder* parentFolder, AssetFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback)
{
	for (int fileIdx = 0; fileIdx < currentFolder.fileCount(); fileIdx++)
	{
		AssetFile& currentFile = currentFolder.getFile(fileIdx);

		//TODO : couleur à changer en fonction du type de resource.
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

		bool isItemHovered = false;
		if (!currentFile.isBeingRenamed())
		{

			bool isActive = (currentFolder.getPath() == m_model->getSelectedFileFolderPath() 
				&& m_model->getSelectedFileNameWithExtention() == currentFile.getFilenameWithExtention());

			if (ImGui::RadioButton(currentFile.getDisplayName().c_str(), isActive))
			{
				m_model->selectAsset(currentFolder.getPath(), currentFile.getFilenameWithExtention());
				m_editorPtr->onResourceSelected();
			}

			isItemHovered = ImGui::IsItemHovered();
			ImGui::SameLine();
			currentFile.drawIconeInAssetTree();

			if (isItemHovered)
			{
				if (ImGui::IsMouseClicked(1))
				{
					if (outOpenModaleCallback != nullptr)
					{
						outOpenModaleCallback->shouldOpen = true;
						outOpenModaleCallback->modaleName = "assetFileContextMenu";
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
			DragAndDropManager::beginDragAndDrop(std::make_shared<AssetDragAndDropOperation>(&currentFile, &currentFolder));
		}
	}
}

void AssetTreeView::displayFoldersRecusivly(AssetFolder* parentFolder, AssetFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback, DropCallback* outDropCallback)
{

	int colorStyleModifierCount = 0;
	if (DragAndDropManager::isDragAndDropping() && (DragAndDropManager::getOperationType() | (EditorDragAndDropType::AssetDragAndDrop | EditorDragAndDropType::AssetFolderDragAndDrop)))
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
			DragAndDropManager::beginDragAndDrop(std::make_shared<AssetFolderDragAndDropOperation>(&currentFolder, parentFolder, m_model));
		}

		//recursivity
		for (int subFolderIdx = 0; subFolderIdx < currentFolder.subFolderCount(); subFolderIdx++)
		{
			AssetFolder* itFolder = currentFolder.getSubFolder(subFolderIdx);

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

void AssetTreeView::displayModales()
{
	std::string popupToOpen = "";
	bool needOpenPopup = false;

	//right clic on file menu display :
	if (ImGui::BeginPopup("assetFileContextMenu"))
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
				AssetTree::deleteAssetFrom(*m_fileWeRightClicOn, *m_folderWeRightClicOn);
		}

		// Show per resource right click mouse UI
		if (m_fileWeRightClicOn != nullptr)
		{
			Asset* asset = AssetManager::instance().getAsset(m_fileWeRightClicOn->getAssetId());
			if(asset != nullptr)
				needOpenPopup = asset->drawRightClicContextMenu(popupToOpen);
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
				AssetFolder* folderToMoveOrCopy = m_model->getAssetFolder().getSubFolder(m_folderWaitingPastPath);
				AssetFolder* parentFolder = m_model->getAssetFolder().getSubFolder(folderToMoveOrCopy->getParentFolderPath());
				assert(parentFolder != nullptr);

				if (m_shouldMoveFileOrFolder)
					AssetTree::moveSubFolderTo(folderToMoveOrCopy->getName(), *parentFolder, *m_folderWeRightClicOn);
				else
					AssetTree::copySubFolderTo(folderToMoveOrCopy->getName(), *parentFolder, *m_folderWeRightClicOn);
			}
		}
		else if (m_isMovingItemFolder == false && m_fileWeRightClicOn != nullptr && ImGui::Button("Past file."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr && m_fileWeRightClicOn != nullptr)
			{
				AssetFile* fileToMoveOrCopy = m_model->getAssetFolder().getFile(m_fileWaitingPastPath);
				AssetFolder* parentFolder = m_model->getAssetFolder().getSubFolder(fileToMoveOrCopy->getParentFolderPath());
				assert(parentFolder != nullptr);

				if (m_shouldMoveFileOrFolder)
					AssetTree::moveAssetTo(*fileToMoveOrCopy, *parentFolder, *m_folderWeRightClicOn);
				else
					AssetTree::copyAssetTo(*fileToMoveOrCopy, *parentFolder, *m_folderWeRightClicOn);
			}
		}
		else if (ImGui::Button("Delete folder."))
		{
			//ImGui::EndPopup();
			if (m_folderWeRightClicOn != nullptr)
			{
				AssetFolder* parentFolder = m_model->getAssetFolder().getSubFolder(m_folderWeRightClicOn->getParentFolderPath());
				assert(parentFolder != nullptr);

				AssetTree::deleteSubFolderFrom(m_folderWeRightClicOn->getName(), *parentFolder);
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
		if (ImGui::Button("Material."))
		{
			//ImGui::EndPopup();
			//ImGui::Ext::openStackingPopUp("AddShaderProgramPopUp");
			needOpenPopup = true;
			popupToOpen = "AddMaterialPopUp";
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
	if (ImGui::BeginPopup("AddMaterialInstancePopUp"))
	{
		popUpToAddMaterialInstance();
	}

	if (ImGui::BeginPopup("AddMaterialPopUp"))
	{
		popUpToAddMaterial();
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

void AssetTreeView::popUpToRenameFile()
{
	m_uiString.resize(100);
	ImGui::InputText("##newName", &m_uiString[0], 100);
	m_uiString = m_uiString.substr(0, m_uiString.find_first_of('\0'));
	assert(m_fileWeRightClicOn != nullptr);

	AssetFolder* parentFolder = m_model->getAssetFolder().getSubFolder(m_fileWeRightClicOn->getParentFolderPath());
	assert(parentFolder != nullptr);

	if (!parentFolder->hasFile(m_uiString + m_fileWeRightClicOn->getPath().getExtention()))
	{
		if (FileHandler::isValidFileOrDirectoryName(m_uiString))
		{
			ImGui::SameLine();
			if (ImGui::Button("Validate##validateName") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
			{
				AssetTree::renameAssetIn(*m_fileWeRightClicOn, m_uiString, *parentFolder);

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

void AssetTreeView::popUpToRenameFolder()
{
	m_uiString.resize(100);
	ImGui::InputText("##newName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);

	AssetFolder* parentFolder = m_model->getAssetFolder().getSubFolder(m_folderWeRightClicOn->getParentFolderPath());
	assert(parentFolder != nullptr);

	if (!parentFolder->hasSubFolder(m_uiString))
	{
		if (FileHandler::isValidFileOrDirectoryName(m_uiString))
		{
			ImGui::SameLine();
			if (ImGui::Button("Validate##validateName") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
			{
				AssetTree::renameSubFolderIn(m_folderWeRightClicOn->getName(), m_uiString, *parentFolder);

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

void AssetTreeView::popUpToAddFolder()
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
				AssetTree::addSubFolderTo(m_uiString, *m_folderWeRightClicOn);
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

void AssetTreeView::popUpToChooseMaterial()
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

	auto matIterBegin = AssetManager::instance().getPool<Material>()->getDataIteratorBegin();
	auto matIterEnd = AssetManager::instance().getPool<Material>()->getDataIteratorEnd();

	for (auto& it = matIterBegin; it != matIterEnd; it++)
	{
		const std::string matName = it->getAssetName();
		const std::string btnLabel = matName + "##" + std::to_string(tmpProgramIdx++);

		if (ImGui::Button(btnLabel.data()))
		{
			//m_chooseMaterialName = matName;
			ImGui::EndPopup();
			ImGui::Ext::openStackingPopUp("AddMaterialPopUp");
			shouldEndPopup = false;
		}
		//else
		//	ImGui::EndPopup();
	}

	if (shouldEndPopup)
		ImGui::EndPopup();
}

void AssetTreeView::popUpToAddMaterial()
{
	//assert(!m_chooseShderProgram.empty());

	m_uiString.resize(100);
	ImGui::InputText("##shaderProgramName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(m_uiString + ".mat"))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddShaderProgram") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				AssetTree::addNewMaterialTo(m_uiString, *m_folderWeRightClicOn);
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

void AssetTreeView::popUpToAddMaterialInstance()
{
	//assert(!m_chooseShderProgram.empty());

	m_uiString.resize(100);
	ImGui::InputText("##materialName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(m_uiString + ".imat"))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddMaterial") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				AssetTree::addNewMaterialInstanceTo(m_uiString, *m_fileWeRightClicOn, *m_folderWeRightClicOn);
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

void AssetTreeView::popUpToAddCubeTexture()
{
	m_uiString.resize(100);
	ImGui::InputText("##fileName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(m_uiString + ".cTex"))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddFile") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				AssetTree::addNewCubeTextureTo(m_uiString, *m_folderWeRightClicOn);
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

void AssetTreeView::popUpToAddAnimationStateMachine()
{
	m_uiString.resize(100);
	ImGui::InputText("##fileName", &m_uiString[0], 100);
	assert(m_folderWeRightClicOn != nullptr);
	if (!m_folderWeRightClicOn->hasFile(m_uiString + ".asm"))
	{
		ImGui::SameLine();
		if (ImGui::Button("Validate##AddFile") || ImGui::IsKeyPressed(GLFW_KEY_ENTER))
		{
			if (m_folderWeRightClicOn != nullptr)
			{
				AssetTree::addNewAnimationStateMachineTo(m_uiString, *m_folderWeRightClicOn);
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

void AssetTreeView::drawContent(Project& project, EditorModal* parentModale)
{
	DropCallback dropCallback(nullptr, EditorDropContext::DropIntoFileOrFolder);
	OpenModaleCallback openModaleCallback;

	//Display asset folder
	displayFoldersRecusivly(nullptr, m_model->getAssetFolder(), &openModaleCallback, &dropCallback);

	//show modals if needed
	openModaleCallback.openCallbackIfNeeded();
	displayModales();

	//handle drop asynchronously
	if (dropCallback.currentFolder != nullptr)
	{
		DragAndDropManager::dropDraggedItem(dropCallback.currentFolder, EditorDropContext::DropIntoFileOrFolder);
	}

	ImGui::Separator();

	//Display defaults (no callback, no drop)
	displayFoldersRecusivly(nullptr, m_model->getDefaultAssetFolder(), nullptr, nullptr);
}

void AssetTreeView::setModel(AssetTree* model)
{
	m_model = model;
}