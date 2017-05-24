#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include "glm/common.hpp"
#include "Resource.h"
#include "ResourcePointer.h"
#include "SkeletalAnimation.h"
#include "EditorGUI.h"
#include "ISerializable.h"


namespace Sungine { namespace Animation {

class AnimationState;
class AnimationTransition;
class ICondition;

class Trigger
{
private:
	bool m_value;
public:
	void setValue(bool value) { m_value = value; };
	bool getValue() const { return m_value; }
};

class AnimationStateMachine final : public Resource, public ISerializable
{
private:
	int m_entryNodeIdx;
	int m_currentNodeIdx;
	// Warning : order matters !
	std::vector<std::shared_ptr<Sungine::Animation::AnimationState>> m_states;
	// Warning : order matters !
	std::vector<std::shared_ptr<Sungine::Animation::AnimationTransition>> m_transitions;
	std::unordered_map<std::string, bool> m_boolParameters;
	std::unordered_map<std::string, float> m_floatParameters;
	std::unordered_map<std::string, Trigger> m_triggerParameters;

	//For UI :
	glm::vec2 m_backgroundDecal;
	std::vector<int> m_selectedNodeIdx;
	glm::vec2 m_dragAnchorPos;
	glm::vec2 m_newNodePos;
	std::shared_ptr<Sungine::Animation::AnimationTransition> m_draggedLink;
	int m_draggedNodeIdx;
	bool m_isSelectingNodes;
	int m_currentParamType;
	int m_selectedLinkIdx;
	bool m_isHoveringANode;

public:

	//Deep copy
	AnimationStateMachine(const AnimationStateMachine& other);
	AnimationStateMachine& operator=(const AnimationStateMachine& other);

	AnimationStateMachine()
		: m_entryNodeIdx(-1)
		, m_currentNodeIdx(-1)
	{}

	AnimationStateMachine(const FileHandler::CompletePath& completePath, bool isDefaultResource = false)
		: Resource(completePath, isDefaultResource)
		, m_entryNodeIdx(-1)
		, m_currentNodeIdx(-1)
	{}

	/////////////
	// Inherit from Resource
	void init(const FileHandler::CompletePath& completePath, const ID& id) override;
	void resolvePointersLoading() override;
	void save() override;

	virtual void drawInInspector(Scene & scene) override;

	/////////////
	// Inherit from ISerializable
	void save(Json::Value& entityRoot) const override;
	void load(const Json::Value& entityRoot) override;

	//////////////
	// Setter to parameters
	void setBool(const std::string& parameterName, bool value);
	void trigger(const std::string& parameterName);
	void setFloat(const std::string& parameterName, float value);
	// getter to parameters
	bool getBool(const std::string& parameterName, bool& outValue) const;
	bool getFloat(const std::string& parameterName, float& outValue) const;

	// Safe way to get animation state from idx
	Sungine::Animation::AnimationState* getState(int index);

	// Safe way to get animation transition from idx
	Sungine::Animation::AnimationTransition* getTransition(int index);

	////////////////////////////////
	// Update animations
	void updateAnimations();

	// For UI :
	void setDragAnchorPos(const glm::vec2& pos);
	bool isSelectingNodes() const;
	void startSelectingNodes();
	void stopSelectingNodes();
	bool getSelectingNodeCount() const;
	void selectSingleNode(int idx);
	void selectNode(int idx);
	void deselectNode(int idx);
	bool isNodeSelected(int idx);
	void deselectAllNodes();
	bool isDraggingLink() const;
	void startDraggingLink(int currentAnimationIdx);
	void stopDraggingLink();
	void makeLinkFromDragginfLink(int nextAnimationStateIdx);
	void startDraggingNode(int stateIdx);
	void stopDraggingNode();
	bool isDraggingNode();
	void removeState(int stateIdx);
	void removeTransition(int transitionIdx);
	void drawUI();
	template<typename T>
	inline bool drawParametersPopup(std::string& outName) { assert(false && "wrong type."); }
	template<typename T>
	inline bool getParameterValue(const std::string& parameterName, T* outParameterValue) { assert(false && "wrong type."); }

};


template<>
inline bool AnimationStateMachine::drawParametersPopup<bool>(std::string& outName)
{
	for (auto parameter : m_boolParameters)
	{
		if (ImGui::Button(parameter.first.c_str()))
		{
			outName = parameter.first;
			return true;
		}
	}
	return false;
}

template<>
inline bool AnimationStateMachine::drawParametersPopup<float>(std::string& outName)
{
	for (auto parameter : m_floatParameters)
	{
		if (ImGui::Button(parameter.first.c_str()))
		{
			outName = parameter.first;
			return true;
		}
	}
	return false;
}

template<>
inline bool AnimationStateMachine::drawParametersPopup<Trigger>(std::string& outName)
{
	for (auto parameter : m_triggerParameters)
	{
		if (ImGui::Button(parameter.first.c_str()))
		{
			outName = parameter.first;
			return true;
		}
	}
	return false;
}

template<>
inline bool AnimationStateMachine::getParameterValue<bool>(const std::string& parameterName, bool* outParameterValue)
{
	auto foundIt = m_boolParameters.find(parameterName);
	if (foundIt != m_boolParameters.end())
	{
		outParameterValue = &foundIt->second;
		return true;
	}
	return false;
}
template<>
inline bool AnimationStateMachine::getParameterValue<float>(const std::string& parameterName, float* outParameterValue)
{
	auto foundIt = m_floatParameters.find(parameterName);
	if (foundIt != m_floatParameters.end())
	{
		outParameterValue = &foundIt->second;
		return true;
	}
	return false;
}

template<>
inline bool AnimationStateMachine::getParameterValue<Trigger>(const std::string& parameterName, Trigger* outParameterValue)
{
	auto foundIt = m_triggerParameters.find(parameterName);
	if (foundIt != m_triggerParameters.end())
	{
		outParameterValue = &foundIt->second;
		return true;
	}
	return false;
}

class AnimationState : public ISerializable
{
	friend AnimationStateMachine;

	// For UI :
	enum MouseState { NONE, HOVERED, RIGHT_CLICKED_DOWN, RIGHT_CLICKED_UP, LEFT_CLICKED_DOWN, LEFT_CLICKED_UP, RIGHT_SELECTED, LEFT_SELECTED };

private:
	ResourcePtr<SkeletalAnimation> m_animation;
	std::shared_ptr<SkeletalAnimation> m_localAnimation;
	float m_animationTime;
	bool m_loop;
	std::vector<int> m_transitionIdx;

	// For UI :
	glm::vec2 m_position;
	glm::vec2 m_size;
	MouseState m_mouseState;

public:
	AnimationState();
	AnimationState(SkeletalAnimation* animation, bool loop = false);
	AnimationState(const AnimationState& other);
	AnimationState& operator=(const AnimationState& other);

	// We consider that an animation is finished if we are at the end of the animation (if it remains timeLeft second to finish the animation)
	bool isFinished(float timeLeft = 0.5);
	Sungine::Animation::AnimationTransition* evaluateTransitions(AnimationStateMachine& manager);
	// Call when we just have transitionning to this animation state
	void start();

	// Call before transitionning to an other animation
	void finish();

	// Inherited from ISerializable
	virtual void save(Json::Value & entityRoot) const override;
	virtual void load(const Json::Value & entityRoot) override;
	void resolvePointerLoading(const Json::Value & entityRoot);

	// For UI :
	void drawUI(AnimationStateMachine& manager, const ImVec2& windowCursor, int stateIdx);
	glm::vec2 getCenter() const;
	void drawInInspector();
};


class AnimationTransition : public ISerializable
{
	friend Sungine::Animation::AnimationStateMachine;

	enum TransitionType { IMMEDIATE, WAIT_END };

	// For UI :
	enum MouseState { NONE, HOVERED, RIGHT_CLICKED_DOWN, RIGHT_CLICKED_UP, LEFT_CLICKED_DOWN, LEFT_CLICKED_UP, RIGHT_SELECTED, LEFT_SELECTED };

private:
	std::vector<std::shared_ptr<Sungine::Animation::ICondition>> m_conditions;
	TransitionType m_transitionType;
	int m_currentAnimationStateIdx;
	int m_nextAnimationStateIdx;

	// For UI :
	MouseState m_mouseState;
	int m_currentConditionType;

public:
	AnimationTransition(int currentAnimationStateIdx = -1, int nextAnimationStateIdx = -1);
	bool evaluate(AnimationStateMachine& manager);
	AnimationState* getCurrentAnimation(AnimationStateMachine& manager);
	AnimationState* getNextAnimation(AnimationStateMachine& manager);
	int getCurrentAnimationIdx();
	int getNextAnimationIdx();

	// Inherited from ISerializable
	virtual void save(Json::Value & entityRoot) const override;
	virtual void load(const Json::Value & entityRoot) override;

	// For UI :
	void drawUI(AnimationStateMachine& manager, const ImVec2& windowCursor);
	void drawInInspector(AnimationStateMachine& manager);
};

//////////////////////////////////////////////////////////////////////////////////////
//// BEGIN : Parameters and conditions

class ICondition : public ISerializable
{
public:
	enum ConditionType { NONE, BOOL, FLOAT, TRIGGER };

protected:
	ConditionType m_conditionType;

public:
	ICondition(ConditionType type)
		: m_conditionType(type)
	{}
	ConditionType getConditionType() { return m_conditionType; }
	virtual bool evaluate(AnimationStateMachine& manager) const = 0;
	virtual void use(AnimationStateMachine& manager) {}

	// For UI :
	virtual void drawUI(AnimationStateMachine& manager) = 0;
};

template<typename T>
class Condition final : public ICondition
{
protected:
	std::function<bool(T, T)> m_operator;
	std::string m_parameterName;
	T m_value;

	// For UI :
	int m_currentOperatorIdx;

public:
	Condition()
		: ICondition(ConditionType::NONE)
	{}

	bool evaluate(AnimationStateMachine& manager) const override
	{
		T* parameter;
		if (manager.getParameterValue<T>(m_parameterName, parameter))
		{
			return m_operator && m_operator(*parameter, m_value);
		}
		else
			return false;
	}

	std::function<bool(T, T)> getOperatorFromIdx()
	{
		assert(false && "wrong type");
	}

	// Inherited fromISerializable
	virtual void save(Json::Value & entityRoot) const override
	{
		assert(false && "wrong type.");
	}

	virtual void load(const Json::Value & entityRoot) override
	{
		assert(false && "wrong type.");
	}

	// For UI :
	void drawUI(AnimationStateMachine& manager) override
	{
		assert(false && "wrong type.");
	}
};

template<>
Condition<bool>::Condition()
	: ICondition(ICondition::ConditionType::BOOL)
{}

template<>
Condition<float>::Condition()
	: ICondition(ICondition::ConditionType::FLOAT)
{}

template<>
std::function<bool(bool, bool)> Condition<bool>::getOperatorFromIdx()
{
	if (m_currentOperatorIdx == 0)
	{
		return [](bool a, bool b) { return a == b; };
	}
	else if (m_currentOperatorIdx == 1)
	{
		return [](bool a, bool b) { return a != b; };
	}
	else
	{
		return [](bool a, bool b) { return a == b; };
	}
}

template<>
std::function<bool(float, float)> Condition<float>::getOperatorFromIdx()
{
	if (m_currentOperatorIdx == 0)
	{
		return [](float a, float b) { return a == b; };
	}
	else if (m_currentOperatorIdx == 1)
	{
		return [](float a, float b) { return a != b; };
	}
	else if (m_currentOperatorIdx == 2)
	{
		return [](float a, float b) { return a > b; };
	}
	else if (m_currentOperatorIdx == 3)
	{
		return [](float a, float b) { return a < b; };
	}
	else
	{
		return [](float a, float b) { return a == b; };
	}
}

template<>
void Condition<bool>::save(Json::Value & entityRoot) const
{
	entityRoot["parameterName"] = m_parameterName;
	entityRoot["value"] = m_value;
	entityRoot["operatorIdx"] = m_currentOperatorIdx;
}

template<>
void Condition<float>::save(Json::Value & entityRoot) const
{
	entityRoot["parameterName"] = m_parameterName;
	entityRoot["value"] = m_value;
	entityRoot["operatorIdx"] = m_currentOperatorIdx;
}

template<>
void Condition<bool>::load(const Json::Value & entityRoot)
{
	m_parameterName = entityRoot["parameterName"].asString();
	m_value = entityRoot["value"].asBool();
	m_currentOperatorIdx = entityRoot["operatorIdx"].asInt();

	m_operator = getOperatorFromIdx();
}

template<>
void Condition<float>::load(const Json::Value & entityRoot)
{
	m_parameterName = entityRoot["parameterName"].asString();
	m_value = entityRoot["value"].asFloat();
	m_currentOperatorIdx = entityRoot["operatorIdx"].asInt();

	m_operator = getOperatorFromIdx();
}

template<>
void Condition<bool>::drawUI(AnimationStateMachine& manager)
{
	// parameter name
	ImVec2 popupPos;
	if (ImGui::Button(m_parameterName.c_str()))
	{
		popupPos = ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y);
		ImGui::OpenPopup("chooseParameterPopup");
	}
	if (ImGui::BeginPopup("chooseParameterPopup"))
	{
		std::string tmpParameterName;
		if (manager.drawParametersPopup<bool>(tmpParameterName))
		{
			m_parameterName = tmpParameterName;
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	// operator
	if (ImGui::Combo("##operator", &m_currentOperatorIdx, "equal/0different"))
	{
		m_operator = getOperatorFromIdx();
	}
	ImGui::SameLine();
	// Value
	ImGui::Checkbox("##value", &m_value);
}

template<>
void Condition<float>::drawUI(AnimationStateMachine& manager)
{
	// parameter name
	ImVec2 popupPos;
	if (ImGui::Button(m_parameterName.c_str()))
	{
		popupPos = ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y);
		ImGui::OpenPopup("chooseParameterPopup");
	}
	if (ImGui::BeginPopup("chooseParameterPopup"))
	{
		std::string tmpParameterName;
		if (manager.drawParametersPopup<float>(tmpParameterName))
		{
			m_parameterName = tmpParameterName;
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	// operator
	if (ImGui::Combo("##operator", &m_currentOperatorIdx, "equal/0different/0greater/0less"))
	{
		m_operator = getOperatorFromIdx();
	}
	ImGui::SameLine();
	// Value
	ImGui::InputFloat("##value", &m_value);
}


template<>
class Condition<Trigger> final : public ICondition, public ISerializable
{
protected:
	std::string m_parameterName;

public:

	Condition()
		: ICondition(ICondition::ConditionType::TRIGGER)
	{}

	bool evaluate(AnimationStateMachine& manager) const override
	{
		Trigger* parameter;
		if (manager.getParameterValue(m_parameterName, parameter))
			return parameter->getValue();
		else
			return false;
	}

	void use(AnimationStateMachine& manager) override
	{
		Trigger* parameter;
		if (manager.getParameterValue(m_parameterName, parameter))
			return parameter->setValue(false);
	}

	void drawUI(AnimationStateMachine& manager)
	{
		// parameter name
		ImVec2 popupPos;
		if (ImGui::Button(m_parameterName.c_str()))
		{
			popupPos = ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y);
			ImGui::OpenPopup("chooseParameterPopup");
		}
		if (ImGui::BeginPopup("chooseParameterPopup"))
		{
			std::string tmpParameterName;
			if (manager.drawParametersPopup<float>(tmpParameterName))
			{
				m_parameterName = tmpParameterName;
			}
			ImGui::EndPopup();
		}
	}

	// Hérité via ISerializable
	virtual void save(Json::Value & entityRoot) const override
	{
		entityRoot["parameterName"] = m_parameterName;
	}

	virtual void load(const Json::Value & entityRoot) override
	{
		m_parameterName = entityRoot["parameterName"].asString();
	}

};


//// END : Parameters and conditions
//////////////////////////////////////////////////////////////////////////////////////

}}
