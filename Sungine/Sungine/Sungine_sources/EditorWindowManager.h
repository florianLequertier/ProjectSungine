#pragma once

#include <map>

#include "EditorWindows.h"

//Manager which handle windows and modals
class EditorWindowManager
{
private:
	//windows (views)
	std::vector<std::shared_ptr<EditorWindow>> m_editorWindows;
	std::vector<int> m_freeWindowIds;
	std::vector<std::shared_ptr<EditorNode>> m_windowsToAdd;

	//Modals (views)
	std::vector<std::shared_ptr<EditorModal>> m_editorModals;
	std::vector<int> m_freeModalIds;

	bool m_isResizingChild;
	float m_topMenuOffset;

	std::map<std::string, std::weak_ptr<EditorFrame>> m_frameMapping;

public:
	EditorWindowManager();

	bool isFrameOpen(const std::string& frameName) const;
	void onFrameClosed(const std::string& frameName);

	std::shared_ptr<EditorWindow> getBackgroundWindow() const;
	void setBackgroundWindow(std::shared_ptr<EditorWindow> window);
	void displayBackgroundWindows(Project& project, Editor& editor);

	void displayFloatingWindows(Project& project, Editor& editor);
	void addWindow(std::shared_ptr<EditorFrame> windowFrame);
	void addWindow(std::shared_ptr<EditorNode> windowNode);
	void removeWindow(EditorWindow* modal);
	void removeWindow(int windowId);
	std::shared_ptr<EditorWindow> getWindow(int windowId) const;
	void addWindowAsynchrone(std::shared_ptr<EditorFrame> windowFrame);
	void addWindowAsynchrone(std::shared_ptr<EditorNode> windowNode);

	void displayModals(Project& project, Editor& editor);
	void addModal(std::shared_ptr<EditorFrame> windowFrame);
	void removeModal(EditorModal* modal);
	void removeModal(int modalId);

	void setIsResizingChild(bool state);
	bool getIsResizingChild() const;

	void showSeparators();
	void hideSeparators();

	//deals with asynchonous commands
	void update();

	//external callbacks
	void onScreenResized();
};