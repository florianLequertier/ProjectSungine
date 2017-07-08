#pragma once

#include "Object.h"

// A unique id for an asset
struct AssetId
{
	int type;
	int id;

	bool operator==(const AssetId& other)
	{
		return type == other.type && id == other.id;
	}

	bool operator<(const AssetId& other)
	{
		if (type == other.type)
			return id < other.id;
		else
			return type < other.type;
	}
};

// Small class to handle assetId creation
template<typename T>
class AssetIdHandler
{
private:
	static int m_assetLastId = 0;

public:
	static AssetId generateUniqueId()
	{
		AssetId newId;
		newId.id = ++m_assetLastId;
		newId.type = Object::getStaticClassId<T>();
		return newId;
	}

	static int lastGeneratedId()
	{
		return m_assetLastId;
	}
};

// All assets must have a unique id, can be serialisable, and have a meta file which store at least the unique asset id.
template<typename T>
class Asset : public Object
{
	friend class AssetHanler;

private:
	// A unique number which identify this asset
	AssetId m_id;

public:

	Asset()
	{
		m_id = AssetIdHandler<T>::generateUniqueId();
	}

	Asset(AssetId id)
	{
		m_id = id;
	}
	
	// Load asset datas from a file
	virtual void loadFromFile(const FileHandler::CompletePath& filePath)
	{
		saveMetas(filePath);
	}

	virtual void saveMetas(const FileHandler::CompletePath& filePath)
	{
		// By default, create a minimal meta file (i.e : a file which stores the assetId)
		FileHandler::CompletePath assetMetaPath = assetPath;
		assetMetaPath.replaceExtension("meta");
		if (!FileHandler::fileExists(assetMetaPath))
		{
			Json::Value jsonMeta;
			jsonMeta["id"] = m_id;
			Json::FastWriter fastWriter;
			std::string stringMeta = fastWriter.write(root);
			AssetManager::createAssetMetaFile(filePath, stringMeta);
		}
	}

	// Save an asset to file. Only few asset are allowed to be saved in a file (materials, cube maps,...).
	virtual void saveToFile(const FileHandler::CompletePath& filePath)
	{
		saveMetas(filePath);
	}

	// will export the asset in memory to an asset file. Depending on the asset type it can be a .mat (materials), a .cTex (cubeTexture) or a .MeshAsset, .TexAsset, ... (meshes, textures,...)
	//  It could be nice to save mesh to .obj and textures to .bmp by default.
	virtual void createNewAssetFile(const FileHandler::CompletePath& filePath)
	{
		saveMetas(filePath);
	}
};