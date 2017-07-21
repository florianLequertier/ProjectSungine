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

public:
	AssetManager();

	// Will load all assets which are in the asset folder into the engine AssetManager.
	template<typename Archive>
	void loadAssets(const Archive& archive)
	{
		for (auto& pool : m_poolMapping)
		{
			pool.second->load(archive);
		}

		const FileHandler::Path& assetsPath = Project::getAssetsFolderPath();

		loadAssetsRec(m_assetTree, assetPath);
	}

	void loadAssetsRec(ResourceFolder& currentFolder, const FileHandler::Path& folderPath);

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

	// Take an object (ex : a Mesh, created with "new Mesh()") and insert it to the asset system.
	// This will call the saveAssetToFile function of the asset.
	template<typename T>
	AssetHandle<T> createAsset(const T& object, const FileHandler::CompletePath& assetPath)
	{
		getPool<T>()->createAsset(object, assetPath);
	}

	template<typename T>
	AssetPool<T>* getPool()
	{
		return static_cast<PoolMapping<T>*>(m_poolMapping[Object::getStaticClassId<T>()]);
	}

	static void createAssetMetaFile(const FileHandler::CompletePath& assetPath, const std::string& metaContent)
	{
		FileHandler::CompletePath assetMetaPath = assetPath;
		assetMetaPath.replaceExtension("meta");

		FileHandler::createFile(assetMetaPath, metaContent); // should use createFileOverride instead
	}

	template<typename T>
	void addDefaultAsset(const Asset& asset)
	{
		getPool<T>()->addDefaultAsset(asset);
	}

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
};