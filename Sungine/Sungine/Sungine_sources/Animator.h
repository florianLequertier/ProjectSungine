#pragma once

#include "Component.h"
#include "SkeletalAnimation.h"
#include "Skeleton.h"
#include "ResourcePointer.h"

class Animator: public Component
{
	COMPONENT_IMPLEMENTATION_HEADER(Animator)

	OBJECT_CLASS(Animator,
		ObjectDescriptor<Animator>::registerParentClass<Component>();
		ObjectDescriptor<Animator>::registerProperty<FileHandler::CompletePath>(&Animator::m_skeletonPath)
	)

private:
	FileHandler::CompletePath m_skeletonPath; //TODO : J'aime pas ça, faire en sorte que le skeleton soit une sous-resource ce serai mieux
	Skeleton* m_skeleton;
	int m_currentAnimIdx;
	std::vector<ResourcePtr<SkeletalAnimation>> m_skeletonAnimations;
	bool m_isPlaying;

	//for ui : 
	std::string m_skeletonName;
	std::string m_animationName;

public:
	Animator();
	~Animator();
	
	void setSkeleton(Skeleton* skeleton);
	void addAnimation(ResourcePtr<SkeletalAnimation> animation);
	void removeAnimation(SkeletalAnimation* animation);
	void updateAnimations(float timeInSecond);

	void play();
	void play(const std::string& animationName);
	void play(int animationIdx);

	virtual void drawInInspector(Scene & scene) override;
	virtual void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	virtual void save(Json::Value& componentRoot) const override;
	virtual void load(const Json::Value& componentRoot) override;
};

REGISTER_CLASS(Animator)
