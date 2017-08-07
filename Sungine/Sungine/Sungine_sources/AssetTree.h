#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "EditorGUI.h"
#include "EditorNodes.h"
#include "FileHandler.h"
#include "AssetManager.h"

//forward :
class Editor;

//handle simple key which are unique for each file, taking the file name and the file type into account
//struct AssetFileKey
//{
//	FileHandler::FileType type;
//	std::string name;
//	std::string nameWithExtention;
//
//	AssetFileKey(FileHandler::FileType _type = FileHandler::FileType::NONE
//		, const std::string& _name = ""
//		, const std::string& _nameWithExtention = "") 
//		: name(_name)
//		, type(_type)
//		, nameWithExtention(_nameWithExtention)
//	{}
//
//	AssetFileKey(const FileHandler::CompletePath& completePath)
//	{
//		type = completePath.getFileType();
//		assert(type != ResourceType::NONE);
//		name = completePath.getFilename();
//		nameWithExtention = completePath.getFilenameWithExtention();
//	}
//
//	bool operator==(const AssetFileKey& other) const {
//		return type == other.type && nameWithExtention == other.nameWithExtention;
//	}
//
//	bool operator<(const AssetFileKey& other) const {
//		if (nameWithExtention == other.nameWithExtention)
//		{
//			return type < other.type;
//		}
//		else
//		{
//			return nameWithExtention < other.nameWithExtention;
//		}
//	}
//};

class AssetFile// : public IDrawableInInspector
{
private:
	FileHandler::CompletePath m_path;
	//AssetFileKey m_key;
	std::string m_displayName;
	bool m_isBeingRenamed;
	AssetId m_assetId;
	Asset* m_pointedAsset;

public:

	AssetFile(const FileHandler::CompletePath& completePath, const AssetId& assetId)
		//: m_key(completePath.getFileType(), completePath.getFilename(), completePath.getFilenameWithExtention())
		: m_path(completePath)
		, m_displayName(m_path.getFilename())
		, m_isBeingRenamed(false)
		, m_assetId(assetId)
		, m_pointedAsset(AssetManager::instance().getAsset(m_assetId))
	{
		assert(m_pointedAsset != nullptr);
	}

	AssetFile(const AssetFile& other)
		//: m_key(other.m_key)
		: m_path(other.m_path)
		, m_displayName(other.m_displayName)
		, m_isBeingRenamed(false)
		, m_assetId(other.m_assetId)
		, m_pointedAsset(other.m_pointedAsset)
	{
		assert(m_pointedAsset != nullptr);
	}

	//AssetFile(const AssetFile& other, const std::string& path)
	//	: AssetFile(other)
	//{
	//	m_path = path;
	//}

	void begingRenamingResource()
	{
		m_isBeingRenamed = true;
	}

	void endRenamingResource()
	{
		m_isBeingRenamed = false;
	}

	bool isBeingRenamed() const
	{
		return m_isBeingRenamed;
	}

	bool drawRenamingInputText()
	{
		bool enterPressed = false;
		//m_key.name.reserve(100);
		m_displayName.reserve(100);
		enterPressed = ImGui::InputText("##currentRenamedText", &m_displayName[0], 100, ImGuiInputTextFlags_EnterReturnsTrue);
		return (enterPressed || ImGui::IsKeyPressed(GLFW_KEY_TAB) || (!ImGui::IsItemHovered() && ImGui::IsMouseClickedAnyButton()));
	}

	const std::string& getDisplayName() const { return m_displayName; }
	const std::string& getFilenameWithExtention() const { return m_path.getFilenameWithExtention; }
	const FileHandler::CompletePath& getPath() const { return m_path; }
	FileHandler::FileType getType() const { return FileHandler::getFileTypeFromExtention(m_path.getExtention()); }
	FileHandler::Path getParentFolderPath() const { return m_path.getPath(); }

	void rename(const std::string& newName)
	{
		// First, call the rename function inside the AssetManager. 
		//Then, if the operation succedeed, it will change the assetFile name
		if (AssetManager::instance().renameAsset(m_assetId, newName))
		{
			FileHandler::CompletePath oldFilePath = getPath();
			FileHandler::Path newPathToFile = m_path.getPath();
			//newPathToFile.pop_back(); // ???
			FileHandler::CompletePath newFilePath(newPathToFile, newName, m_path.getExtention());

			FileHandler::renameFile(oldFilePath, newName);

			m_path = FileHandler::CompletePath(newFilePath);
			m_displayName = m_path.getFilename();
		}
	}

	void onParentRenamed(const FileHandler::Path& newPathToParent)
	{
		if (AssetManager::instance().moveAsset(m_assetId, newPathToParent))
		{
			m_path = FileHandler::CompletePath(newPathToParent, m_path.getFilenameWithExtention());
		}
	}

	bool operator<(const AssetFile& other) { return m_displayName < other.m_displayName; }

	//void drawInInspector(Scene & scene, const std::vector<IDrawableInInspector*>& selection) override;
	//void drawInInspector(Scene & scene) override;

	void drawIconeInAssetTree()
	{
		if (m_pointedAsset != nullptr)
			m_pointedAsset->drawIconeInResourceTree();
	}

	void drawUIOnHovered()
	{
		if (m_pointedAsset != nullptr)
			m_pointedAsset->drawUIOnHovered();
	}

	AssetId getAssetId() const
	{
		return m_assetId;
	}

	int getAssetType() const
	{
		return m_assetId.type;
	}

	Asset* getPointedAsset() const
	{
		return m_pointedAsset;
	}
};

class AssetFolder
{
protected:
	std::vector<AssetFile> m_filesContainer;
	std::vector<AssetFolder> m_subFoldersContainer;
	std::string m_name;
	FileHandler::Path m_path;

public:
	AssetFolder()
		: m_name("")
		, m_path("")
	{ }

	AssetFolder(const FileHandler::Path& path)
		: m_name(path.back())
		, m_path(path)
	{ }

	virtual ~AssetFolder()
	{ }

	//AssetFolder(const AssetFolder& other, const std::string& path)
	//	: AssetFolder(other)
	//{ 
	//	m_path = path;
	//}

	const std::string& getName() const { return m_name; }
	const FileHandler::Path& getPath() const { return m_path; }
	FileHandler::Path getParentFolderPath() const {	return m_path.getSubPath(0, m_path.size() - 1); }

	//deals with files 
	std::vector<AssetFile>::iterator filesBegin() { return m_filesContainer.begin(); };
	std::vector<AssetFile>::iterator filesEnd() { return m_filesContainer.end(); };
	size_t fileCount() { return m_filesContainer.size(); }
	AssetFile& getFile(int idx) { return m_filesContainer[idx]; }

	void addFile(const std::string& fileNameAndExtention, const AssetId& assetId)
	{
		FileHandler::CompletePath resourcePath(m_path, fileNameAndExtention);

		//addResourceToFactory(resourcePath);
		m_filesContainer.push_back(AssetFile(resourcePath, assetId));
	}

	void addFile(const FileHandler::CompletePath& filePath, const AssetId& assetId)
	{
		//addResourceToFactory(filePath);
		m_filesContainer.push_back(AssetFile(filePath, assetId));
	}

	//template<typename U>
	//void addFile(const FileHandler::CompletePath& filePath, U* resource)
	//{
	//	getResourceFactory<U>().addResourceForce(filePath, resource);
	//	m_filesContainer.push_back(AssetFile(filePath));
	//}

	void moveFileFrom(AssetFolder& folderFrom, const AssetFile& movingFile)
	{
		if (AssetManager::instance().moveAsset(movingFile.getAssetType(), movingFile.getAssetId(), m_path))
		{
			//AssetFile* fileToMove = folderFrom.getFile(filekey);

			m_filesContainer.push_back(movingFile);
			folderFrom.removeFile(movingFile);
		}

		//renameResourceInFactory(oldPath, newPath);
	}

	void removeFile(const AssetFile& fileToRemove)
	{
		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
		{
			if (itFile->getAssetId() == fileToRemove.getAssetId())
			{
				if (AssetManager::instance().removeAsset(fileToRemove.getAssetId()))
				{
					m_filesContainer.erase(itFile);
				}
				return;
			}
		}
	}

	void removeAllFiles()
	{
		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
		{
			if (AssetManager::instance().removeAsset(itFile->getAssetId()))
			{
				m_filesContainer.erase(itFile);
			}
		}

		m_filesContainer.clear();
	}

	bool hasFile(const std::string& filenameWithExtention) const
	{
		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
			if (itFile->getFilenameWithExtention() == filenameWithExtention)
				return true;
		return false;
	}

	AssetFile* getFile(const std::string& filenameWithExtention)
	{
		for (auto& itFile = m_filesContainer.begin(); itFile != m_filesContainer.end(); itFile++)
			if (itFile->getFilenameWithExtention() == filenameWithExtention)
			{
				return &(*itFile);
			}
		return nullptr;
	}

	AssetFile* getFile(const FileHandler::CompletePath& filePath, int depth = 0)
	{
		AssetFolder* foundFolder = getSubFolder(filePath.getPath());

		if (foundFolder != nullptr)
		{
			return foundFolder->getFile(filePath.getFilenameWithExtention());
		}
		else
			return nullptr;
	}

	void rename(const std::string& newName)
	{
		m_path.pop_back();
		m_path.push_back(newName);
		m_name = newName;

		for (auto& file : m_filesContainer)
		{
			file.onParentRenamed(m_path);
		}

		for (auto& folder : m_subFoldersContainer)
		{
			folder.onParentRenamed(m_path);
		}
	}

	void onParentRenamed(const FileHandler::Path& newPathToParent)
	{
		m_path = newPathToParent;
		m_path.push_back(m_name);

		for (auto& file : m_filesContainer)
		{
			file.onParentRenamed(m_path);
		}

		for (auto& folder : m_subFoldersContainer)
		{
			folder.onParentRenamed(m_path);
		}
	}

	//deals with sub folders :
	std::vector<AssetFolder>::iterator subFoldersBegin() { return m_subFoldersContainer.begin(); };
	std::vector<AssetFolder>::iterator subFoldersEnd() { return m_subFoldersContainer.end(); };
	size_t subFolderCount() { return m_subFoldersContainer.size(); }
	AssetFolder* getSubFolder(int idx)
	{
		if (idx < 0 || idx >= m_subFoldersContainer.size())
			return nullptr;

		return &m_subFoldersContainer[idx];
	}
	AssetFolder* getSubFolder(const std::string& folderName)
	{
		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
			if (itFolder->getName() == folderName)
				return &(*itFolder);

		return nullptr;
	}
	AssetFolder* getSubFolder(const FileHandler::Path& folderPath, int depth = 0)
	{
		if (depth > folderPath.size() - 1)
			return nullptr;

		std::string currentFolderName = folderPath[depth];

		if (depth == folderPath.size() - 1)
			if (getName() == currentFolderName)
				return this;

		std::string nextFolderName = folderPath[depth + 1];

		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
		{
			if (itFolder->getName() == nextFolderName)
			{
				return itFolder->getSubFolder(folderPath, ++depth);
			}
		}

		return nullptr;
	}

	bool addSubFolder(const std::string& folderName, int* outFolderIdx = nullptr)
	{
		if (hasSubFolder(folderName, outFolderIdx))
			return false;

		m_subFoldersContainer.push_back(AssetFolder(FileHandler::Path(m_path, folderName)));

		if (outFolderIdx != nullptr)
		{
			*outFolderIdx = (m_subFoldersContainer.size() - 1);
		}
		return true;
	}

	bool addSubFolder(AssetFolder& folder, int* outFolderIdx = nullptr)
	{
		if (hasSubFolder(folder, outFolderIdx))
			return false;

		m_subFoldersContainer.push_back(folder);
		if (outFolderIdx != nullptr)
		{
			*outFolderIdx = (m_subFoldersContainer.size() - 1);
		}
		return true;
	}

	void removeSubFolder(const std::string& folderName)
	{
		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
			if (itFolder->getName() == folderName)
			{
				//recursivly clear sub folders and files, then remove the folder from folder container
				itFolder->removeAllFiles();
				itFolder->removeAllSubFolders();
				m_subFoldersContainer.erase(itFolder);
				return;
			}
	}

	void removeAllSubFolders()
	{
		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
		{
			itFolder->removeAllSubFolders();
		}
		m_subFoldersContainer.clear();
	}

	bool hasSubFolder(const std::string& folderName, int* outSubFolderIdx = nullptr)
	{
		int idx = 0;
		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
		{
			if (itFolder->getName() == folderName)
			{
				if (outSubFolderIdx != nullptr)
				{
					*outSubFolderIdx = idx;
				}
				return true;
			}
			idx++;
		}
		return false;
	}

	bool hasSubFolder(const AssetFolder& folder, int* outSubFolderIdx = nullptr)
	{
		int idx = 0;
		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
		{
			if (itFolder->getName() == folder.getName())
			{
				if (outSubFolderIdx != nullptr)
				{
					*outSubFolderIdx = idx;
				}
				return true;
			}
			idx++;
		}
		return false;
	}

	std::vector<AssetFolder>& getSubFolders()
	{
		return m_subFoldersContainer;
	}

	std::vector<AssetFile>& getFiles()
	{
		return m_filesContainer;
	}

	bool AssetFolder::searchFileRecursivly(const std::string& filenameWithExtention, AssetFile* outFile, std::vector<std::string>* outPathToFile)
	{
		outFile = getFile(filenameWithExtention);
		if (outFile != nullptr)
			return true;
		else
		{
			bool fileFound = false;
			for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
			{
				if (outPathToFile != nullptr)
					outPathToFile->push_back(getName());
				fileFound = searchFileRecursivly(filenameWithExtention, outFile, outPathToFile);
				if (fileFound)
					return true;

				if (outPathToFile != nullptr)
					outPathToFile->pop_back();
			}
			return false;
		}
	}

	bool containsFileRecursivly(const std::string& filenameWithExtention, std::vector<std::string>* outPathToFile)
	{
		if (hasFile(filenameWithExtention))
			return true;
		else
		{
			for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
			{
				if (outPathToFile != nullptr)
					outPathToFile->push_back(getName());
				containsFileRecursivly(filenameWithExtention, outPathToFile);
				if (outPathToFile != nullptr)
					outPathToFile->pop_back();
			}
		}
	}

	int getFileCountRecursivly()
	{
		int fileCount = 0;
		for (auto& itFolder = m_subFoldersContainer.begin(); itFolder != m_subFoldersContainer.end(); itFolder++)
		{
			fileCount += m_filesContainer.size() + getFileCountRecursivly();
		}
		return fileCount;
	}

	////Recursivly get datas and construct hierachy from explorer folder hierarchy
	//void fillDatasFromExplorerFolder(const FileHandler::Path& folderPath);

	//return true if the path exists in the current hierachy
	bool pathExists(const FileHandler::Path& path, AssetFolder* outFolderAtPath = nullptr) const
	{
		if (path.size() >= 1)
		{
			const std::string& firstFolderName = path[0];

			for (auto& subFolder : m_subFoldersContainer)
			{
				if (subFolder.getName() == firstFolderName)
				{
					if (outFolderAtPath != nullptr)
						*outFolderAtPath = subFolder;

					FileHandler::Path subPath(path);
					subPath.pop_front();
					return subFolder.pathExists(subPath, outFolderAtPath);
				}
			}

			return false;
		}
		return true;
	}

	bool operator<(const AssetFolder& other) { return m_name < other.m_name; }

	//move a subFolder to a new location
	bool moveSubFolderToNewLocation(const std::string& subFolderName, AssetFolder& newLocation);
	bool copySubFolderToNewLocation(const std::string& subFolderName, AssetFolder& newLocation);

private:
	//move the folder to a new location, used by moveSubFolderToNewLocation.
	bool moveTo(AssetFolder& newLocation);
	bool copyTo(AssetFolder& newLocation);
};

//utility callback to handle asynchronous file or folder removal
struct DropCallback
{
	AssetFolder* currentFolder;
	EditorDropContext dropContext;

	DropCallback(AssetFolder* _currentFolder, EditorDropContext _dropContext) : currentFolder(_currentFolder), dropContext(_dropContext)
	{}
};

//utility callback to open modale asynchronously
struct OpenModaleCallback
{
	std::string modaleName;
	bool shouldOpen;

	OpenModaleCallback() : shouldOpen(false)
	{}

	void openCallbackIfNeeded()
	{
		if (shouldOpen)
		{
			ImGui::OpenPopup(modaleName.data());
		}
	}
};


class AssetTree
{
private:
	FileHandler::Path m_selectedFileFolderPath;
	std::string m_selectedFileNameWithExtention; // filenameWithExtention

	AssetFolder m_assetFolder;
	AssetFolder m_defaultAssetFolder;

public:
	AssetTree(const FileHandler::Path& assetResourcePath);
	virtual ~AssetTree()
	{}

	void selectAsset(const FileHandler::Path& assetPath, const std::string& assetFilenameWithExtention);

	AssetFile* getSelectedAssetFile();
	const FileHandler::Path& getSelectedFileFolderPath() const;
	const std::string& getSelectedFileNameWithExtention() const;

	AssetFolder& getAssetFolder();
	AssetFolder& getDefaultAssetFolder();

	static void deleteSubFolderFrom(const std::string& folderName, AssetFolder& folderFrom);
	static void deleteAssetFrom(const AssetFile& assetFileToDelete, AssetFolder& folderFrom);

	static void moveAssetTo(const AssetFile& assetFileToMove, AssetFolder& folderFrom, AssetFolder& folderTo);
	static void copyAssetTo(const AssetFile& assetFileToMove, AssetFolder& folderFrom, AssetFolder& folderTo);

	static void addNewMaterialInstanceTo(const std::string& newMaterialName, AssetFile& shaderProgramFile, AssetFolder& folderTo);
	//static void addNewMaterialTo(const std::string& materialName, const std::string& ShaderProgramName, AssetFolder& folderTo); //DEPRECATED
	static void addNewMaterialTo(const std::string& shaderProgramName, AssetFolder& folderTo);
	static void addNewCubeTextureTo(const std::string& textureName, AssetFolder& folderTo);
	static void addNewAnimationStateMachineTo(const std::string& textureName, AssetFolder& folderTo);
	static void addSubFolderTo(const std::string& folderName, AssetFolder& folderTo);

	static void moveSubFolderTo(const std::string& folderName, AssetFolder& folderFrom, AssetFolder& folderTo);
	static void copySubFolderTo(const std::string& folderName, AssetFolder& folderFrom, AssetFolder& folderTo);

	static void renameSubFolderIn(const std::string& folderName, const std::string& newFolderName, AssetFolder& parentFolder);
	static void renameAssetIn(AssetFile& fileToRename, const std::string& newFileName, AssetFolder& parentFolder);

	static void addExternalAssetTo(const FileHandler::CompletePath& assetPath, AssetFolder& folderTo);

};


class AssetTreeView : public EditorFrame
{
	//private:
	//	std::vector<AssetFolder> m_resourceFolders;
	//	std::vector<AssetFile> m_resourceFiles;
private:
	AssetTree* m_model;
	Editor* m_editorPtr;

	std::string m_uiString;
	AssetFolder* m_folderWeRightClicOn;
	AssetFile* m_fileWeRightClicOn;
	//std::string m_chooseMaterialName;

	bool m_isMovingItemFolder;
	bool m_shouldMoveFileOrFolder;
	FileHandler::CompletePath m_fileWaitingPastPath;
	FileHandler::Path m_folderWaitingPastPath;

public:
	AssetTreeView(const std::string& name, AssetTree* model, Editor* editorPtr);
	~AssetTreeView();
	virtual void drawContent(Project& project, EditorModal* parentModale = nullptr) override;
	void setModel(AssetTree* model);
	/*void addFolder(const std::string& folderName);
	void removeFolder(const std::string& folderName);
	AssetFolder& getFolder(const std::string& folderName);
	void addFileToFolder(AssetFile file, const std::string& folderName);
	void addFileToFolder(AssetFile file, size_t folderIdx);*/

	void displayFiles(AssetFolder* parentFolder, AssetFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback);
	void displayFoldersRecusivly(AssetFolder* parentFolder, AssetFolder& currentFolder, OpenModaleCallback* outOpenModaleCallback, DropCallback* outDropCallback = nullptr);
	void displayModales();
	//void displayFoldersRecusivly(AssetFolder* parentFolder, std::vector<AssetFolder>& foldersToDisplay, std::vector<AssetFile>& filesToDisplay);
	void popUpToRenameFile();
	void popUpToRenameFolder();
	void popUpToAddFolder();
	void popUpToAddCubeTexture();
	void popUpToAddAnimationStateMachine();
	void popUpToChooseMaterial();
	void popUpToAddMaterial();
	void popUpToAddMaterialInstance();

	//save / load functions :
	//void save() const;
	//void load();
	//void saveRecursivly(Json::Value& root) const;
	//void loadRecursivly(Json::Value& root);
};