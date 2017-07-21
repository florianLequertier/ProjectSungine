#pragma once

#include "Object.h"
#include "FileHandler.h"

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
class AssetIdHandler
{
private:
	static int m_assetLastId = 0;

public:
	static AssetId generateUniqueId(int typeId)
	{
		AssetId newId;
		newId.id = ++m_assetLastId;
		newId.type = typeId;
		return newId;
	}

	static int lastGeneratedId()
	{
		return m_assetLastId;
	}
};

// All assets must have a unique id, can be serialisable, and have a meta file which store at least the unique asset id.
class Asset : public Object
{
	friend class AssetHanler;

private:
	// A unique number which identify this asset
	AssetId m_id;

public:

	Asset(int typeId);
	Asset(AssetId id);

	// will export the asset in memory to an asset file. Depending on the asset type it can be a .mat (materials), a .cTex (cubeTexture) or a .MeshAsset, .TexAsset, ... (meshes, textures,...)
	//  It could be nice to save mesh to .obj and textures to .bmp by default.
	virtual void createNewAssetFile(const FileHandler::CompletePath& filePath);
	// Load asset datas from a file
	virtual void loadFromFile(const FileHandler::CompletePath& filePath);
	// Save an asset to file. Only few asset are allowed to be saved in a file (materials, cube maps,...).
	virtual void saveToFile(const FileHandler::CompletePath& filePath);
	// Only save metas. Create the file if it doesn't exists
	virtual void saveMetas(const FileHandler::CompletePath& filePath);
	// Only load metas
	virtual void loadMetas(const FileHandler::CompletePath& filePath);

	virtual void drawIconeInResourceTree();
	virtual void drawUIOnHovered();
	virtual void drawIconeInResourceField();
	virtual bool drawRightClicContextMenu(std::string& popupToOpen) { return false; };

	virtual void drawInInspector(const std::vector<void*>& objectInstances) override;

};