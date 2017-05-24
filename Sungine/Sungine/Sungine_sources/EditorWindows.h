#pragma once

#include "EditorNodes.h"

// Forwards
class EditorNode;
class Project;
class Editor;
class EditorModal;
class EditorFrame;
class EditorWindow;
class RemovedNodeDatas;

///////////////////////////////////////////
//// BEGIN : Editor windows

//A modale displayed in the editor
class EditorModal
{
protected:
	std::string m_modalName;
	int m_modalId;
	bool m_shouldCloseModale;
	std::shared_ptr<EditorFrame> m_frame;

public:
	EditorModal(int windowId, std::shared_ptr<EditorFrame> frame);
	virtual void draw(Project& project, Editor& editor, bool* opened = nullptr);
	bool shouldCloseModal() const;
	void closeModal();
};

//A small helper to allaw asynchronous node removal
struct RemovedNodeDatas {
	std::shared_ptr<EditorNode> nodeToRemove;
};

//A window displayed in the editor
class EditorWindow
{
protected:
	std::string m_windowLabel;
	std::string m_windowStrId;
	int m_windowId;
	std::shared_ptr<EditorNode> m_node;
	ImVec2 m_size;
	ImVec2 m_position;
	bool m_isActive;
	ImVec2 m_separatorSize;
	bool m_areSeparatorsHidden;

	//Style
	float m_alpha; //[0.f, 1.f]

	std::vector<EditorNode*> m_nodesToSimplify;

public:
	EditorWindow(int windowId, std::shared_ptr<EditorFrame> frame);
	EditorWindow(int windowId, std::shared_ptr<EditorNode> node);
	virtual void draw(Project& project, Editor& editor);
	void drawHeader(const std::string& title, bool& shouldClose, bool& shouldMove);

	void setNode(std::shared_ptr<EditorNode> newNode);
	std::shared_ptr<EditorNode> getNode() const;

	//Modifiers : 
	void setIsActive(bool state);
	bool getIsActive() const;
	void setAlpha(float alpha);
	float getAlpha() const;
	void move(const ImVec2& delta);
	void setSize(float w, float h);
	void setPosition(float x, float y);

	void showSeparators();
	void hideSeparators();
	float getSeparatorWidth() const;
	float getSeparatorHeight() const;
	bool getAreSeparatorsHidden() const;

	void simplifyNodeAsynchrone(EditorNode* nodeToSimplify);
	void executeNodeSimplification();
	void update();

	void findAllChildFramesRecursivly(std::vector<std::weak_ptr<EditorFrame>>& childFrames) const;
};

class EditorBackgroundWindow : public EditorWindow
{
public:
	EditorBackgroundWindow(std::shared_ptr<EditorFrame> frame);
	EditorBackgroundWindow(std::shared_ptr<EditorNode> node);
	virtual void draw(Project& project, Editor& editor) override;
};

//// END : Editor windows
///////////////////////////////////////////

