//#pragma once
//
//#include <string>
//#include <map>
//#include <vector>
//#include <memory>
//
//#include "EditorGUI.h"
//#include "EditorNodes.h"
//#include "FileHandler.h"
//#include "Factories.h"
//#include "Resource.h"
//
////forward :
//class Editor;
//
////handle simple key which are unique for each file, taking the file name and the file type into account
//struct ResourceFileKey
//{
//	ResourceType type;
//	std::string name;
//
//	ResourceFileKey(ResourceType _type = ResourceType::NONE, std::string _name = "") : name(_name), type(_type)
//	{}
//
//	ResourceFileKey(const FileHandler::CompletePath& completePath)
//	{
//		type = getResourceTypeFromFileType(completePath.getFileType());
//		assert(type != ResourceType::NONE);
//		name = completePath.getFilename();
//	}
//
//	bool operator==(const ResourceFileKey& other) const {
//		return type == other.type && name == other.name;
//	}
//
//	bool operator<(const ResourceFileKey& other) const {
//		if (name == other.name)
//		{
//			return type < other.type;
//		}
//		else
//		{
//			return name < other.name;
//		}
//	}
//};
//
//class ResourceFile : public IDrawableInInspector
//{
//private:
//	//TODO
//	//ResourceFactory* m_factory;
//	ResourceFileKey m_key;
//	FileHandler::CompletePath m_path;
//	bool m_isBeingRenamed;
//	Resource* m_pointedResource;
//
//public:
//
//	ResourceFile(const FileHandler::CompletePath& completePath)
//		: m_key(ResourceType::NONE, completePath.getFilename())
//		, m_path(completePath)
//		, m_isBeingRenamed(false)
//	{
//		m_key.type = getResourceTypeFromFileType(completePath.getFileType());
//		assert(m_key.type != ResourceType::NONE);
//		m_pointedResource = getResourceFromTypeAndCompletePath(m_key.type, completePath);
//		assert(m_pointedResource != nullptr);
//	}
//
//	ResourceFile(const ResourceFile& other)
//		: m_key(other.m_key)
//		, m_path(other.m_path)
//		, m_isBeingRenamed(false)
//		, m_pointedResource(other.m_pointedResource)
//	{
//		assert(m_key.type != ResourceType::NONE);
//		assert(m_pointedResource != nullptr);
//	}
//
//	//ResourceFile(const ResourceFile& other, const std::string& path)
//	//	: ResourceFile(other)
//	//{
//	//	m_path = path;
//	//}
//
//	void begingRenamingResource()
//	{
//		m_isBeingRenamed = true;
//	}
//
//	void endRenamingResource()
//	{
//		m_isBeingRenamed = false;
//	}
//
//	bool isBeingRenamed() const
//	{
//		return m_isBeingRenamed;
//	}
//
//	bool drawRenamingInputText()
//	{
//		bool enterPressed = false;
//		m_key.name.reserve(100);
//		enterPressed = ImGui::InputText("##currentRenamedText", &m_key.name[0], 100, ImGuiInputTextFlags_EnterReturnsTrue);
//		return (enterPressed || ImGui::IsKeyPressed(GLFW_KEY_TAB) || (!ImGui::IsItemHovered() && ImGui::IsMouseClickedAnyButton()));
//	}
//
//	const std::string& getName() const { return m_key.name; }
//	const FileHandler::CompletePath& getPath() const { return m_path; }
//	ResourceType getType() const { return m_key.type; }
//	const ResourceFileKey& getKey() const { return m_key; }
//	FileHandler::Path getParentFolderPath() const { 
//		return m_path.getPath();
//	}
//
//	void rename(const std::string& newName)
//	{
//		FileHandler::CompletePath oldFilePath = getPath();
//		FileHandler::Path newPathToFile = m_path.getPath();
//		newPathToFile.pop_back();
//		FileHandler::CompletePath newFilePath(newPathToFile, newName, m_path.getExtention());
//
//		m_path = FileHandler::CompletePath(newFilePath);
//		m_key.name = newName;
//
//		renameResourceInFactory(oldFilePath, newFilePath);
//	}
//
//	void onParentRenamed(const FileHandler::Path& newPathToParent)
//	{
//		m_path = FileHandler::CompletePath(newPathToParent, m_path.getFilenameWithExtention());
//	}
//
//	bool operator<(const ResourceFile& other) { return m_key < other.m_key; }
//
//	void drawInInspector(Scene & scene, const std::vector<IDrawableInInspector*>& selection) override;
//	void drawInInspector(Scene & scene) override;
//
//	void drawIconeInResourceTree();
//	void drawUIOnHovered();
//
//	Resource* getPointedResource() const
//	{
//		return m_pointedResource;
//	}
//};
//
//class ResourceFolder
//{
//protected:
//	std::vector<ResourceFile> m_filesContainer;
//	std::vector<ResourceFolder> m_subFoldersContainer;
//	std::string m_name;
//	FileHandler::Path m_path;
//
//public:
//	ResourceFolder()
//		: m_name("")
//		, m_path("")
//	{ }
//
//	ResourceFolder(const FileHandler::Path& path) 
//		: m_name(path.back())
//		, m_path(path)
//	{ }
//
//	virtual ~ResourceFolder()
//	{ }
//
//	//ResourceFolder(const ResourceFolder& other, const std::string& path)
//	//	: ResourceFolder(other)
//	//{ 
//	//	m_path = path;
//	//}
//
//	const std::string& getName() const { return m_name; }
//	const FileHandler::Path& getPath() const { return m_path; }
//	FileHandler::Path getParentFolderPath() const {
//		return m_path.getSubPath(0, m_path.size() - 1);
//	}
//
//	//deals with files 
//	std::vector<ResourceFile>::iterator filesBegin() { return m_filesContainer.begin(); };
//	std::vector<ResourceFile>::iterator filesEnd() { return m_filesContainer.end(); };
//	size_t fileCount() { return m_filesContainer.size(); }
//	ResourceFile& getFile(int idx) { return m_filesContainer[idx]; }
//
//	void addFile(const std::string& fileNameAndExtention) 
//	{
//		FileHandler::CompletePath resourcePath(m_path, fileNameAndExtention);
//
//		//addResourceToFactory(resourcePath);
//		m_filesContainer.push_back(ResourceFile(resourcePath));
//	}
//
//	void addFile(const FileHandler::CompletePath& filePath) 
//	{ 
//		//addResourceToFactory(filePath);
//		m_filesContainer.push_back(ResourceFile(filePath));
//	}
//
//	template<typename U>
//	void addFile(const FileHandler::CompletePath& filePath, U* resource)
//	{
//		getResourceFactory<U>().addResourceForce(filePath, resource);
//		m_filesContainer.push_back(ResourceFile(filePath));
//	}
//
//	void moveFileFrom(const FileHandler::CompletePath& oldPath, const FileHandler::CompletePath& newPath, ResourceFolder& folderFrom)
//	{
//		m_filesContainer.push_back(ResourceFile(newPath));
//		folderFrom.removeFile(ResourceFileKey(getResourceTypeFromFileType(FileHandler::getFileTypeFromExtention(oldPath.getExtention())), oldPath.getFilename()), false);
//
//		renameResourceInFactory(oldPath, newPath);
//	}
//
//	void removeFile(const ResourceFileKey& fileKey, bool synchronizeWithFactory = true)
//	{
//		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
//			if (itFile->getKey() == fileKey)
//			{
//				if(synchronizeWithFactory)
//					removeResourceFromFactory(itFile->getPath());
//				m_filesContainer.erase(itFile);
//
//				return;
//			}
//	}
//
//	void removeAllFiles()
//	{
//		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
//		{
//			removeResourceFromFactory(itFile->getPath());
//		}
//
//		m_filesContainer.clear();
//	}
//
//	bool hasFile(const ResourceFileKey& fileKey) const
//	{
//		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
//			if (itFile->getKey() == fileKey)
//				return true;
//		return false;
//	}
//
//	ResourceFile* getFile(const ResourceFileKey& fileKey)
//	{
//		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
//			if (itFile->getKey() == fileKey)
//			{
//					return &(*itFile);
//			}
//		return nullptr;
//	}
//
//	ResourceFile* getFile(const FileHandler::CompletePath& filePath, int depth = 0)
//	{
//		ResourceFolder* foundFolder = getSubFolder(filePath.getPath());
//
//		if (foundFolder != nullptr)
//		{
//			return foundFolder->getFile(ResourceFileKey(getResourceTypeFromFileType(FileHandler::getFileTypeFromExtention(filePath.getExtention())), filePath.getFilename()));
//		}
//		else
//			return nullptr;
//	}
//
//	void rename(const std::string& newName)
//	{
//		m_path.pop_back();
//		m_path.push_back(newName);
//		m_name = newName;
//
//		for (auto& file : m_filesContainer)
//		{
//			file.onParentRenamed(m_path);
//		}
//
//		for (auto& folder : m_subFoldersContainer)
//		{
//			folder.onParentRenamed(m_path);
//		}
//	}
//
//	void onParentRenamed(const FileHandler::Path& newPathToParent)
//	{
//		m_path = newPathToParent;
//		m_path.push_back(m_name);
//
//		for (auto& file : m_filesContainer)
//		{
//			file.onParentRenamed(m_path);
//		}
//
//		for (auto& folder : m_subFoldersContainer)
//		{
//			folder.onParentRenamed(m_path);
//		}
//	}
//
//	//deals with sub folders :
//	std::vector<ResourceFolder>::iterator subFoldersBegin() { return m_subFoldersContainer.begin(); };
//	std::vector<ResourceFolder>::iterator subFoldersEnd() { return m_subFoldersContainer.end(); };
//	size_t subFolderCount() { return m_subFoldersContainer.size(); }
//	ResourceFolder* getSubFolder(int idx) 
//	{ 
//		if (idx < 0 || idx >= m_subFoldersContainer.size())
//			return nullptr;
//
//		return &m_subFoldersContainer[idx];
//	}
//	ResourceFolder* getSubFolder(const std::string& folderName)
//	{ 
//		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//			if (itFolder->getName() == folderName)
//				return &(*itFolder);
//
//		return nullptr;
//	}
//	ResourceFolder* getSubFolder(const FileHandler::Path& folderPath, int depth = 0)
//	{
//		if (depth > folderPath.size() - 1)
//			return nullptr;
//
//		std::string currentFolderName = folderPath[depth];
//
//		if (depth == folderPath.size() - 1)
//			if (getName() == currentFolderName)
//				return this;
//
//		std::string nextFolderName = folderPath[depth+1];
//
//		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//		{
//			if (itFolder->getName() == nextFolderName)
//			{
//				return itFolder->getSubFolder(folderPath, ++depth);
//			}
//		}
//
//		return nullptr;
//	}
//
//	bool addSubFolder(const std::string& folderName, int* outFolderIdx = nullptr)
//	{
//		if (hasSubFolder(folderName, outFolderIdx))
//			return false;
//
//		m_subFoldersContainer.push_back(ResourceFolder(FileHandler::Path(m_path, folderName)));
//
//		if (outFolderIdx != nullptr)
//		{
//			*outFolderIdx = (m_subFoldersContainer.size() - 1);
//		}
//		return true;
//	}
//
//	bool addSubFolder(ResourceFolder& folder, int* outFolderIdx = nullptr)
//	{
//		if (hasSubFolder(folder, outFolderIdx))
//			return false;
//
//		m_subFoldersContainer.push_back(folder);
//		if (outFolderIdx != nullptr)
//		{
//			*outFolderIdx = (m_subFoldersContainer.size() - 1);
//		}
//		return true;
//	}
//
//	void removeSubFolder(const std::string& folderName)
//	{
//		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//			if (itFolder->getName() == folderName)
//			{
//				//recursivly clear sub folders and files, then remove the folder from folder container
//				itFolder->removeAllFiles();
//				itFolder->removeAllSubFolders();
//				m_subFoldersContainer.erase(itFolder);
//				return;
//			}
//	}
//
//	void removeAllSubFolders()
//	{
//		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//		{
//			itFolder->removeAllSubFolders();
//		}
//		m_subFoldersContainer.clear();
//	}
//
//	bool hasSubFolder(const std::string& folderName, int* outSubFolderIdx = nullptr)
//	{
//		int idx = 0;
//		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//		{
//			if (itFolder->getName() == folderName)
//			{
//				if (outSubFolderIdx != nullptr)
//				{
//					*outSubFolderIdx = idx;
//				}
//				return true;
//			}
//			idx++;
//		}
//		return false;
//	}
//
//	bool hasSubFolder(const ResourceFolder& folder, int* outSubFolderIdx = nullptr)
//	{
//		int idx = 0;
//		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//		{
//			if (itFolder->getName() == folder.getName())
//			{
//				if (outSubFolderIdx != nullptr)
//				{
//					*outSubFolderIdx = idx;
//				}
//				return true;
//			}
//			idx++;
//		}
//		return false;
//	}
//
//	std::vector<ResourceFolder>& getSubFolders()
//	{
//		return m_subFoldersContainer;
//	}
//
//	std::vector<ResourceFile>& getFiles()
//	{
//		return m_filesContainer;
//	}
//
//	bool ResourceFolder::searchFileRecursivly(const ResourceFileKey& fileKey, ResourceFile* outFile, std::vector<std::string>* outPathToFile)
//	{
//		outFile = getFile(fileKey);
//		if (outFile != nullptr)
//			return true;
//		else
//		{
//			bool fileFound = false;
//			for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//			{
//				if (outPathToFile != nullptr)
//					outPathToFile->push_back(getName());
//				fileFound = searchFileRecursivly(fileKey, outFile, outPathToFile);
//				if (fileFound)
//					return true;
//
//				if (outPathToFile != nullptr)
//					outPathToFile->pop_back();
//			}
//			return false;
//		}
//	}
//
//	bool containsFileRecursivly(const ResourceFileKey& fileKey, std::vector<std::string>* outPathToFile)
//	{
//		if (hasFile(fileKey))
//			return true;
//		else
//		{
//			for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//			{
//				if (outPathToFile != nullptr)
//					outPathToFile->push_back(getName());
//				containsFileRecursivly(fileKey, outPathToFile);
//				if (outPathToFile != nullptr)
//					outPathToFile->pop_back();
//			}
//		}
//	}
//
//	int getFileCountRecursivly()
//	{
//		int fileCount = 0;
//		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
//		{
//			fileCount += m_filesContainer.size() + getFileCountRecursivly();
//		}
//		return fileCount;
//	}
//
//	//Recursivly get datas and construct hierachy from explorer folder hierarchy
//	void fillDatasFromExplorerFolder(const FileHandler::Path& folderPath);
//
//	//return true if the path exists in the current hierachy
//	bool pathExists(const FileHandler::Path& path, ResourceFolder* outFolderAtPath = nullptr) const
//	{
//		if (path.size() >= 1)
//		{
//			const std::string& firstFolderName = path[0];
//		
//			for (auto& subFolder : m_subFoldersContainer)
//			{
//				if (subFolder.getName() == firstFolderName)
//				{
//					if (outFolderAtPath != nullptr)
//						*outFolderAtPath = subFolder;
//
//					FileHandler::Path subPath(path);
//					subPath.pop_front();
//					return subFolder.pathExists(subPath, outFolderAtPath);
//				}
//			}
//
//			return false;
//		}
//		return true;
//	}
//
//	bool operator<(const ResourceFolder& other) { return m_name < other.m_name; }
//
//	//move a subFolder to a new location
//	bool moveSubFolderToNewLocation(const std::string& subFolderName, ResourceFolder& newLocation);
//	bool copySubFolderToNewLocation(const std::string& subFolderName, ResourceFolder& newLocation);
//
//private:
//	//move the folder to a new location, used by moveSubFolderToNewLocation.
//	bool moveTo(ResourceFolder& newLocation);
//	bool copyTo(ResourceFolder& newLocation);
//};
//
////utility callback to handle asynchronous file or folder removal
//struct DropCallback
//{
//	ResourceFolder* currentFolder;
//	EditorDropContext dropContext;
//
//	DropCallback(ResourceFolder* _currentFolder, EditorDropContext _dropContext) : currentFolder(_currentFolder), dropContext(_dropContext)
//	{}
//};
//
////utility callback to open modale asynchronously
//struct OpenModaleCallback
//{
//	std::string modaleName;
//	bool shouldOpen;
//
//	OpenModaleCallback() : shouldOpen(false)
//	{}
//
//	void openCallbackIfNeeded()
//	{
//		if (shouldOpen)
//		{
//			ImGui::OpenPopup(modaleName.data());
//		}
//	}
//};
//
//
//class ResourceTree : public ResourceFolder
//{
//private:
//	FileHandler::Path m_selectedFileFolderPath;
//	ResourceFileKey m_selectedFileKey;
//
//public :
//	ResourceTree(const FileHandler::Path& assetResourcePath);
//	virtual ~ResourceTree()
//	{}
//
//	ResourceFile* getSelectedResource();
//	const FileHandler::Path& getSelectedFileFolderPath() const;
//	const ResourceFileKey& getSelectedFileKey() const;
//	void setSelectedFileFolderPath(const FileHandler::Path& resourceFolderPath);
//	void setSelectedFileKey(const ResourceFileKey& resourceKey);
//
//	static void deleteSubFolderFrom(const std::string& folderName, ResourceFolder& folderFrom);
//	static void deleteResourceFrom(const ResourceFile& resourceFileToDelete, ResourceFolder& folderFrom);
//
//	static void moveResourceTo(const ResourceFile& resourceFileToMove, ResourceFolder& folderFrom, ResourceFolder& folderTo);
//	static void copyResourceTo(const ResourceFile& resourceFileToMove, ResourceFolder& folderFrom, ResourceFolder& folderTo);
//
//	static void addNewMaterialTo(const std::string& newMaterialName, ResourceFile& shaderProgramFile, ResourceFolder& folderTo);
//	//static void addNewMaterialTo(const std::string& materialName, const std::string& ShaderProgramName, ResourceFolder& folderTo); //DEPRECATED
//	static void addNewShaderProgramTo(const std::string& shaderProgramName, ResourceFolder& folderTo);
//	static void addNewCubeTextureTo(const std::string& textureName, ResourceFolder& folderTo);
//	static void addNewAnimationStateMachineTo(const std::string& textureName, ResourceFolder& folderTo);
//	static void addSubFolderTo(const std::string& folderName, ResourceFolder& folderTo);
//
//	static void moveSubFolderTo(const std::string& folderName, ResourceFolder& folderFrom, ResourceFolder& folderTo);
//	static void copySubFolderTo(const std::string& folderName, ResourceFolder& folderFrom, ResourceFolder& folderTo);
//
//	static void renameSubFolderIn(const std::string& folderName, const std::string& newFolderName, ResourceFolder& parentFolder);
//	static void renameResourceIn(ResourceFile& fileToRename, const std::string& newFileName, ResourceFolder& parentFolder);
//
//	static void addExternalResourceTo(const FileHandler::CompletePath& resourcePath, ResourceFolder& resourceFolder);
//
//};
//
//
//class ResourceTreeView : public EditorFrame
//{
////private:
////	std::vector<ResourceFolder> m_resourceFolders;
////	std::vector<ResourceFile> m_resourceFiles;
//private:
//	ResourceTree* m_model;
//	Editor* m_editorPtr;
//
//	std::string m_uiString;
//	ResourceFolder* m_folderWeRightClicOn;
//	ResourceFile* m_fileWeRightClicOn;
//	//std::string m_chooseMaterialName;
//
//	bool m_isMovingItemFolder;
//	bool m_shouldMoveFileOrFolder;
//	FileHandler::CompletePath m_fileWaitingPastPath;
//	FileHandler::Path m_folderWaitingPastPath;
//
//public:
//	ResourceTreeView(const std::string& name, ResourceTree* model, Editor* editorPtr);
//	~ResourceTreeView();
//	virtual void drawContent(Project& project, EditorModal* parentModale = nullptr) override;
//	void setModel(ResourceTree* model);
//	/*void addFolder(const std::string& folderName);
//	void removeFolder(const std::string& folderName);
//	ResourceFolder& getFolder(const std::string& folderName);
//	void addFileToFolder(ResourceFile file, const std::string& folderName);
//	void addFileToFolder(ResourceFile file, size_t folderIdx);*/
//	
//	void displayFiles(ResourceFolder* parentFolder, ResourceFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback);
//	void displayFoldersRecusivly(ResourceFolder* parentFolder, ResourceFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback, DropCallback* outDropCallback = nullptr);
//	void displayModales();
//	//void displayFoldersRecusivly(ResourceFolder* parentFolder, std::vector<ResourceFolder>& foldersToDisplay, std::vector<ResourceFile>& filesToDisplay);
//	void popUpToRenameFile();
//	void popUpToRenameFolder();
//	void popUpToAddFolder();
//	void popUpToAddCubeTexture();
//	void popUpToAddAnimationStateMachine();
//	void popUpToChooseMaterial();
//	void popUpToAddShaderProgram();
//	void popUpToAddMaterial();
//
//	//save / load functions :
//	//void save() const;
//	//void load();
//	//void saveRecursivly(Json::Value& root) const;
//	//void loadRecursivly(Json::Value& root);
//};