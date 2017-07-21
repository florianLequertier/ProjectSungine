#include "Asset.h"


Asset::Asset(int typeId)
{
	m_id = AssetIdHandler::generateUniqueId(typeId);
}

Asset::Asset(AssetId id)
{
	m_id = id;
}

// will export the asset in memory to an asset file. Depending on the asset type it can be a .mat (materials), a .cTex (cubeTexture) or a .MeshAsset, .TexAsset, ... (meshes, textures,...)
//  It could be nice to save mesh to .obj and textures to .bmp by default.
void Asset::createNewAssetFile(const FileHandler::CompletePath& filePath)
{
	saveMetas(filePath);
}

// Load asset datas from a file
void Asset::loadFromFile(const FileHandler::CompletePath& filePath)
{
	loadMetas(filePath);
}

// Save an asset to file. Only few asset are allowed to be saved in a file (materials, cube maps,...).
void Asset::saveToFile(const FileHandler::CompletePath& filePath)
{
	saveMetas(filePath);
}

void Asset::saveMetas(const FileHandler::CompletePath& filePath)
{
	// By default, create a minimal meta file (i.e : a file which stores the assetId)
	FileHandler::CompletePath assetMetaPath = filePath;
	assetMetaPath.replaceExtension("meta");
	if (!FileHandler::fileExists(assetMetaPath))
	{
		Json::Value jsonMeta;
		jsonMeta["id"]["id"] = m_id.id;
		jsonMeta["id"]["type"] = m_id.type;
		Json::FastWriter fastWriter;
		std::string stringMeta = fastWriter.write(jsonMeta);
		AssetManager::createAssetMetaFile(filePath, stringMeta);
	}
}

void Asset::loadMetas(const FileHandler::CompletePath& filePath)
{
	//TODO
}

void Asset::drawInInspector(Scene & scene)
{
	ImGui::Text(getName().c_str());
}

void Asset::drawIconeInResourceTree()
{
	ImGui::Dummy(ImVec2(1, 1));
}

void Asset::drawUIOnHovered()
{
	// Nothing
}

void Asset::drawIconeInResourceField()
{
	ImGui::Dummy(ImVec2(1, 1));
}

void Asset::drawInInspector(const std::vector<void*>& objectInstances)
{
	ImGui::Text("Multiple edition not supported for assets");
}
