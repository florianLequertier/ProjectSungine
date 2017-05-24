#pragma once

#include <vector>
#include <map>

#include "glm/glm.hpp"
#include "glm/common.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "SkeletalAnimation.h"

static const unsigned int MAX_BONE_DATA_PER_VERTEX = 4;

struct BoneInfo {
	//TODO
};

struct VertexBoneData
{
	unsigned int ids[MAX_BONE_DATA_PER_VERTEX];
	float weights[MAX_BONE_DATA_PER_VERTEX];

	inline VertexBoneData() 
	{
		for (int i = 0; i < MAX_BONE_DATA_PER_VERTEX; i++) 
		{
			ids[i] = 0;
			weights[i] = 0;
		}
	}
};

class Bone
{
private:
	// index of the bone in Skeleton::m_bonesTransform and Skeleton::m_bones
	unsigned int m_index;
	std::string m_name;
	glm::mat4 m_offset;
	std::vector<unsigned int> m_childrenIdx;

public:

	Bone(const std::string& name = "", unsigned int index = 0, const glm::mat4& offset = glm::mat4(1))
		: m_index(index)
		, m_name(name)
		, m_offset(offset)
	{}

	void clearChildren()
	{
		m_childrenIdx.clear();
	}

	void addChild(unsigned int idx)
	{
		m_childrenIdx.push_back(idx);
	}

	const std::string& getName() const
	{
		return m_name;
	}

	const unsigned int getIndex() const
	{
		return m_index;
	}

	const glm::mat4& getOffset() const
	{
		return m_offset;
	}

	std::vector<unsigned int>::const_iterator getChildrenIdxBegin() const
	{
		return m_childrenIdx.begin();
	}

	std::vector<unsigned int>::const_iterator getChildrenIdxEnd() const
	{
		return m_childrenIdx.end();
	}
};

class Skeleton
{
	//unsigned int m_boneCount;
	//std::map<std::string, unsigned int> m_boneMapping;
	//std::vector<aiMatrix4x4> m_bonesOffset;

	std::vector<Bone> m_bones; // Bone at idx 0 is the root bone
	std::vector<glm::mat4> m_bonesTransform;
	std::vector<VertexBoneData> m_boneDatas; //index = vertex id
	glm::mat4 m_rootInverseTransform;

public:
	Skeleton(const aiMesh* pMesh, const aiNode* rootNode, unsigned int firstVertexId);
	~Skeleton();

	unsigned int getBoneCount() const;
	//const std::vector<aiMatrix4x4>& getBonesOffset() const;
	const std::vector<glm::mat4>& getBonesTransform() const;
	const glm::mat4& getBoneTransform(int i) const;
	const std::vector<VertexBoneData>& getBoneDatas() const;

	void playAnimationStep(float timeInSecond, const SkeletalAnimation& animation);
	void loadBones(const aiMesh* pMesh, const aiNode* rootNode, unsigned int firstVertexId);

private:
	void readNodeHierarchy(float animationTime, const SkeletalAnimation& animation, const unsigned int currentBoneIdx, const glm::mat4& parentTransform);
	void setupBoneHierarchy(const aiNode& node, const std::map<std::string, unsigned int>& boneMapping);

	unsigned int findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	void computeInterpolatedPosition(glm::vec3& outTranslation, float animationTime, const BoneAnimChannel* nodeAnim);
	void computeInterpolatedRotation(glm::quat& outRotation, float animationTime, const BoneAnimChannel* nodeAnim);
	void computeInterpolatedScaling(glm::vec3& outScale, float animationTime, const BoneAnimChannel* nodeAnim);
};