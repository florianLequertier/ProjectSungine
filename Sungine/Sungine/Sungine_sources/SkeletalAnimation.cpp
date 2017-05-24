

#include "SkeletalAnimation.h"
//forwards :
#include "Application.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

BoneAnimChannel::BoneAnimChannel()
	: m_name("")
{

}

BoneAnimChannel::BoneAnimChannel(const BoneAnimChannel& other)
	: m_name(other.m_name)
{
	m_positionKeys.resize(other.m_positionKeys.size());
	for (int i = 0; i < other.m_positionKeys.size(); i++)
	{
		m_positionKeys[i] = other.m_positionKeys[i];
	}
	m_rotationKeys.resize(other.m_rotationKeys.size());
	for (int i = 0; i < other.m_rotationKeys.size(); i++)
	{
		m_rotationKeys[i] = other.m_rotationKeys[i];
	}
	m_scaleKeys.resize(other.m_scaleKeys.size());
	for (int i = 0; i < other.m_scaleKeys.size(); i++)
	{
		m_scaleKeys[i] = other.m_scaleKeys[i];
	}
}

BoneAnimChannel::BoneAnimChannel(BoneAnimChannel&& other)
	: BoneAnimChannel()
{
	swap(*this, other);
}

BoneAnimChannel& BoneAnimChannel::operator=(BoneAnimChannel other)
{
	swap(*this, other);
	return *this;
}


BoneAnimChannel::BoneAnimChannel(const aiNodeAnim& nodeAnim)
{
	m_name = nodeAnim.mNodeName.data;

	m_positionKeys.resize(nodeAnim.mNumPositionKeys);
	for (int i = 0; i < nodeAnim.mNumPositionKeys; i++)
	{
		aiVectorKey& currentKey = nodeAnim.mPositionKeys[i];
		m_positionKeys[i] = Vec3Key(currentKey.mTime, glm::vec3(currentKey.mValue.x, currentKey.mValue.y, currentKey.mValue.z));
	}
	m_rotationKeys.resize(nodeAnim.mNumRotationKeys);
	for (int i = 0; i < nodeAnim.mNumRotationKeys; i++)
	{
		aiQuatKey& currentKey = nodeAnim.mRotationKeys[i];
		m_rotationKeys[i] = QuatKey(currentKey.mTime, glm::quat(currentKey.mValue.w, currentKey.mValue.x, currentKey.mValue.y, currentKey.mValue.z));
	}
	m_scaleKeys.resize(nodeAnim.mNumScalingKeys);
	for (int i = 0; i < nodeAnim.mNumScalingKeys; i++)
	{
		aiVectorKey& currentKey = nodeAnim.mScalingKeys[i];
		m_scaleKeys[i] = Vec3Key(currentKey.mTime, glm::vec3(currentKey.mValue.x, currentKey.mValue.y, currentKey.mValue.z));
	}
}

bool BoneAnimChannel::findPositionKeys(float time, glm::vec3& outCurrentKeyPos, glm::vec3& outNextKeyPos, float& outFactor) const
{
	for (unsigned int i = 0; i < m_positionKeys.size() - 1; i++)
	{
		if (time < m_positionKeys[i + 1].time)
		{
			outCurrentKeyPos = m_positionKeys[i].value;
			outNextKeyPos = m_positionKeys[i + 1].value;

			const float deltaTime = m_positionKeys[i + 1].time - m_positionKeys[i].time;
			outFactor = (time - m_positionKeys[i].time) / deltaTime;

			return true;
		}
	}
	return false;
}

bool BoneAnimChannel::findRotationKeys(float time, glm::quat& outCurrentKeyRot, glm::quat& outNextKeyRot, float& outFactor) const
{
	for (unsigned int i = 0; i < m_rotationKeys.size() - 1; i++)
	{
		if (time < m_rotationKeys[i + 1].time)
		{
			outCurrentKeyRot = m_rotationKeys[i].value;
			outNextKeyRot = m_rotationKeys[i + 1].value;

			const float deltaTime = m_rotationKeys[i + 1].time - m_rotationKeys[i].time;
			outFactor = (time - m_rotationKeys[i].time) / deltaTime;

			return true;
		}
	}
	return false;
}

bool BoneAnimChannel::findScaleKeys(float time, glm::vec3& outCurrentKeyScale, glm::vec3& outNextKeyScale, float& outFactor) const
{
	for (unsigned int i = 0; i < m_scaleKeys.size() - 1; i++)
	{
		if (time < m_scaleKeys[i + 1].time)
		{
			outCurrentKeyScale = m_scaleKeys[i].value;
			outNextKeyScale = m_scaleKeys[i + 1].value;

			const float deltaTime = m_scaleKeys[i + 1].time - m_scaleKeys[i].time;
			outFactor = (time - m_scaleKeys[i].time) / deltaTime;

			return true;
		}
	}
	return false;
}

const std::string & BoneAnimChannel::getName() const
{
	return m_name;
}

unsigned int BoneAnimChannel::getPositionKeyCount() const
{
	return m_positionKeys.size();
}

unsigned int BoneAnimChannel::getRotationKeyCount() const
{
	return m_rotationKeys.size();
}

unsigned int BoneAnimChannel::getScaleKeyCount() const
{
	return m_scaleKeys.size();
}

const Vec3Key & BoneAnimChannel::getPositionKey(int index) const
{
	return m_positionKeys[index];
}

const QuatKey & BoneAnimChannel::getRotationKey(int index) const
{
	return m_rotationKeys[index];
}

const Vec3Key & BoneAnimChannel::getScaleKey(int index) const
{
	return m_scaleKeys[index];
}

////////////////////////////////////////////

BonesAnimation::BonesAnimation()
	: m_name("")
	, m_duration(0)
	, m_ticksPerSecond(0)
{

}

BonesAnimation::BonesAnimation(const BonesAnimation & other)
	: m_name(other.m_name)
	, m_duration(other.m_duration)
	, m_ticksPerSecond(other.m_ticksPerSecond)
{
	m_channels.resize(other.m_channels.size());
	for (int i = 0; i < other.m_channels.size(); i++)
	{
		m_channels[i] = other.m_channels[i];
	}
}

BonesAnimation::BonesAnimation(const aiAnimation& other)
{
	m_name = other.mName.data;
	m_duration = other.mDuration;
	m_ticksPerSecond = other.mTicksPerSecond;

	m_channels.resize(other.mNumChannels);
	for (int i = 0; i < other.mNumChannels; i++)
	{
		m_channels[i] = *other.mChannels[i];
	}
}

const std::string & BonesAnimation::getName() const
{
	return m_name;
}

float BonesAnimation::getDuration() const
{
	return m_duration;
}

float BonesAnimation::getTicksPerSecond() const
{
	return m_ticksPerSecond;
}

const BoneAnimChannel* BonesAnimation::getChannelByName(const std::string & channelName) const
{
	for (int i = 0; i < m_channels.size(); i++)
	{
		if (channelName == m_channels[i].getName())
			return &m_channels[i];
	}
	return nullptr;
}

BonesAnimation& BonesAnimation::operator=(BonesAnimation other)
{
	swap(*this, other);

	return *this;
}

BonesAnimation::BonesAnimation(BonesAnimation&& other)
	: BonesAnimation()
{
	swap(*this, other);
}

////////////////////////////////////////////

//MESh ANIMATION
MeshAnimations::MeshAnimations(const FileHandler::CompletePath& scenePath)
{
	Assimp::Importer* importer = new Assimp::Importer();
	const aiScene* pScene = importer->ReadFile(scenePath.c_str(), 0); //no flags

	for (int i = 0; i < pScene->mNumAnimations; i++)
	{
		const std::string animName = pScene->mAnimations[i]->mName.data;
		m_animations[animName] = SkeletalAnimation(pScene->mAnimations[i]);
	}
}

bool MeshAnimations::contains(const std::string& name)
{
	return m_animations.find(name) != m_animations.end();
}

SkeletalAnimation& MeshAnimations::get(const std::string& name)
{
	return m_animations[name];
}


//SKELETAL ANIMATION

SkeletalAnimation::SkeletalAnimation()
	: Animation(0.0, true)
	, m_animation()
{

}

SkeletalAnimation::SkeletalAnimation(const SkeletalAnimation & other)
	: m_animation(other.m_animation)
{
}

SkeletalAnimation::SkeletalAnimation(SkeletalAnimation && other)
	: SkeletalAnimation()
{
	swap(*this, other);
}

SkeletalAnimation & SkeletalAnimation::operator=(SkeletalAnimation other)
{
	swap(*this, other);
	return *this;
}

SkeletalAnimation::SkeletalAnimation(aiAnimation* animation)
	: Animation(0.0, true)
	, m_animation(*animation)
{
	float ticksPerSecond = getTicksPerSecond() == 0 ? 25 : getTicksPerSecond();
	m_duration = getDurationInTick() / ticksPerSecond;
}

SkeletalAnimation::~SkeletalAnimation()
{
}

void SkeletalAnimation::init(const FileHandler::CompletePath& path, const ID& id)
{
	Resource::init(path, id);

	const std::string animName = path.getSubFileName();
	assert(!animName.empty());

	//bool Ret = false;
	//if (importer != nullptr)
	//	delete importer;
	Assimp::Importer* importer = new Assimp::Importer();

	const aiScene* pScene = importer->ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (pScene->HasAnimations())
	{
		//TODO : meilleur recherche
		for (int i = 0; i < pScene->mNumAnimations; i++)
		{
			if (animName.compare(pScene->mAnimations[i]->mName.C_Str()) == 0)
			{
				m_animation = *pScene->mAnimations[i];
			}
		}
	}

	delete importer;
}

void SkeletalAnimation::save()
{
	assert(false && "TODO");
}

void SkeletalAnimation::resolvePointersLoading()
{
	//No pointers;
}

std::string SkeletalAnimation::getName() const
{
	return m_animation.getName();
}

float SkeletalAnimation::getTicksPerSecond() const
{
	return m_animation.getTicksPerSecond();
}

const BoneAnimChannel* SkeletalAnimation::getNodeAnim(const std::string& nodeName) const
{
	return m_animation.getChannelByName(nodeName);
}

float SkeletalAnimation::getAnimationTime() const
{
	float ticksPerSecond = getTicksPerSecond() != 0 ? getTicksPerSecond() : 25.0f;
	float timeInTicks = getElapsedTime() * ticksPerSecond;
	return fmod(timeInTicks, getDurationInTick()); //note : the fmod is normally not necessary since getElapsedTime already loop the time of the animation.
}

float SkeletalAnimation::getDurationInTick() const
{
	return m_animation.getDuration();
}

void SkeletalAnimation::setIsLooping(bool isLooping)
{
	m_isLooping = isLooping;
}
