#pragma once

#include <map>

#include "cereal.hpp"

#include "ISingleton.h"
#include "Object.h"
#include "AssetPool.h"

#define ASSET_ELEMENT_COUNT 1000

class Mesh;
class Texture;
class CubeTexture;
namespace Sungine {
	namespace Animation {
		class AnimationStateMachine;
	}
}
class Material;
class MaterialInstance;
class SkeletalAnimation;
class ResourceFolder;

class AssetManager : public ISingleton<AssetManager>
{
private:
	// Mapping : file type -> class id
	static std::unordered_map<FileHandler::FileType, int> m_fileTypeToObjectClassId;

	// Mapping : class id -> asset pool
	std::unordered_map<int, IAssetPool*> m_poolMapping;

	// All the asset pools
	AssetPool<Mesh> m_meshPool;
	AssetPool<Texture> m_texturePool;
	AssetPool<CubeTexture> m_cubeTexturePool;
	AssetPool<Sungine::Animation::AnimationStateMachine> m_animationStateMachinePool;
	AssetPool<Material> m_shaderProgramPool;
	AssetPool<MaterialInstance> m_materialPool;
	AssetPool<SkeletalAnimation> m_skeletalAnimationPool;

public:
	static void registerAssetFileTypes(int objectClassId, const std::vector<FileHandler::FileType>& fileTypes)
	{
		for (auto& fileType : fileTypes)
		{
			m_fileTypeToObjectClassId[fileType] = objectClassId;
		}
	}

	static int getClassIdFromFileType(const FileHandler::FileType& fileType)
	{
		auto found = m_fileTypeToObjectClassId.find(fileType);
		if (found != m_fileTypeToObjectClassId.end())
			return found->second;
		else
			return -1;
	}

public:
	AssetManager();

	// Utility

	template<typename T>
	AssetHandle<T> getHandle(const Asset& asset) const
	{
		getPool<T>()->getAsset(asset.getAssetId());
	}

	// Save / Load

	// Will load all assets which are in the asset folder into the engine AssetManager.
	template<typename Archive>
	void init(const Archive& archive, class AssetTree* tree = nullptr)
	{
		for (auto& pool : m_poolMapping)
		{
			pool.second->load(archive);
		}

		loadAssets(tree);
	}

	Asset* loadSingleAsset(const FileHandler::CompletePath& assetPath);
	void loadAssets(class AssetTree* tree = nullptr);
	void loadDefaultAssets(class AssetTree* tree = nullptr);
	void loadAssetsRec(const FileHandler::Path& folderPath, AssetFolder* currentFolder = nullptr);

	// Save/Load all the pools. Will mostly save metas and mappings and load mappings.
	// Only serealize asset mapping
	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(m_meshPool
		, m_texturePool
		, m_cubeTexturePool
		, m_animationStateMachinePool
		, m_shaderProgramPool
		, m_materialPool
		, m_skeletalAnimationPool);
	}

	// Pool getters

	template<typename T>
	AssetPool<T>* getPool()
	{
		return static_cast<PoolMapping<T>*>(m_poolMapping[Object::getStaticClassId<T>()]);
	}

	IAssetPool* getPool(int assetClassId)
	{
		return m_poolMapping[assetClassId];
	}

	// Add / Create asset and metas

	static void createAssetMetaFile(const FileHandler::CompletePath& assetPath, const std::string& metaContent)
	{
		FileHandler::CompletePath assetMetaPath = assetPath;
		assetMetaPath.replaceExtension("meta");

		FileHandler::createFile(assetMetaPath, metaContent); // should use createFileOverride instead
	}

	// Take an object (ex : a Mesh, created with "new Mesh()") and insert it to the asset system.
	// This will call the saveAssetToFile function of the asset.
	template<typename T>
	AssetHandle<T> createAsset(const T& object, const FileHandler::CompletePath& assetPath)
	{
		getPool<T>()->createAsset(object, assetPath);
	}

	template<typename T>
	void addDefaultAsset(const Asset& asset, const std::string& assetName)
	{
		getPool<T>()->addDefaultAsset(asset, assetName);
	}

	// Asset getters

	template<typename T>
	bool getDefaultAsset(const std::string& assetName, AssetHandle<T>& outHandle)
	{
		return getPool<T>()->getDefaultAsset(assetName, outHandle);
	}

	template<typename T>
	const AssetHandle<T>& getDefaultAsset(const std::string& assetName)
	{
		return getPool<T>()->getDefaultAsset(assetName);
	}

	template<typename T>
	bool getAsset(const AssetId& assetId, AssetHandle<T>& outHandle)
	{
		return getPool<T>()->getAsset(assetName, outHandle);
	}

	template<typename T>
	const AssetHandle<T>& getAsset(const AssetId& assetId)
	{
		return getPool<T>()->getAsset(assetName);
	}

	Asset* getAsset(const AssetId& assetId)
	{
		Asset* found = nullptr;
		for (auto& pool : m_poolMapping)
		{
			found = pool.second->getAssetPtr(assetId);
			if (found != nullptr)
				return found;
		}
		return nullptr;
	}

	// Check asset existence

	bool assetExists(const AssetId& assetId)
	{
		for (auto& pool : m_poolMapping)
		{
			if (pool.second->assetExists(assetId))
				return true;
		}
		return false;
	}

	template<typename T>
	bool assetExists(const AssetId& assetId)
	{
		return getPool<T>()->assetExists();
	}
	
	// Asset actions called by AssetTree feedbacks. Perform operations both on hard drive and on AssetManager

	// Rename

	bool renameAsset(const AssetId& assetId, const std::string& newName)
	{
		Asset* foundAsset = getAsset(assetId);
		if (foundAsset != nullptr)
		{
			return renameAsset(foundAsset, newName);
		}
		return false;
	}

	bool renameAsset(int assetTypeId, const AssetId& assetId, const std::string& newName)
	{
		IAssetPool* pool = getPool(assetTypeId);
		if (pool != nullptr)
		{
			// Modify in asset system
			Asset* targetAsset = pool->getAssetPtr(assetId);
			return renameAsset(targetAsset, newName);
		}
		return false;
	}

	bool renameAsset(Asset* asset, const std::string& newName)
	{
		if (asset != nullptr)
		{
			FileHandler::CompletePath oldAssetPath = asset->getAssetPath();

			// Modify in asset system
			asset->setAssetName(newName);

			// Modify in hard drive, don't forget the meta file
			FileHandler::renameFile(oldAssetPath, newName);
			FileHandler::CompletePath metaPath = oldAssetPath;
			metaPath.replaceExtension(".meta");
			FileHandler::renameFile(metaPath, newName);

			return true;
		}
		return false;
	}

	// Move

	bool moveAsset(const AssetId& assetId, const FileHandler::Path& newFolder)
	{
		Asset* foundAsset = getAsset(assetId);
		if (foundAsset != nullptr)
		{
			return moveAsset(foundAsset, newFolder);
		}
		return false;
	}

	bool moveAsset(int assetTypeId, const AssetId& assetId, const FileHandler::Path& newFolder)
	{
		IAssetPool* pool = getPool(assetTypeId);
		if (pool != nullptr)
		{
			// Modify in asset system
			Asset* targetAsset = pool->getAssetPtr(assetId);
			return moveAsset(targetAsset, newFolder);
		}
		return false;
	}

	bool moveAsset(Asset* asset, const FileHandler::Path& newFolder)
	{
		if (asset != nullptr)
		{
			FileHandler::CompletePath oldAssetPath = asset->getAssetPath();
			FileHandler::CompletePath newAssetPath(newFolder, oldAssetPath.getFilenameWithExtention());

			// Modify in asset system
			asset->setAssetPath(newAssetPath);

			// Modify in hard drive, don't forget the meta file
			FileHandler::moveFile(oldAssetPath, newFolder);
			FileHandler::CompletePath metaPath = oldAssetPath;
			metaPath.replaceExtension(".meta");
			if (FileHandler::fileExists(metaPath))
				FileHandler::moveFile(metaPath, newFolder);
			else
			{
				// If metas doesn't exists yet, we create the meta file
				asset->saveMetas();
			}

			return true;
		}
		return false;
	}

	// Remove

	bool removeAsset(const AssetId& assetId)
	{
		IAssetPool* pool = getPool(assetId.type);
		if (pool != nullptr)
		{
			Asset* asset = pool->getAssetPtr(assetId);
			FileHandler::CompletePath assetPath = asset->getAssetPath();

			// Modify in asset system
			pool->dealocate(assetId);

			// Modify in hard drive, don't forget the meta file
			FileHandler::deleteFile(assetPath);
			FileHandler::CompletePath metaPath = assetPath;
			metaPath.replaceExtension(".meta");
			FileHandler::deleteFile(metaPath);
		
			return true;
		}
		
		return false;
	}

	//TODO

private:
	Asset* loadAsset(const FileHandler::Path& folderPath, const std::string& assetFilenameAndExtention);
};