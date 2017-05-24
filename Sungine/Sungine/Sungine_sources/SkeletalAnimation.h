#pragma once
#include <map>
#include <utility>

#include <assimp/anim.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Animation.h"
#include "FileHandler.h"
#include "Resource.h"

struct Vec3Key
{
	float time;
	glm::vec3 value;

	Vec3Key(float _time = 0, const glm::vec3& _value = glm::vec3(0, 0, 0))
		: time(_time), value(_value)
	{}
};

struct QuatKey
{
	float time;
	glm::quat value;

	QuatKey(float _time = 0, const glm::quat& _value = glm::quat(0,0,0,0))
		: time(_time), value(_value)
	{}
};

class BoneAnimChannel
{
private:
	std::string m_name;
	std::vector<Vec3Key> m_positionKeys;
	std::vector<QuatKey> m_rotationKeys;
	std::vector<Vec3Key> m_scaleKeys;

public:

	friend void swap(BoneAnimChannel& a, BoneAnimChannel& b)
	{
		using std::swap;

		swap(a.m_name, b.m_name);
		swap(a.m_positionKeys, b.m_positionKeys);
		swap(a.m_rotationKeys, b.m_rotationKeys);
		swap(a.m_scaleKeys, b.m_scaleKeys);
	}

	BoneAnimChannel();
	BoneAnimChannel(const BoneAnimChannel& other);
	BoneAnimChannel(BoneAnimChannel&& other);
	BoneAnimChannel& operator=(BoneAnimChannel other);
	BoneAnimChannel(const aiNodeAnim& nodeAnim);

	bool findPositionKeys(float time, glm::vec3& outCurrentKeyPos, glm::vec3& outNextKeyPos, float& outFactor) const;
	bool findRotationKeys(float time, glm::quat& outCurrentKeyRot, glm::quat& outNextKeyRot, float& outFactor) const;
	bool findScaleKeys(float time, glm::vec3& outCurrentKeyScale, glm::vec3& outNextKeyScale, float& outFactor) const;

	const std::string& getName() const;

	unsigned int getPositionKeyCount() const;
	unsigned int getRotationKeyCount() const;
	unsigned int getScaleKeyCount() const;

	const Vec3Key& getPositionKey(int index) const;
	const QuatKey& getRotationKey(int index) const;
	const Vec3Key& getScaleKey(int index) const;
};

class BonesAnimation
{
private:
	std::string m_name;
	float m_duration;
	float m_ticksPerSecond;
	std::vector<BoneAnimChannel> m_channels;

public:
	friend void swap(BonesAnimation& a, BonesAnimation& b)
	{
		using std::swap;
		swap(a.m_name, b.m_name);
		swap(a.m_duration, b.m_duration);
		swap(a.m_ticksPerSecond, b.m_ticksPerSecond);
		swap(a.m_channels, b.m_channels);
	}

	BonesAnimation();
	BonesAnimation(const BonesAnimation& other);
	BonesAnimation(BonesAnimation&& other);
	BonesAnimation& operator=(BonesAnimation other);
	BonesAnimation(const aiAnimation& other);

	const std::string& getName() const;
	float getDuration() const;
	float getTicksPerSecond() const;

	const BoneAnimChannel* getChannelByName(const std::string& channelName) const;
};


class SkeletalAnimation: public Animation, public Resource
{
	BonesAnimation m_animation; //Carefull : the animation will be deleted when the mesh is deleted
	
public:
	friend void swap(SkeletalAnimation& a, SkeletalAnimation& b)
	{
		using std::swap;
		swap(a.m_animation, b.m_animation);
	}

	SkeletalAnimation();
	SkeletalAnimation(const SkeletalAnimation& other);
	SkeletalAnimation(SkeletalAnimation&& other);
	SkeletalAnimation& operator=(SkeletalAnimation other);
	SkeletalAnimation(aiAnimation* animation);
	~SkeletalAnimation();

	// Inherit from Resource
	void init(const FileHandler::CompletePath& path, const ID& id) override;
	void save() override;
	void resolvePointersLoading() override;

	float getDurationInTick() const;
	float getAnimationTime() const;
	void setIsLooping(bool isLooping);
	std::string getName() const;
	float getTicksPerSecond() const;
	const BoneAnimChannel* getNodeAnim(const std::string& nodeName) const;
};

//animations attached to a mesh
struct MeshAnimations
{
private:
	std::map<std::string, SkeletalAnimation> m_animations;

public:
	MeshAnimations(const FileHandler::CompletePath& scenePath);
	bool contains(const std::string& name);
	SkeletalAnimation& get(const std::string& name);
};

