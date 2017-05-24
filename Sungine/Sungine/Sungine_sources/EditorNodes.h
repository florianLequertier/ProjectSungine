#pragma once

#include <memory>
#include <vector>

#include "GUI.h"
#include "imgui/imgui.h"
#include "glm/glm.hpp"

class Project;
class Editor;
class RemovedNodeDatas;
class EditorNode;
class EditorWindow;
class EditorModal;
class EditorFrame;

enum EditorNodeDisplayLogicType {
	HorizontalDisplay,
	VerticalDisplay,
	UniqueDisplay,
	EmptyDisplay,
	FrameDisplay
};

class EditorNodeDisplayLogic
{
public:
	enum ResizeMode {
		NONE,
		HORIZONTAL,
		VERTICAL
	};

private:
	EditorNodeDisplayLogicType m_logicType;
public:
	EditorNodeDisplayLogic(EditorNodeDisplayLogicType logicType) : m_logicType(logicType)
	{}
	virtual bool drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset) = 0;

	virtual void onChildAdded(EditorNode& node, int index) {};
	virtual void onBeforeRemoveChild(EditorNode& node, int index) {};
	void onAfterRemoveChild(EditorNode & node);
	EditorNodeDisplayLogicType getLogicType() const { return m_logicType; }
	int drawDropZone(EditorNode& node, ImVec2 pos, ImVec2 size, bool isActive, Editor& editor, ResizeMode resizeMode);
};

class EditorNodeHorizontalDisplay : public EditorNodeDisplayLogic
{
public:
	EditorNodeHorizontalDisplay() : EditorNodeDisplayLogic(EditorNodeDisplayLogicType::HorizontalDisplay)
	{}
	virtual bool drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset) override;

	virtual void onChildAdded(EditorNode& node, int index) override;
	virtual void onBeforeRemoveChild(EditorNode& node, int index) override;
	bool drawSeparator(int index, float width, float height, EditorNode& node, Editor& editor);
};

class EditorNodeVerticalDisplay : public EditorNodeDisplayLogic
{
public:
	EditorNodeVerticalDisplay() : EditorNodeDisplayLogic(EditorNodeDisplayLogicType::VerticalDisplay)
	{}
	virtual bool drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset) override;

	virtual void onChildAdded(EditorNode& node, int index) override;
	virtual void onBeforeRemoveChild(EditorNode& node, int index) override;
	bool drawSeparator(int index, float width, float height, EditorNode& node, Editor& editor);
};

class EditorNodeUniqueDisplay : public EditorNodeDisplayLogic
{
public:
	EditorNodeUniqueDisplay() : EditorNodeDisplayLogic(EditorNodeDisplayLogicType::UniqueDisplay)
	{}

	virtual bool drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset) override;

	virtual void onChildAdded(EditorNode& node, int index) override;
};

class EditorNodeEmptyDisplay : public EditorNodeDisplayLogic
{
public:
	EditorNodeEmptyDisplay() : EditorNodeDisplayLogic(EditorNodeDisplayLogicType::EmptyDisplay)
	{}
	virtual bool drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset) override;
};

class EditorNodeFrameDisplay : public EditorNodeDisplayLogic
{
public:
	EditorNodeFrameDisplay() : EditorNodeDisplayLogic(EditorNodeDisplayLogicType::FrameDisplay)
	{}
	virtual bool drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset) override;
};

class EditorNode
{
public:
	static bool s_shouldDrawMouseCursorWithImGui;

private:
	std::shared_ptr<EditorNodeDisplayLogic> m_displayLogic;
	std::vector<std::shared_ptr<EditorNode>> m_childNodes;
	std::shared_ptr<EditorFrame> m_frame;
	EditorNode* m_parent;
	EditorWindow* m_parentWindow;
	ImVec2 m_size;
	//ImVec2 m_sizeOffset;
	//ImVec2 m_correctedSize;

public:
	EditorNode(std::shared_ptr<EditorNodeDisplayLogic> displayLogic);
	EditorNode(std::shared_ptr<EditorNodeDisplayLogic> displayLogic, std::shared_ptr<EditorNode> firstNode);
	EditorNode(std::shared_ptr<EditorFrame> frame);
	EditorNode* EditorNode::getParentNode() const;
	const ImVec2& getSize() const;
	//const ImVec2& getSizeOffset() const;
	//const ImVec2& getCorrectedSize() const;
	void setWidth(float newWidth);
	void setHeight(float newHeight);
	void setParentWindow(EditorWindow* parentWindow);
	EditorWindow* getParentWindow() const;
	void findAllChildFramesRecursivly(std::vector<std::weak_ptr<EditorFrame>>& frames) const;

	void removeChild(int index);
	void removeChild(EditorNode* nodeToRemove);
	void addChild(std::shared_ptr<EditorNode> childNode);
	int getChildCount() const;
	std::shared_ptr<EditorNode> getChild(int index) const;
	void insertChild(std::shared_ptr<EditorNode> childNode, int index);
	void onChildAdded(int index);
	void onBeforeRemoveChild(int index);
	EditorNode* removeFromParent();

	bool drawContent(Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset);
	void setDisplayLogic(std::shared_ptr<EditorNodeDisplayLogic> displayLogic);
	EditorNodeDisplayLogicType getDisplayLogicType() const;
	const std::string& getFrameName() const;
	void setFrame(std::shared_ptr<EditorFrame> frame);
	std::shared_ptr<EditorFrame> getFrame() const;
	void onChildRemoved();

	void replaceAABPaterns();
	void simplifyNode();

	friend class EditorNodeDisplayLogic;
	friend class EditorNodeHorizontalDisplay;
	friend class EditorNodeUniqueDisplay;
	friend class EditorNodeVerticalDisplay;
};

class EditorFrame
{
protected:
	std::string m_frameName;
	glm::vec2 m_position;
	glm::vec2 m_size;

public:
	EditorFrame(const std::string& name) : m_frameName(name)
	{}
	virtual void drawContent(Project& project, EditorModal* parentModale = nullptr) = 0;
	const std::string& getName() { return m_frameName; }
	void setSize(float w, float h) { m_size.x = w; m_size.y = h; onFrameResized(); }
	const glm::vec2& getSize() const { return m_size; }
	void setPosition(float x, float y) { m_position.x = x; m_position.y = y; onFrameMoved(); }
	const glm::vec2& getPosition() const { return m_position; }
	virtual void onFrameMoved(){}
	virtual void onFrameResized(){}
};