
#include "Skeleton.h"
//forwards : 
#include "Utils.h"



Skeleton::Skeleton(const aiMesh* pMesh, const aiNode* rootNode, unsigned int firstVertexId)
{
	m_rootInverseTransform = assimpMat4ToglmMat4(rootNode->mTransformation);
	glm::inverse(m_rootInverseTransform);

	loadBones(pMesh, rootNode, firstVertexId);
}

Skeleton::~Skeleton()
{
}

unsigned int Skeleton::getBoneCount() const
{
	return m_bones.size();
}

//const std::vector<aiMatrix4x4>& Skeleton::getBonesOffset() const
//{
//	return m_bonesOffset;
//}

const std::vector<glm::mat4>& Skeleton::getBonesTransform() const
{
	return m_bonesTransform;
}

const glm::mat4 & Skeleton::getBoneTransform(int i) const
{
	assert(i >= 0 && i < m_bonesTransform.size());
	return m_bonesTransform[i];
}

const std::vector<VertexBoneData>& Skeleton::getBoneDatas() const
{
	return m_boneDatas;
}

void Skeleton::playAnimationStep(float timeInSecond, const SkeletalAnimation& animation)
{
	float animationTime = animation.getAnimationTime();

	glm::mat4 identity(1);
	readNodeHierarchy(animationTime, animation, 0, identity);
}

//void Skeleton::loadBones(const aiMesh* pMesh, unsigned int firstVertexId)
//{
//	m_bonesTransform.resize(pMesh->mNumBones, glm::mat4(1));
//	m_bonesOffset.resize(pMesh->mNumBones);
//
//	for (unsigned int i = 0; i < pMesh->mNumBones; i++) 
//	{
//		std::string boneName = pMesh->mBones[i]->mName.data;
//		unsigned int boneIndex = 0;
//
//		if (m_boneMapping.find(boneName) == m_boneMapping.end()) 
//		{
//			boneIndex = m_boneCount++;
//
//			m_boneMapping[boneName] = boneIndex;
//			m_bonesOffset[boneIndex] = pMesh->mBones[i]->mOffsetMatrix;
//		}
//		else 
//		{
//			boneIndex = m_boneMapping[boneName];
//			m_bonesOffset[boneIndex] = pMesh->mBones[i]->mOffsetMatrix;
//		}
//
//		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; j++) 
//		{
//			float weight = pMesh->mBones[i]->mWeights[j].mWeight;
//			if (weight == 0)
//				continue;
//
//			unsigned int vertexId = pMesh->mBones[i]->mWeights[j].mVertexId + firstVertexId;
//
//			if (vertexId + 1 > m_boneDatas.size())
//				m_boneDatas.resize(vertexId + 1);
//
//			for (int k = 0; k < MAX_BONE_DATA_PER_VERTEX; k++) 
//			{
//				if (m_boneDatas[vertexId].weights[k] == 0) 
//				{
//					m_boneDatas[vertexId].ids[k] = boneIndex;
//					m_boneDatas[vertexId].weights[k] = weight;
//					break;
//				}
//			}
//		}
//	}
//}

void Skeleton::loadBones(const aiMesh* pMesh, const aiNode* rootNode, unsigned int firstVertexId)
{
	m_bonesTransform.resize(pMesh->mNumBones, glm::mat4(1));
	m_bones.resize(pMesh->mNumBones);

	// The mapping assure we are nore dupplicating bones
	std::map<std::string, unsigned int> tmpBoneMapping;
	int processedBoneCount = 0;

	for (unsigned int i = 0; i < pMesh->mNumBones; i++)
	{
		std::string boneName = pMesh->mBones[i]->mName.data;
		unsigned int boneIndex = 0;

		// If a previous processed bone had the same name as the bone we are currently processing, we retrieve it's id instread of duplicating the id.
		auto foundBoneIdIter = tmpBoneMapping.find(boneName);
		if (foundBoneIdIter == tmpBoneMapping.end())
		{
			boneIndex = processedBoneCount++;
			tmpBoneMapping[boneName] = boneIndex;
		}
		else
			boneIndex = foundBoneIdIter->second;

		// We create the new bone with the right informations
		m_bones.push_back(Bone(boneName, boneIndex, assimpMat4ToglmMat4(pMesh->mBones[i]->mOffsetMatrix)));

		// Populate boneDatas infos at vertex index where vertex are influenced by the new processing bone.
		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; j++)
		{
			float weight = pMesh->mBones[i]->mWeights[j].mWeight;
			if (weight == 0)
				continue;

			unsigned int vertexId = pMesh->mBones[i]->mWeights[j].mVertexId + firstVertexId;

			if (vertexId + 1 > m_boneDatas.size())
				m_boneDatas.resize(vertexId + 1);

			// Find the first enplacement with weight == 0, then write the new weight and the corresponding bone index.
			for (int k = 0; k < MAX_BONE_DATA_PER_VERTEX; k++)
			{
				if (m_boneDatas[vertexId].weights[k] == 0)
				{
					m_boneDatas[vertexId].ids[k] = boneIndex;
					m_boneDatas[vertexId].weights[k] = weight;
					break;
				}
			}
		}
	}

	// Setup child hierarchy for each bone.
	setupBoneHierarchy(*rootNode, tmpBoneMapping);
}

void Skeleton::setupBoneHierarchy(const aiNode& node, const std::map<std::string, unsigned int>& boneMapping)
{
	const std::string nodeName = node.mName.data;

	// Try to find if the current node is a valid bone for this skeleton.
	auto foundBoneIter = boneMapping.find(nodeName);
	if (foundBoneIter != boneMapping.end())
	{
		// Take care that we have an empty list of child before populate it.
		m_bones[foundBoneIter->second].clearChildren();

		// For each node child.
		for (int i = 0; i < node.mNumChildren; i++)
		{
			const aiNode& child = *node.mChildren[i];
			const std::string childName = child.mName.data;

			// Try to find if the child node is a valid bone for this skeleton.
			auto foundChildrenBoneIter = boneMapping.find(childName);
			if (foundChildrenBoneIter != boneMapping.end())
			{
				// If it is a bone, we add it as the child of the bone, and launch recursivity.
				m_bones[foundBoneIter->second].addChild(foundChildrenBoneIter->second);
				setupBoneHierarchy(child, boneMapping);
			}
		}
	}
}

unsigned int Skeleton::findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


unsigned int Skeleton::findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


unsigned int Skeleton::findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
		{
			return i;
		}
	}

	assert(0);

	return 0;
}


void Skeleton::computeInterpolatedPosition(glm::vec3& outTranslation, float animationTime, const BoneAnimChannel* nodeAnim)
{
	// Trivial case
	if (nodeAnim->getPositionKeyCount() == 1)
	{
		outTranslation = nodeAnim->getPositionKey(0).value;
		return;
	}

	glm::vec3 start, end;
	float factor = 0;
	nodeAnim->findPositionKeys(animationTime, start, end, factor);
	assert(factor >= 0.0f && factor <= 1.0f);

	outTranslation = start + factor * (end - start);
}


void Skeleton::computeInterpolatedRotation(glm::quat& outRotation, float animationTime, const BoneAnimChannel* nodeAnim)
{
	// Trivial case
	if (nodeAnim->getRotationKeyCount() == 1)
	{
		outRotation = nodeAnim->getRotationKey(0).value;
		return;
	}

	glm::quat start, end;
	float factor = 0;
	nodeAnim->findRotationKeys(animationTime, start, end, factor);
	assert(factor >= 0.0f && factor <= 1.0f);

	// Interpolate using the factor
	outRotation = glm::lerp(start, end, factor);

	//aiQuaternion::Interpolate(OutRotation, StartRotationQ, EndRotationQ, Factor);
	//OutRotation = OutRotation.Normalize();
}


void Skeleton::computeInterpolatedScaling(glm::vec3& outScaling, float animationTime, const BoneAnimChannel* nodeAnim)
{
	// Trivial case
	if (nodeAnim->getScaleKeyCount() == 1)
	{
		outScaling = nodeAnim->getScaleKey(0).value;
		return;
	}

	glm::vec3 start, end;
	float factor = 0;
	nodeAnim->findScaleKeys(animationTime, start, end, factor);
	assert(factor >= 0.0f && factor <= 1.0f);	

	// Interpolate using the factor
	outScaling = start + factor * (end - start);
}

void Skeleton::readNodeHierarchy(float animationTime, const SkeletalAnimation& animation, const unsigned int currentBoneIdx, const glm::mat4& parentTransform)
{
	Bone* currentBone = &m_bones[currentBoneIdx];
	const std::string& nodeName = currentBone->getName();
	const BoneAnimChannel* nodeAnim = animation.getNodeAnim(nodeName);
	glm::mat4 nodeTransformation = m_bonesTransform[currentBoneIdx];

	if (nodeAnim)
	{
		glm::vec3 scaling;
		computeInterpolatedScaling(scaling, animationTime, nodeAnim);

		glm::quat rotationQ;
		computeInterpolatedRotation(rotationQ, animationTime, nodeAnim);

		glm::vec3 translation;
		computeInterpolatedPosition(translation, animationTime, nodeAnim);

		nodeTransformation = glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotationQ) * glm::scale(glm::mat4(1), scaling);
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransformation;

	m_bonesTransform[currentBoneIdx] = m_rootInverseTransform * globalTransformation * currentBone->getOffset();

	for (auto it = currentBone->getChildrenIdxBegin(); it != currentBone->getChildrenIdxEnd(); it++)
	{
		readNodeHierarchy(animationTime, animation, *it, globalTransformation);
	}
}
