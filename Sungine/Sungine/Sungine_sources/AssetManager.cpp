#include "AssetManager.h"
#include "AssetTree.h"
#include "Project.h"

#include "EngineMaterials.h"

//#include "Mesh.h"
//#include "Texture.h"
//#include "AnimationStateMachine.h"
//#include "Material.h"
//#include "Materials.h"
//#include "SkeletalAnimation.h"


void AssetManager::registerAssetFileTypes(int objectClassId, const std::vector<FileHandler::FileType>& fileTypes)
{
	for (auto& fileType : fileTypes)
	{
		m_fileTypeToObjectClassId[fileType] = objectClassId;
	}
}

// Specialisations for getPool(), to spped up the accessor.
#define ASSET_POOL_GETTER_SPECIALISATION(Class, name)\
template<>\
inline AssetPool<Class>* AssetManager::getPool<Class>()\
{\
	return &name;\
}

ASSET_POOL_GETTER_SPECIALISATION(Mesh, m_meshPool)
ASSET_POOL_GETTER_SPECIALISATION(Texture, m_texturePool)
ASSET_POOL_GETTER_SPECIALISATION(CubeTexture, m_cubeTexturePool)
ASSET_POOL_GETTER_SPECIALISATION(Sungine::Animation::AnimationStateMachine, m_animationStateMachinePool)
ASSET_POOL_GETTER_SPECIALISATION(Material, m_shaderProgramPool)
ASSET_POOL_GETTER_SPECIALISATION(MaterialInstance, m_materialPool)
ASSET_POOL_GETTER_SPECIALISATION(SkeletalAnimation, m_skeletalAnimationPool)

AssetManager::AssetManager()
{
	// Mesh
	m_meshPool.resize(ASSET_ELEMENT_COUNT);
	m_poolMapping[Object::getStaticClassId<Mesh>()] = &m_meshPool;
	//Texture
	m_texturePool.resize(ASSET_ELEMENT_COUNT);
	m_poolMapping[Object::getStaticClassId<Texture>()] = &m_texturePool;
	//CubeTexture
	m_cubeTexturePool.resize(ASSET_ELEMENT_COUNT);
	m_poolMapping[Object::getStaticClassId<CubeTexture>()] = &m_cubeTexturePool;
	//AnimationStateMachine
	m_animationStateMachinePool.resize(ASSET_ELEMENT_COUNT);
	m_poolMapping[Object::getStaticClassId<Sungine::Animation::AnimationStateMachine>()] = &m_animationStateMachinePool;
	//Material
	m_shaderProgramPool.resize(ASSET_ELEMENT_COUNT);
	m_poolMapping[Object::getStaticClassId<Material>()] = &m_shaderProgramPool;
	//MaterialInstance
	m_materialPool.resize(ASSET_ELEMENT_COUNT);
	m_poolMapping[Object::getStaticClassId<MaterialInstance>()] = &m_materialPool;
	//SkeletalAnimation
	m_skeletalAnimationPool.resize(ASSET_ELEMENT_COUNT);
	m_poolMapping[Object::getStaticClassId<SkeletalAnimation>()] = &m_skeletalAnimationPool;

	// Defaults:
	// Materials :
	// PointLight :

	/*Material matPointLight(FileHandler::CompletePath(Project::getShaderFolderPath().toString() + "pointLight/pointLight.glProg"), Rendering::MaterialType::INTERNAL);
	AssetManager::instance().addDefaultAsset<Material>(matPointLight, "materialPointLight");
	MaterialPointLight matInstancePointLight(AssetManager::instance().getDefaultAsset<Material>("materialPointLight"));
	AssetManager::instance().addDefaultAsset<MaterialInstance>(matInstancePointLight, "materialInstancePointLight");*/

}

Asset* AssetManager::loadSingleAsset(const FileHandler::CompletePath& assetPath)
{
	assert(Project::isPathPointingInsideProjectFolder(assetPath));

	return loadAsset(assetPath.getPath(), assetPath.getFilenameWithExtention());
}

void AssetManager::loadAssets(AssetTree* tree = nullptr)
{
	const FileHandler::Path& assetsPath = Project::getAssetsFolderPath();

	loadAssetsRec(assetsPath, &tree->getAssetFolder());
}

void AssetManager::loadDefaultAssets(AssetTree* tree = nullptr)
{
	const FileHandler::Path& defaultAssetsPath = Project::getDefaultAssetsFolderPath();

	loadAssetsRec(defaultAssetsPath, &tree->getDefaultAssetFolder());
}

// Recurssion called by loadAssets()
void AssetManager::loadAssetsRec(const FileHandler::Path& folderPath, AssetFolder* currentFolder)
{
	std::vector<std::string> dirNames;
	FileHandler::getAllDirNames(folderPath, dirNames);

	for (auto& dirName : dirNames)
	{
		int subFolderIdx = 0;
		if (currentFolder != nullptr)
			currentFolder->addSubFolder(dirName, &subFolderIdx);
		loadAssetsRec(FileHandler::Path(folderPath, dirName), currentFolder->getSubFolder(subFolderIdx));
	}

	std::vector<std::string> fileNames;
	FileHandler::getAllFileNames(folderPath, fileNames);
	Asset* newAsset = nullptr;

	for (auto& fileNameAndExtention : fileNames)
	{
		newAsset = loadAsset(folderPath, fileNameAndExtention);
		if (newAsset != nullptr)
		{
			if(currentFolder != nullptr)
				currentFolder->addFile(fileNameAndExtention, newAsset->getAssetId());
		}
	}
}

Asset* AssetManager::loadAsset(const FileHandler::Path& folderPath, const std::string& assetFilenameAndExtention)
{
	std::string outExtention;
	//We only add files that engine understand
	FileHandler::getExtentionFromExtendedFilename(assetFilenameAndExtention, outExtention);
	FileHandler::FileType fileType = FileHandler::getFileTypeFromExtention(outExtention);
	if (FileHandler::getFileTypeFromExtention(outExtention) != FileHandler::FileType::NONE)
	{
		FileHandler::CompletePath assetPath(folderPath, assetFilenameAndExtention);
		int foundClassId = getClassIdFromFileType(fileType);
		if (foundClassId != -1)
		{
			return m_poolMapping[foundClassId]->loadAsset(assetPath);
		}

		return nullptr;
	}
	return nullptr;
}
