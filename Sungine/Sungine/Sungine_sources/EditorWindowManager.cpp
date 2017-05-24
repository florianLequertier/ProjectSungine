#include "EditorWindowManager.h"
#include "EditorGUI.h" //Forward
#include "Application.h" //Forward
#include "Editor.h" //Forward


EditorWindowManager::EditorWindowManager()
{
	m_editorWindows.push_back(std::make_shared<EditorBackgroundWindow>(std::make_shared<EditorNode>(std::make_shared<EditorNodeUniqueDisplay>())));
	m_editorWindows[0]->setSize(Application::get().getWindowWidth(), Application::get().getWindowHeight());
}

bool EditorWindowManager::isFrameOpen(const std::string & frameName) const
{
	return m_frameMapping.find(frameName) != m_frameMapping.end();
}

void EditorWindowManager::onFrameClosed(const std::string & frameName)
{
	auto found = m_frameMapping.find(frameName);
	assert(found != m_frameMapping.end());

	m_frameMapping.erase(found);
}

void EditorWindowManager::addModal(std::shared_ptr<EditorFrame> windowFrame)
{
	if (!m_freeModalIds.empty())
	{
		int currentId = m_freeModalIds.back();
		m_freeModalIds.pop_back();
		m_editorModals[currentId] = std::make_shared<EditorModal>(currentId, windowFrame);
	}
	else
	{
		int currentId = m_editorModals.size();
		m_editorModals.push_back(std::make_shared<EditorModal>(currentId, windowFrame));
	}
}

void EditorWindowManager::removeModal(EditorModal* modal)
{
	auto found = std::find_if(m_editorModals.begin(), m_editorModals.end(), [modal](const std::shared_ptr<EditorModal>& item) { return item.get() == modal; });
	if (found != m_editorModals.end())
	{
		found->reset();
		m_freeModalIds.push_back((int)std::distance(m_editorModals.begin(), found));
	}
}

void EditorWindowManager::removeModal(int modalId)
{
	m_editorModals[modalId].reset();
	m_freeModalIds.push_back(modalId);
}

void EditorWindowManager::setIsResizingChild(bool state)
{
	m_isResizingChild = state;
}

bool EditorWindowManager::getIsResizingChild() const
{
	return m_isResizingChild;
}

void EditorWindowManager::showSeparators()
{
	for (auto window : m_editorWindows)
	{
		if(window)
			window->showSeparators();
	}
}

void EditorWindowManager::hideSeparators()
{
	for (auto window : m_editorWindows)
	{
		if (window)
			window->hideSeparators();
	}
}

void EditorWindowManager::displayModals(Project& project, Editor& editor)
{
	auto modalIter = m_editorModals.begin();
	while (modalIter != m_editorModals.end())
	{
		auto nextModalIter = modalIter;
		nextModalIter++;

		if (*modalIter)
		{
			(*modalIter)->draw(project, editor);

			//close modal ?
			if ((*modalIter)->shouldCloseModal())
			{
				removeModal(std::distance(m_editorModals.begin(), modalIter));
			}
		}

		modalIter = nextModalIter;
	}
}

void EditorWindowManager::addWindow(std::shared_ptr<EditorFrame> windowFrame)
{
	if (!m_freeWindowIds.empty())
	{
		int currentId = m_freeWindowIds.back();
		m_freeWindowIds.pop_back();
		m_editorWindows[currentId] = std::make_shared<EditorWindow>(currentId, windowFrame);
	}
	else
	{
		int currentId = m_editorWindows.size();
		m_editorWindows.push_back(std::make_shared<EditorWindow>(currentId, windowFrame));
	}

	m_frameMapping[windowFrame->getName()] = windowFrame;
}

void EditorWindowManager::addWindow(std::shared_ptr<EditorNode> windowNode)
{
	if (!m_freeWindowIds.empty())
	{
		int currentId = m_freeWindowIds.back();
		m_freeWindowIds.pop_back();
		m_editorWindows[currentId] = std::make_shared<EditorWindow>(currentId, windowNode);
	}
	else
	{
		int currentId = m_editorWindows.size();
		m_editorWindows.push_back(std::make_shared<EditorWindow>(currentId, windowNode));
	}

	std::vector<std::weak_ptr<EditorFrame>> foundFrames;
	windowNode->findAllChildFramesRecursivly(foundFrames);
	for (auto frame : foundFrames)
	{
		m_frameMapping[frame.lock()->getName()] = frame;
	}
}

void EditorWindowManager::addWindowAsynchrone(std::shared_ptr<EditorFrame> windowFrame)
{
	m_windowsToAdd.push_back(std::make_shared<EditorNode>(windowFrame));
}

void EditorWindowManager::addWindowAsynchrone(std::shared_ptr<EditorNode> windowNode)
{
	m_windowsToAdd.push_back(windowNode);
}

void EditorWindowManager::removeWindow(EditorWindow* window)
{
	std::vector<std::weak_ptr<EditorFrame>> childFrames;
	window->findAllChildFramesRecursivly(childFrames);
	std::vector<std::string> childFrameNames;
	for (auto frame : childFrames)
		childFrameNames.push_back(frame.lock()->getName());

	auto found = std::find_if(m_editorWindows.begin(), m_editorWindows.end(), [window](const std::shared_ptr<EditorWindow>& item) { return item.get() == window; });
	if (found != m_editorWindows.end())
	{
		found->reset();
		m_freeWindowIds.push_back((int)std::distance(m_editorWindows.begin(), found));
	}

	int index = 0;
	for (auto frame : childFrames)
	{
		if (frame.expired())
			onFrameClosed(childFrameNames[index]);

		index++;
	}
}

void EditorWindowManager::removeWindow(int windowId)
{
	std::vector<std::weak_ptr<EditorFrame>> childFrames;
	m_editorWindows[windowId]->findAllChildFramesRecursivly(childFrames);
	std::vector<std::string> childFrameNames;
	for (auto frame : childFrames)
		childFrameNames.push_back(frame.lock()->getName());

	m_editorWindows[windowId].reset();
	m_freeWindowIds.push_back(windowId);	

	int index = 0;
	for (auto frame : childFrames)
	{
		if(frame.expired())
			onFrameClosed(childFrameNames[index]);

		index++;
	}
}

std::shared_ptr<EditorWindow> EditorWindowManager::getWindow(int windowId) const
{
	assert(windowId >= 0 && windowId < m_editorWindows.size());

	return m_editorWindows[windowId];
}

std::shared_ptr<EditorWindow> EditorWindowManager::getBackgroundWindow() const
{
	assert(m_editorWindows.size() >= 1);
	return m_editorWindows[0];
}

void EditorWindowManager::setBackgroundWindow(std::shared_ptr<EditorWindow> window)
{
	m_editorWindows[0] = window;
	m_editorWindows[0]->setSize(Application::get().getWindowWidth(), Application::get().getWindowHeight());

	std::vector<std::weak_ptr<EditorFrame>> foundFrames;
	window->getNode()->findAllChildFramesRecursivly(foundFrames);
	for (auto frame : foundFrames)
	{
		m_frameMapping[frame.lock()->getName()] = frame;
	}
}

void EditorWindowManager::displayFloatingWindows(Project& project, Editor& editor)
{
	auto& windowIter = m_editorWindows.begin() + 1;
	while (windowIter != m_editorWindows.end())
	{
		if (*windowIter != nullptr)
		{
			(*windowIter)->draw(project, editor);
		}

		windowIter++;
	}
}

void EditorWindowManager::displayBackgroundWindows(Project& project, Editor& editor)
{
	if (std::abs(m_topMenuOffset - editor.getMenuTopOffset()) > 0.1f)
	{
		m_topMenuOffset = editor.getMenuTopOffset();
		m_editorWindows[0]->setSize(Application::get().getWindowWidth(), Application::get().getWindowHeight() - m_topMenuOffset);
	}

	m_editorWindows[0]->draw(project, editor);
}

void EditorWindowManager::update()
{
	//update windows
	for (auto& window : m_editorWindows)
	{
		if (window)
			window->update();
	}

	ImGui::GetIO().MouseDrawCursor = m_isResizingChild || EditorNode::s_shouldDrawMouseCursorWithImGui;

	if (ImGui::IsMouseReleased(GLFW_MOUSE_BUTTON_LEFT))
	{
		setIsResizingChild(false);
	}

	//deals with asynchronous adding
	for (auto& windowToAdd : m_windowsToAdd)
	{
		addWindow(windowToAdd);
	}
	m_windowsToAdd.clear();
}

void EditorWindowManager::onScreenResized()
{
	getBackgroundWindow()->setSize(Application::get().getWindowWidth(), Application::get().getWindowHeight() - m_topMenuOffset);
}
