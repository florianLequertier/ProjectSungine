#pragma once

#include <map>

#include "cereal.hpp"

#include "Object.h"
#include "AssetPool.h"

#include "Project.h"

class AssetManager
{
private:
	static std::map<FileHandler::FileType, int> m_fileTypeToObjectClassId;

	std::map<int, IObjectPool*> m_poolMapping;

	AssetPool<Mesh> m_meshPool;
	//...

public:
	void registerAssetFileTypes(int objectClassId, const std::vector<FileHandler::FileType>& fileTypes)
	{
		for (auto& fileType : fileTypes)
		{
			m_fileTypeToObjectClassId[fileType] = objectClassId;
		}
	}

public:
	AssetManager()
	{
		m_meshPool.resize(ASSET_ELEMENT_COUNT);
		m_poolMapping[Object::getStaticClassId<Mesh>()] = &m_meshPool;
		//...
	}

	void loadAssets()
	{
		const FileHandler::Path& assetsPath = Project::getAssetsFolderPath();

		loadAssetsRec(m_assetTree, assetPath);
	}

	void loadAssetsRec(const AssetFolder& currentFolder, const FileHandler::Path& folderPath)
	{
		std::vector<std::string> dirNames;
		FileHandler::getAllDirNames(folderPath, dirNames);

		for (auto& dirName : dirNames)
		{
			int subFolderIdx = 0;
			if (currentFolder.addSubFolder(dirName, &subFolderIdx) )
			{
				loadAssetsRec(FileHandler::Path(folderPath, dirName));
			}
		}

		std::vector<std::string> fileNames;
		FileHandler::getAllFileNames(folderPath, fileNames);
		std::string outExtention;

		for (auto& fileNameAndExtention : fileNames)
		{
			//We only add files that engine understand
			FileHandler::getExtentionFromExtendedFilename(fileNameAndExtention, outExtention);
			FileHandler::FileType fileType = FileHandler::getFileTypeFromExtention(outExtention);
			if (FileHandler::getFileTypeFromExtention(outExtention) != FileHandler::FileType::NONE)
			{
				currentFolder.addFile(fileNameAndExtention);

				FileHandler::CompletePath assetPath(folderPath, fileNameAndExtention);
				auto foundClassId = m_fileTypeToObjectClassId.find(fileType);
				if (foundClassId != m_fileTypeToObjectClassId.end())
				{
					m_poolMapping[foundClassId->second]->loadAsset(assetPath);
				}
			}
		}
	}

	// Only serealize asset mapping
	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(m_meshPool);
		//...
	}
};