#include "Asset.h"
#include "AssetManager.h"

Asset::Asset(int typeId)
	: m_id(AssetIdHandler::generateUniqueId(typeId))
{
}

Asset::Asset(AssetId id)
	: m_id(id)
{
}

// will export the asset in memory to an asset file. Depending on the asset type it can be a .mat (materials), a .cTex (cubeTexture) or a .MeshAsset, .TexAsset, ... (meshes, textures,...)
//  It could be nice to save mesh to .obj and textures to .bmp by default.
void Asset::createNewAssetFile(const FileHandler::CompletePath& filePath)
{
	saveToFile();
}

// Load asset datas from a file
void Asset::loadFromFile(const FileHandler::CompletePath& filePath)
{
	// Restore path
	m_assetPath = filePath;
	// Load metas, will at least restore the assetId
	loadMetas();
}

// Save an asset to file. Only few asset are allowed to be saved in a file (materials, cube maps,...).
void Asset::saveToFile()
{
	saveMetas();
}

void Asset::saveMetas()
{
	// By default, create a minimal meta file (i.e : a file which stores the assetId)
	FileHandler::CompletePath assetMetaPath = m_assetPath;
	assetMetaPath.replaceExtension("meta");
	if (!FileHandler::fileExists(assetMetaPath))
	{
		Json::Value jsonMeta;
		m_id.save(jsonMeta["assetId"]);
		Json::FastWriter fastWriter;
		std::string stringMeta = fastWriter.write(jsonMeta);
		AssetManager::instance().createAssetMetaFile(m_assetPath, stringMeta);
	}
}

void Asset::loadMetas()
{
	// Will restaure the assetId
	FileHandler::CompletePath assetMetaPath = m_assetPath;
	assetMetaPath.replaceExtension("meta");
	std::ifstream stream;
	if (Utils::OpenFileStreamRead(assetMetaPath, stream))
	{
		Json::Value root;
		stream >> root;
		m_id.load(root["assetId"]);
	}
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
