

#include "Animator.h"
//forwards : 
#include "Scene.h"
#include "SceneAccessor.h"
#include "Factories.h"

#include "EditorGUI.h"

COMPONENT_IMPLEMENTATION_CPP(Animator)

Animator::Animator()
	: Component(ComponentType::ANIMATOR)
	, m_skeleton(nullptr)
	, m_currentAnimIdx(0)
	, m_isPlaying(false)
{
}


Animator::~Animator()
{
}

void Animator::setSkeleton(Skeleton* skeleton)
{
	m_skeleton = skeleton;
}

void Animator::addAnimation(ResourcePtr<SkeletalAnimation> animation)
{
	m_skeletonAnimations.push_back(animation);
}

void Animator::removeAnimation(SkeletalAnimation * animation)
{
	auto findIt = std::find_if(m_skeletonAnimations.begin(), m_skeletonAnimations.end(), [&animation](const ResourcePtr<SkeletalAnimation>& resource) { return resource.get() == animation; });
	m_skeletonAnimations.erase(findIt);
	//m_animations.erase(std::remove(m_animations.begin(), m_animations.end(), animation), m_animations.end());
}

void Animator::updateAnimations(float timeInSecond)
{
	if (m_isPlaying && m_skeleton != nullptr && m_currentAnimIdx >= 0 && m_currentAnimIdx < m_skeletonAnimations.size())
		m_skeleton->playAnimationStep(timeInSecond, *m_skeletonAnimations[m_currentAnimIdx]);
}

void Animator::play()
{
	m_isPlaying = true;
}

void Animator::play(const std::string& animationName)
{
	auto& foundIt = std::find_if(m_skeletonAnimations.begin(), m_skeletonAnimations.end(), [animationName](const ResourcePtr<SkeletalAnimation>& it) { return it->getName() == animationName; });
	if ( foundIt != m_skeletonAnimations.end() )
	{
		m_isPlaying = true;
		m_currentAnimIdx = foundIt - m_skeletonAnimations.begin();
	}
}

void Animator::play(int animationIdx)
{
	if (animationIdx >= 0 && animationIdx < m_skeletonAnimations.size())
	{
		m_isPlaying = true;
		m_currentAnimIdx = animationIdx;
	}
}

void Animator::drawInInspector(Scene & scene)
{
	char tmpSkeletalPath[100];
	m_skeletonName.copy(tmpSkeletalPath, m_skeletonName.size());
	tmpSkeletalPath[m_skeletonName.size()] = '\0';

	//%NOCOMMIT%
	//if (ImGui::InputText("skeleton/mesh name", tmpSkeletalName, 20)) {

	//	m_skeletonName = tmpSkeletalName;

	//	if (getMeshFactory().contains(m_skeletonName)) {
	//		ResourcePtr<Mesh> tmpMesh = getMeshFactory().get(m_skeletonName);
	//		if (tmpMesh.isValid()) {
	//			m_currentSkeletonName = m_skeletonName;
	//			m_skeleton = tmpMesh->getSkeleton();
	//		}
	//	}
	//}

	//Get mesh skeleton from mesh
	ResourcePtr<Mesh> meshPtrQuery;
	//EditorGUI::ResourceField<Mesh>(meshPtrQuery, "skeleton/mesh name", tmpSkeletalPath, 100);
	EditorGUI::ResourceField<Mesh>("skeleton/mesh name", meshPtrQuery);
	if (meshPtrQuery.isValid())
	{
		m_skeleton = meshPtrQuery->getSkeleton();
		m_skeletonPath = meshPtrQuery->getCompletePath();// ::CompletePath(tmpSkeletalPath);
	}


	char tmpAnimationName[60];
	m_animationName.copy(tmpAnimationName, m_animationName.size());
	tmpAnimationName[m_animationName.size()] = '\0';

	//Get animation
	ResourcePtr<SkeletalAnimation> animationPtrQuery;
	//EditorGUI::ResourceField<SkeletalAnimation>(animationPtrQuery, "animation", tmpAnimationName, 60);
	EditorGUI::ResourceField<SkeletalAnimation>("animation", animationPtrQuery);

	ImGui::SameLine();
	if (ImGui::Button("add")){
		if (animationPtrQuery.isValid())
		{
			m_skeletonAnimations.push_back(animationPtrQuery);
		}
	}

	int imguiId = 0;
	for (int i = 0; i < m_skeletonAnimations.size(); i++) 
	{
		ImGui::PushID(imguiId);

		ImGui::Text(m_skeletonAnimations[i]->getCompletePath().c_str());
		ImGui::SameLine();
		if (ImGui::Button("remove")) 
		{
			if (m_currentAnimIdx == imguiId)
				m_currentAnimIdx = 0;
			m_skeletonAnimations.erase(m_skeletonAnimations.begin() + i);
		}

		ImGui::PopID();
		imguiId++;
	}
}

void Animator::drawInInspector(Scene& scene, const std::vector<Component*>& components)
{
	//char tmpSkeletalPath[100];
	//m_skeletonName.copy(tmpSkeletalPath, m_skeletonName.size());
	//tmpSkeletalPath[m_skeletonName.size()] = '\0';

	//%NOCOMMIT%
	//if (ImGui::InputText("skeleton/mesh name", tmpSkeletalName, 20)) {

	//	m_skeletonName = tmpSkeletalName;

	//	if (getMeshFactory().contains(m_skeletonName)) {
	//		ResourcePtr<Mesh> tmpMesh = getMeshFactory().get(m_skeletonName);
	//		if (tmpMesh.isValid()) {
	//			m_currentSkeletonName = m_skeletonName;
	//			m_skeleton = tmpMesh->getSkeleton();
	//		}
	//	}
	//}

	//Get mesh skeleton from mesh
	ResourcePtr<Mesh> meshPtrQuery;
	EditorGUI::ResourceField<Mesh>("skeleton/mesh name", meshPtrQuery);
	if (meshPtrQuery.isValid())
	{
		for (auto component : components)
		{
			Animator* castedComponent = static_cast<Animator*>(component);

			castedComponent->m_skeleton = meshPtrQuery->getSkeleton();
			castedComponent->m_skeletonPath = meshPtrQuery->getCompletePath();// ::CompletePath(tmpSkeletalPath);
		}
	}


	//char tmpAnimationName[60];
	//m_animationName.copy(tmpAnimationName, m_animationName.size());
	//tmpAnimationName[m_animationName.size()] = '\0';

	//Get animation
	ResourcePtr<SkeletalAnimation> animationPtrQuery;
	//EditorGUI::ResourceField<SkeletalAnimation>(animationPtrQuery, "animation", tmpAnimationName, 60);
	EditorGUI::ResourceField<SkeletalAnimation>("animation", animationPtrQuery);
	ImGui::SameLine();
	if (ImGui::Button("add")) {
		if (animationPtrQuery.isValid())
		{
			for (auto component : components)
			{
				Animator* castedComponent = static_cast<Animator*>(component);
				castedComponent->m_skeletonAnimations.push_back(animationPtrQuery);
			}
		}
	}

	int imguiId = 0;
	for (int i = 0; i < m_skeletonAnimations.size(); i++)
	{
		ImGui::PushID(imguiId);

		ImGui::Text(m_skeletonAnimations[i]->getCompletePath().c_str());
		ImGui::SameLine();
		if (ImGui::Button("remove"))
		{
			for (auto component : components)
			{
				Animator* castedComponent = static_cast<Animator*>(component);
				if (castedComponent->m_currentAnimIdx == imguiId)
					castedComponent->m_currentAnimIdx = 0;
				castedComponent->m_skeletonAnimations.erase(castedComponent->m_skeletonAnimations.begin() + i);
			}
		}

		ImGui::PopID();
		imguiId++;
	}
}

void Animator::save(Json::Value & componentRoot) const
{
	Component::save(componentRoot);

	componentRoot["skeletonPath"] = m_skeletonPath.toString();

	componentRoot["animationCount"] = m_skeletonAnimations.size();

	for (int i = 0; i < m_skeletonAnimations.size(); i++)
		m_skeletonAnimations[i].save(componentRoot["animations"][i]);

	componentRoot["currentAnimIdx"] = m_currentAnimIdx;
}

void Animator::load(const Json::Value & componentRoot)
{
	Component::load(componentRoot);

	m_skeletonPath = FileHandler::CompletePath(componentRoot.get("skeletonPath", "").asString());
	auto mesh = getMeshFactory().get(m_skeletonPath);
	if (mesh != nullptr) 
	{
		m_skeleton = mesh->getSkeleton();
	}

	int animationCount = componentRoot.get("animationCount", 0).asInt();

	for (int i = 0; i < animationCount; i++) 
	{
	    m_skeletonAnimations.push_back(ResourcePtr<SkeletalAnimation>(componentRoot["animations"][i]));
	}

	m_currentAnimIdx = componentRoot.get("currentAnimIdx", 0).asInt();
}
