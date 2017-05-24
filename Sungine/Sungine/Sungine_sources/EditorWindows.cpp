#include "Editor.h"
#include "EditorGUI.h"
#include "EditorWindows.h"
#include "ResourceTree.h"
#include "Project.h" //forward
#include "imgui_extension.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"


///////////////////////////////////////////////////////////
//// BEGIN : Editor Windows and modals

static float s_separatorWidth = 5.0f;
static float s_separatorHeight = 5.0f;

EditorModal::EditorModal(int windowId, std::shared_ptr<EditorFrame> frame)
	: m_modalId(windowId)
	, m_shouldCloseModale(false)
{
	m_modalName = "editorModale_" + std::to_string(m_modalId);
	m_frame = frame;
}

void EditorModal::draw(Project& project, Editor& editor, bool* opened)
{
	ImGui::OpenPopup(m_modalName.data());

	if (ImGui::BeginPopupModal(m_modalName.data(), opened))
	{
		m_frame->drawContent(project, this);

		ImGui::EndPopup();
	}
}

bool EditorModal::shouldCloseModal() const
{
	return m_shouldCloseModale;
}

void EditorModal::closeModal()
{
	ImGui::CloseCurrentPopup();
	m_shouldCloseModale = true;
}

//////////////////

EditorWindow::EditorWindow(int windowId, std::shared_ptr<EditorFrame> frame)
	: m_windowId(windowId)
	, m_size(200, 300)
	, m_position(0, 0)
	, m_alpha(1)
	, m_separatorSize(s_separatorWidth, s_separatorHeight)
	, m_areSeparatorsHidden(true)
{
	setNode(std::make_shared<EditorNode>(std::make_shared<EditorNodeUniqueDisplay>(), std::make_shared<EditorNode>(frame)));
	m_windowStrId = "editorWindow_" + std::to_string(m_windowId);
}

EditorWindow::EditorWindow(int windowId, std::shared_ptr<EditorNode> node)
	: m_windowId(windowId)
	, m_size(200, 300)
	, m_position(0, 0)
	, m_alpha(1)
	, m_separatorSize(s_separatorWidth, s_separatorHeight)
	, m_areSeparatorsHidden(true)
{
	setNode(std::make_shared<EditorNode>(std::make_shared<EditorNodeUniqueDisplay>(), node));
	m_windowStrId = "editorWindow_" + std::to_string(m_windowId);
}

void EditorWindow::draw(Project& project, Editor& editor)
{
	RemovedNodeDatas removeNodeDatas;
	ImVec2 currentWindowSize = m_size;
	ImVec2 currentWindowPosition = m_position;

	////BEGIN DRAW

	//Override style
	float previousAlpha = ImGui::GetStyle().Alpha;
	ImGui::GetStyle().Alpha = m_alpha;

	////TODO : move this code
	if (m_node->getDisplayLogicType() == EditorNodeDisplayLogicType::UniqueDisplay
		&& m_node->getChildCount() == 1 && m_node->getChild(0)->getDisplayLogicType() == EditorNodeDisplayLogicType::FrameDisplay)
		m_windowLabel = m_node->getFrameName();
	else
		m_windowLabel = "Window";

	ImGui::SetNextWindowPos(m_position);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_ShowBorders;
	ImGuiWindowFlags inputFlag = m_isActive ? 0 : ImGuiWindowFlags_NoInputs;
	flags |= inputFlag;
	ImGui::Begin(m_windowStrId.c_str(), nullptr, flags);
	ImGui::PushID(m_windowStrId.c_str());

	bool shouldClose = false, shouldMove = false;
	drawHeader(m_windowLabel, shouldClose, shouldMove);
	//launch drag and drop of the window
	if (shouldMove)
	{
		DragAndDropManager::beginDragAndDrop(std::make_shared<EditorFrameDragAndDropOperation>(m_windowId, &editor));
	}
	else
	{
		currentWindowPosition = ImGui::GetWindowPos();
	}

	ImGui::BeginChild("##childs", ImVec2(0, 0), false);
	ImVec2 sizeOffset(0, 0);
	m_node->drawContent(project, editor, &removeNodeDatas, sizeOffset);
	currentWindowSize = ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y);
	ImGui::EndChild();

	ImGui::PopID();
	ImGui::End();

	ImGui::GetStyle().Alpha = previousAlpha;

	////END DRAW

	//We have resized the window
	if (std::abs(currentWindowSize.x - m_size.x) > 0.1f || std::abs(currentWindowSize.y - m_size.y) > 0.1f)
	{
		m_size = currentWindowSize;
		m_node->setWidth(m_size.x);
		m_node->setHeight(m_size.y);
	}
	//we have moved the window
	if (std::abs(currentWindowPosition.x - m_position.x) > 0.1f || std::abs(currentWindowPosition.y - m_position.y) > 0.1f)
	{
		m_position = currentWindowPosition;
	}

	//We want to remove a node
	if (removeNodeDatas.nodeToRemove)
	{
		auto parentNode = removeNodeDatas.nodeToRemove->getParentNode();
		parentNode->removeChild(removeNodeDatas.nodeToRemove.get());
		editor.getWindowManager()->addWindowAsynchrone(removeNodeDatas.nodeToRemove);
		//parentNode->onChildRemoved();
		parentNode->getParentWindow()->simplifyNodeAsynchrone(parentNode);
	}

	//We want to close the window
	//close window
	if (!shouldMove && shouldClose)
	{
		editor.getWindowManager()->removeWindow(m_windowId);
	}
}


void EditorWindow::setNode(std::shared_ptr<EditorNode> newNode)
{
	m_node = newNode;
	m_node->setParentWindow(this);
	m_node->setWidth(m_size.x);
	m_node->setHeight(m_size.y);
}

std::shared_ptr<EditorNode> EditorWindow::getNode() const
{
	return m_node;
}

void EditorWindow::setIsActive(bool state)
{
	m_isActive = state;
}

bool EditorWindow::getIsActive() const
{
	return m_isActive;
}

void EditorWindow::setAlpha(float alpha)
{
	m_alpha = alpha;
}

float EditorWindow::getAlpha() const
{
	return m_alpha;
}

void EditorWindow::drawHeader(const std::string& title, bool& shouldClose, bool& shouldMove)
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove;
	flags |= m_isActive ? 0 : ImGuiWindowFlags_NoInputs;

	ImGui::PushID(title.c_str());
	ImGui::BeginChild("##Header", ImVec2(0, 20), false, flags);

	//Render title
	const float topPadding = 3.f;
	const float leftPadding = 5.f;
	const ImGuiWindow* window = ImGui::GetCurrentWindow();
	const ImGuiStyle& style = ImGui::GetStyle();

	const ImVec2 textPos(ImGui::GetWindowPos().x + leftPadding, ImGui::GetWindowPos().y + topPadding);
	ImGui::GetWindowDrawList()->AddText(textPos, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]), title.c_str());

	ImVec2 buttonPos = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - 15.f, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()*0.5);
	bool isCloseButtonHovered = false;
	shouldClose = ImGui::Ext::SquaredCloseButton("><##Header", buttonPos, 16.f, &isCloseButtonHovered);

	shouldMove = ImGui::IsMouseHoveringWindow() && ImGui::IsWindowFocused() && ImGui::IsMouseDown(0) && !isCloseButtonHovered;

	ImGui::EndChild();
	ImGui::PopID();
}

void EditorWindow::move(const ImVec2 & delta)
{
	m_position.x += delta.x;
	m_position.y += delta.y;
}

void EditorWindow::setSize(float w, float h)
{
	m_size.x = w;
	m_size.y = h;

	m_node->setWidth(w);
	m_node->setHeight(h);
}

void EditorWindow::setPosition(float x, float y)
{
	m_position.x = x;
	m_position.y = y;
}

void EditorWindow::showSeparators()
{
	m_areSeparatorsHidden = false;
}

void EditorWindow::hideSeparators()
{
	m_areSeparatorsHidden = true;
}

float EditorWindow::getSeparatorWidth() const
{
	return m_separatorSize.x;
}

float EditorWindow::getSeparatorHeight() const
{
	return m_separatorSize.y;
}

bool EditorWindow::getAreSeparatorsHidden() const
{
	return m_areSeparatorsHidden;
}

void EditorWindow::simplifyNodeAsynchrone(EditorNode* nodeToSimplify)
{
	m_nodesToSimplify.push_back(nodeToSimplify);
}

void EditorWindow::executeNodeSimplification()
{
	for (auto& node : m_nodesToSimplify)
	{
		if (node != nullptr)
			node->simplifyNode();
	}

	m_nodesToSimplify.clear();
}

void EditorWindow::update()
{
	executeNodeSimplification();
}

void EditorWindow::findAllChildFramesRecursivly(std::vector<std::weak_ptr<EditorFrame>>& childFrames) const
{
	if (m_node->getDisplayLogicType() == EditorNodeDisplayLogicType::FrameDisplay)
	{
		auto frame = m_node->getFrame();
		if (frame)
			childFrames.push_back(frame);
	}
	m_node->findAllChildFramesRecursivly(childFrames);
}

EditorBackgroundWindow::EditorBackgroundWindow(std::shared_ptr<EditorFrame> frame)
	: EditorWindow(0, frame)
{
	m_position.x = 0;
	m_position.y = 0;
}

EditorBackgroundWindow::EditorBackgroundWindow(std::shared_ptr<EditorNode> node)
	: EditorWindow(0, node)
{
	m_position.x = 0;
	m_position.y = 0;
}

void EditorBackgroundWindow::draw(Project & project, Editor & editor)
{
	RemovedNodeDatas removeNodeDatas;
	ImVec2 parentSizeOffset(0, 0);

	ImGui::SetNextWindowPos(ImVec2(m_position.x, m_position.y + editor.getMenuTopOffset()));
	ImGui::SetNextWindowSize(ImVec2(m_size.x, m_size.y));
	ImGui::Begin(m_windowStrId.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::PushID("MainWindow");
	m_node->drawContent(project, editor, &removeNodeDatas, parentSizeOffset);
	ImGui::PopID();
	ImGui::End();

	//We want to remove a node
	if (removeNodeDatas.nodeToRemove)
	{
		auto parentNode = removeNodeDatas.nodeToRemove->getParentNode();
		parentNode->removeChild(removeNodeDatas.nodeToRemove.get());
		editor.getWindowManager()->addWindowAsynchrone(removeNodeDatas.nodeToRemove);
		//parentNode->onChildRemoved();
		parentNode->getParentWindow()->simplifyNodeAsynchrone(parentNode);
	}
}


//// END : Editor Windows and modals
///////////////////////////////////////////////////////////