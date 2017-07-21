#include "AssetManager.h"
#include "ResourceTree.h"

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
}

// Recurssion called by loadAssets()
void AssetManager::loadAssetsRec(ResourceFolder& currentFolder, const FileHandler::Path& folderPath)
{
	std::vector<std::string> dirNames;
	FileHandler::getAllDirNames(folderPath, dirNames);

	for (auto& dirName : dirNames)
	{
		int subFolderIdx = 0;
		if (currentFolder.addSubFolder(dirName, &subFolderIdx))
		{
			loadAssetsRec(*currentFolder.getSubFolder(subFolderIdx), FileHandler::Path(folderPath, dirName));
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