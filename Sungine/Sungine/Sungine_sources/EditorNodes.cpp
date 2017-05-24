#include "EditorNodes.h"
#include "EditorGUI.h" //Forward
#include "EditorWindows.h" //Forward
#include "EditorFrames.h" //Forward
#include "Editor.h" //Forward
#include "imgui_extension.h"

///////////////////////////////////////////////////////////
//// BEGIN : Editor nodes

bool EditorNode::s_shouldDrawMouseCursorWithImGui = false;

EditorNode::EditorNode(std::shared_ptr<EditorNodeDisplayLogic> displayLogic)
	: m_displayLogic(displayLogic)
	, m_parent(nullptr)
	, m_parentWindow(nullptr)
	, m_size(1, 1)
{
}

EditorNode::EditorNode(std::shared_ptr<EditorNodeDisplayLogic> displayLogic, std::shared_ptr<EditorNode> firstNode)
	: m_displayLogic(displayLogic)
	, m_parent(nullptr)
	, m_parentWindow(nullptr)
	, m_size(1, 1)
{
	addChild(firstNode);
}

EditorNode::EditorNode(std::shared_ptr<EditorFrame> frame)
	: m_displayLogic(std::make_shared<EditorNodeFrameDisplay>())
	, m_parent(nullptr)
	, m_parentWindow(nullptr)
	, m_size(1, 1)
{
	setFrame(frame);
}

int EditorNode::getChildCount() const
{
	return m_childNodes.size();
}

void EditorNode::removeChild(int index)
{
	assert(index >= 0 && index < m_childNodes.size());

	onBeforeRemoveChild(index);

	m_childNodes[index]->m_parent = nullptr;
	m_childNodes[index]->setParentWindow(nullptr);
	m_childNodes.erase(m_childNodes.begin() + index);

	//m_displayLogic->onAfterRemoveChild(*this);
}

void EditorNode::removeChild(EditorNode* nodeToRemove)
{
	auto found = std::find_if(m_childNodes.begin(), m_childNodes.end(), [nodeToRemove](std::shared_ptr<EditorNode>& item) { return nodeToRemove == item.get(); });

	if (found != m_childNodes.end())
	{
		int index = std::distance(m_childNodes.begin(), found);

		onBeforeRemoveChild(index);

		m_childNodes[index]->m_parent = nullptr;
		m_childNodes[index]->setParentWindow(nullptr);
		m_childNodes.erase(m_childNodes.begin() + index);

		//m_displayLogic->onAfterRemoveChild(*this);
	}
}

void EditorNode::addChild(std::shared_ptr<EditorNode> childNode)
{
	childNode->m_parent = this;
	childNode->setParentWindow(m_parentWindow);
	m_childNodes.push_back(childNode);

	onChildAdded(m_childNodes.size() - 1);
}

EditorNode* EditorNode::getParentNode() const
{
	return m_parent;
}

const ImVec2& EditorNode::getSize() const
{
	return m_size;
}

void EditorNode::setWidth(float newWidth)
{

	//Set size
	const float lastWidth = getSize().x;
	m_size.x = newWidth;

	//Recursivity
	for (auto& child : m_childNodes)
	{
		float relativeWidth = child->getSize().x / lastWidth;
		child->setWidth(relativeWidth * newWidth);
	}

}

void EditorNode::setHeight(float newHeight)
{
	//Set size
	const float lastHeight = getSize().y;
	m_size.y = newHeight;

	//Recursivity
	for (auto& child : m_childNodes)
	{
		float relativeHeight = child->getSize().y / lastHeight;
		child->setHeight(relativeHeight * newHeight);
	}
}

void EditorNode::setParentWindow(EditorWindow * parentWindow)
{
	m_parentWindow = parentWindow;

	for (auto& child : m_childNodes)
	{
		child->setParentWindow(m_parentWindow);
	}
}

EditorWindow * EditorNode::getParentWindow() const
{
	return m_parentWindow;
}

void EditorNode::findAllChildFramesRecursivly(std::vector<std::weak_ptr<EditorFrame>>& frames) const
{
	if(m_frame)
		frames.push_back(m_frame);

	for (auto child : m_childNodes)
		child->findAllChildFramesRecursivly(frames);
}

std::shared_ptr<EditorNode> EditorNode::getChild(int index) const
{
	assert(index >= 0 && index < m_childNodes.size());

	return m_childNodes[index];
}

void EditorNode::insertChild(std::shared_ptr<EditorNode> childNode, int index)
{
	childNode->m_parent = this;
	childNode->setParentWindow(m_parentWindow);
	m_childNodes.insert(m_childNodes.begin() + index, childNode);
	onChildAdded(index);
}

void EditorNode::onChildAdded(int index)
{
	m_displayLogic->onChildAdded(*this, index);
}

void EditorNode::onBeforeRemoveChild(int index)
{
	m_displayLogic->onBeforeRemoveChild(*this, index);
}

bool EditorNode::drawContent(Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset)
{
	assert(m_displayLogic);

	//Trick : We temporary save the current display logic because we can remove it from the node in drawContent().
	auto saveDisplayLogic = m_displayLogic;
	return m_displayLogic->drawContent(*this, project, editor, removeNodeDatas, parentSizeOffset);
}

void EditorNode::setDisplayLogic(std::shared_ptr<EditorNodeDisplayLogic> displayLogic)
{
	m_displayLogic = displayLogic;
}

EditorNodeDisplayLogicType EditorNode::getDisplayLogicType() const
{
	return m_displayLogic->getLogicType();
}

const std::string& EditorNode::getFrameName() const
{
	assert(m_displayLogic->getLogicType() == EditorNodeDisplayLogicType::FrameDisplay
		|| (m_displayLogic->getLogicType() == EditorNodeDisplayLogicType::UniqueDisplay && m_childNodes.size() == 1 && m_childNodes[0]->getDisplayLogicType() == EditorNodeDisplayLogicType::FrameDisplay));

	if (m_displayLogic->getLogicType() == EditorNodeDisplayLogicType::FrameDisplay)
	{
		return m_frame->getName();
	}
	else if (m_displayLogic->getLogicType() == EditorNodeDisplayLogicType::UniqueDisplay)
	{
		return m_childNodes[0]->getFrameName();
	}
}

void EditorNode::setFrame(std::shared_ptr<EditorFrame> frame)
{
	assert(m_displayLogic->getLogicType() == EditorNodeDisplayLogicType::FrameDisplay);
	m_frame = frame;
}

std::shared_ptr<EditorFrame> EditorNode::getFrame() const
{
	assert(m_displayLogic->getLogicType() == EditorNodeDisplayLogicType::FrameDisplay);
	return m_frame;
}

void EditorNode::onChildRemoved()
{
	//for (int i = 0; i < m_childNodes.size(); i++)
	//{
	//	if (m_childNodes[i]->getChildCount() == 0)
	//		removeChild(i);
	//}

	//if (m_parent != nullptr)
	//	m_parent->onChildRemoved();
	////we are in the root window node
	//else if (m_childNodes[0]->getChildCount() == 1)
	//{
	//	//root window node -> (node 01) horizontal node / vertical node -> (node 02) vertical node / horizontal node -> frame node
	//	std::shared_ptr<EditorNode> node01 = m_childNodes[0];
	//	std::shared_ptr<EditorNode> node02 = m_childNodes[0]->getChild(0);
	//	if (node02->getChildCount() == 1)
	//	{
	//		std::shared_ptr<EditorNode> frameNode = node02->getChild(0); //frame node
	//		node02->removeChild(0); //remove frame
	//		node01->removeChild(0); //remove node 02
	//		removeChild(0); //remove node 01

	//		addChild(frameNode); //add frame node again
	//	}
	//}
}

EditorNode* EditorNode::removeFromParent()
{
	EditorNode* savedParent = m_parent;
	if (m_parent != nullptr)
	{
		m_parent->removeChild(this);
	}
	return savedParent;
}

void EditorNode::replaceAABPaterns()
{
	if (getParentNode() == nullptr)
	{
		if (getChildCount() == 1 && getChild(0)->getChildCount() == 1)
		{
			if (getChild(0)->getChild(0)->getChildCount() == 1)
			{
				auto savedChild = getChild(0)->getChild(0)->getChild(0);
				removeChild(0);
				setDisplayLogic(std::make_shared<EditorNodeUniqueDisplay>());
				addChild(savedChild);
			}
		}
	}
	else
	{
		for (int i = 0; i < getChildCount(); i++)
		{
			if (getChild(i)->getChildCount() == 1)
			{
				if (getDisplayLogicType() == getChild(i)->getChild(0)->getDisplayLogicType())
				{
					auto savedChild = getChild(i)->getChild(0);
					if (savedChild->getDisplayLogicType() == EditorNodeDisplayLogicType::VerticalDisplay)
						savedChild->setDisplayLogic(std::make_shared<EditorNodeHorizontalDisplay>());
					else if (savedChild->getDisplayLogicType() == EditorNodeDisplayLogicType::HorizontalDisplay)
						savedChild->setDisplayLogic(std::make_shared<EditorNodeVerticalDisplay>());

					removeChild(i);
					insertChild(savedChild, i);
				}
			}
		}

		getParentNode()->replaceAABPaterns();
	}
}

void EditorNode::simplifyNode()
{
	auto parentNode = this;
	if (parentNode->getChildCount() == 0)
	{
		parentNode = parentNode->removeFromParent();
	}

	if (parentNode == nullptr)
		return;

	//if (parentNode->getParentNode() != nullptr)
	//{
	//	for (int i = 0; i < parentNode->getParentNode()->getChildCount(); i++)
	//	{
	//		parentNode->getParentNode()->getChild(i)->replaceAABPaterns();
	//	}
	//}
	//else
	parentNode->replaceAABPaterns();


	//if (parentNode->getParentNode() != nullptr && parentNode->getParentNode()->getChildCount() == 1)
	//{
	//	parentNode = parentNode->removeFromParent();
	//}
	//else if (parentNode->getParentNode() == nullptr && parentNode->getParentNode()->getChildCount() == 1)
	//{
	//	setDisplayLogic(std::make_shared<EditorNodeUniqueDisplay>());
	//}

	////Remove patern : vertical_01[horizontal_with_unique_child[vertical_02[horizontals_03]]] (same horizontaly)
	////Simplify to : vertical_01[horizontals_03] (same horizontaly)
	//parentNode = parentNode->getParentNode();
	//if (parentNode == nullptr)
	//	return;

	//for (int i = 0; i < parentNode->getChildCount(); i++)
	//{
	//	auto childNode = parentNode->getChild(i); // == horizontal_with_unique_child
	//	if (childNode->getChildCount() == 1)
	//	{
	//		auto childChildNode = childNode->getChild(0); // == vertical_02
	//		childNode->removeChild(0); // Remove vertical_02 from horizontal_with_unique_child
	//		parentNode->removeChild(0); // Remove horizontal_with_unique_child from vertical_01
	//		
	//		if (parentNode->getParentNode() == nullptr) // Root node ?
	//		{
	//			setDisplayLogic(std::make_shared<EditorNodeUniqueDisplay>()); // Change vertical_01 to unique_01
	//		}

	//		//Foreach horizontals_03
	//		const int childChildChildCount = childChildNode->getChildCount();
	//		for (int j = 0; j < childChildChildCount; j++)
	//		{
	//			auto savedChild = childChildNode->getChild(0);
	//			childChildNode->removeChild(0); // Remove horizontals_03[i] from vertical_02
	//			parentNode->addChild(savedChild); // Add horizontals_03[i] to vertical_01
	//		}

	//	}
	//}
}


///////////////////////

//TODO : use active
int EditorNodeDisplayLogic::drawDropZone(EditorNode& node, ImVec2 rectMin, ImVec2 recMax, bool isActive, Editor& editor, ResizeMode resizeMode)
{
	int draggedWindowId = -1;
	bool isHoveringDropZone = false;

	if (ImGui::IsMouseHoveringRect(rectMin, recMax)
		&& ImGui::IsMouseHoveringWindow())
	{
		if (!editor.getWindowManager()->getIsResizingChild()
			&& DragAndDropManager::isDragAndDropping()
			&& DragAndDropManager::getOperationType() == EditorDragAndDropType::EditorFrameDragAndDrop)
		{
			if (ImGui::IsMouseReleased(GLFW_MOUSE_BUTTON_LEFT))
			{
				DragAndDropManager::dropDraggedItem(&draggedWindowId, EditorDropContext::DropIntoEditorWindow);
			}
			else
			{
				isHoveringDropZone = true;
			}
		}
		else
		{
			if (resizeMode == ResizeMode::HORIZONTAL)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
				EditorNode::s_shouldDrawMouseCursorWithImGui = true;
			}
			else if (resizeMode == ResizeMode::VERTICAL)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
				EditorNode::s_shouldDrawMouseCursorWithImGui = true;
			}
		}
	}

	int dropZoneAlpha = 100;
	if (isHoveringDropZone)
		dropZoneAlpha = 255;


	ImGui::GetWindowDrawList()->AddRectFilled(rectMin, recMax, ImColor(0, 255, 255, dropZoneAlpha));

	return draggedWindowId;
}

//TODO : remove ?
void EditorNodeDisplayLogic::onAfterRemoveChild(EditorNode & node)
{
	if (node.getChildCount() == 1)
	{
		if (node.getParentNode() != nullptr) //root node ?
			node.getParentWindow()->simplifyNodeAsynchrone(node.getParentNode());
	}
}

bool EditorNodeHorizontalDisplay::drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset)
{
	const ImVec2 recMin = ImGui::GetWindowContentRegionMin();
	const ImVec2 recMax = ImGui::GetWindowContentRegionMin();
	const ImVec2 recSize = ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight());
	const int childCount = node.m_childNodes.size();
	const float separatorWidth = (node.getParentWindow()->getAreSeparatorsHidden() /*| !ImGui::IsWindowHovered()*/) ? 0.f : node.getParentWindow()->getSeparatorWidth();
	//const float childWidth = (recSize.x - (childCount + 1)*separatorWidth - padding*2.f) / childCount;
	//const float childHeight = recSize.y - padding*2.f;
	const float padding = 2.f; //small top bottom right and left padding
	const float separatorHeight = recSize.y;

	bool removeChildNode = false;
	int removeChildNodeId = -1;
	bool isResizingChild = false;
	ImVec2 currentSizeOffset(parentSizeOffset);
	currentSizeOffset.x += separatorWidth * (node.getChildCount() + 1.f);
	ImVec2 perChildSizeOffset(currentSizeOffset.x / (float)node.getChildCount(), currentSizeOffset.y);

	isResizingChild |= drawSeparator(0, separatorWidth, separatorHeight, node, editor);

	for (int i = 0; i < childCount; i++)
	{
		EditorNode& currentChildNode = *(node.m_childNodes[i]);

		ImGui::PushID(i);

		ImGui::SameLine();
		ImGui::PushID("Child");
		ImGui::BeginChild(i, ImVec2(currentChildNode.getSize().x - perChildSizeOffset.x, currentChildNode.getSize().y - perChildSizeOffset.y), true, ImGuiWindowFlags_NoScrollbar);

		if (currentChildNode.drawContent(project, editor, removeNodeDatas, perChildSizeOffset))
			removeNodeDatas->nodeToRemove = node.m_childNodes[i];

		ImGui::EndChild();
		ImGui::PopID();

		ImGui::SameLine();
		isResizingChild |= drawSeparator(i + 1, node.getParentWindow()->getSeparatorWidth(), separatorHeight, node, editor);

		ImGui::PopID();
	}

	return false;
}

void EditorNodeHorizontalDisplay::onChildAdded(EditorNode& node, int index)
{
	node.m_childNodes[index]->setWidth(node.getSize().x / node.getChildCount());
	node.m_childNodes[index]->setHeight(node.getSize().y);

	const float availableWidth = node.getSize().x - node.m_childNodes[index]->getSize().x;

	int currentIndex = 0;
	for (auto& child : node.m_childNodes)
	{
		if (currentIndex != index)
		{
			float lastRelativeWidth = child->getSize().x / node.getSize().x;
			child->setWidth(availableWidth * lastRelativeWidth);
		}

		currentIndex++;
	}
}

void EditorNodeHorizontalDisplay::onBeforeRemoveChild(EditorNode & node, int index)
{
	const float lastAvailableWidth = node.getSize().x - node.m_childNodes[index]->getSize().x;

	int currentIndex = 0;
	for (auto& child : node.m_childNodes)
	{
		if (currentIndex != index)
		{
			float lastRelativeWidth = child->getSize().x / lastAvailableWidth;
			child->setWidth(node.getSize().x * lastRelativeWidth);
		}
		currentIndex++;
	}
}

bool EditorNodeHorizontalDisplay::drawSeparator(int index, float width, float height, EditorNode& node, Editor& editor)
{
	bool isResizingChild = false;

	//Invisible button for behavior and sizes
	ImGui::InvisibleButton("##button", ImVec2(width, height));
	const ImVec2 size = ImGui::GetItemRectSize();
	const ImVec2 recMin = ImGui::GetItemRectMin();
	const ImVec2 recMax = ImVec2(recMin.x + size.x, recMin.y + size.y);
	const bool isButtonActive = ImGui::IsItemActive();
	//Display the drop zone
	int droppedWindowId = drawDropZone(node, recMin, recMax, ImGui::IsWindowHovered(), editor, ResizeMode::HORIZONTAL);

	if (droppedWindowId != -1)
	{
		//We get the dragged node from its parent window id
		std::shared_ptr<EditorNode> droppedNode = editor.getWindowManager()->getWindow(droppedWindowId)->getNode()->getChild(0);

		//Assure we insert vertical node in an horizonatl list
		std::shared_ptr<EditorNode> newNode = droppedNode;
		if (droppedNode->getDisplayLogicType() != EditorNodeDisplayLogicType::VerticalDisplay)
			newNode = std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), droppedNode);

		if (node.getChildCount() == 1)
		{
			std::shared_ptr<EditorNode> child = node.getChild(0);
			node.removeChild(0);
			node.addChild(std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), child));
		}

		//We insert the new node
		node.insertChild(newNode, index);

		//We delete the dragged window
		editor.getWindowManager()->removeWindow(droppedWindowId);
	}
	else
	{
		//We handle the resize
		if (isButtonActive
			&& index > 0 && index < node.getChildCount())
		{
			if (index > 0)
			{
				const float lastWidth = node.getChild(index - 1)->getSize().x;
				node.getChild(index - 1)->setWidth(lastWidth + ImGui::GetIO().MouseDelta.x);
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}
			if (index <= node.getChildCount())
			{
				const float lastWidth = node.getChild(index)->getSize().x;
				node.getChild(index)->setWidth(lastWidth - ImGui::GetIO().MouseDelta.x);
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}

			isResizingChild = true;
			editor.getWindowManager()->setIsResizingChild(true);
		}
	}

	return isResizingChild;
}

bool EditorNodeVerticalDisplay::drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset)
{
	const ImVec2 recMin = ImGui::GetWindowContentRegionMin();
	const ImVec2 recMax = ImGui::GetWindowContentRegionMin();
	const ImVec2 recSize = ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight());
	const int childCount = node.m_childNodes.size();
	const float padding = 2.f; //small top bottom right and left padding
							   //const float childWidth = recSize.x - padding*2.f;
							   //const float childHeight = (recSize.y - (childCount + 1)*separatorHeight - padding*2.f) / childCount;
	const float separatorHeight = (node.getParentWindow()->getAreSeparatorsHidden() /*| !ImGui::IsWindowHovered()*/) ? 0.f : node.getParentWindow()->getSeparatorHeight();
	const float separatorWidth = recSize.x;

	bool removeChildNode = false;
	int removeChildNodeId = -1;
	bool isResizingChild = false;
	ImVec2 currentSizeOffset(parentSizeOffset);
	currentSizeOffset.y += separatorHeight * (node.getChildCount() + 1.f);
	ImVec2 perChildSizeOffset(currentSizeOffset.x, currentSizeOffset.y / (float)node.getChildCount());

	isResizingChild |= drawSeparator(0, separatorWidth, separatorHeight, node, editor);

	for (int i = 0; i < node.m_childNodes.size(); i++)
	{
		EditorNode& currentChildNode = *node.m_childNodes[i];

		ImGui::PushID(i);

		ImGui::PushID("Child");
		ImGui::BeginChild(i, ImVec2(currentChildNode.getSize().x - perChildSizeOffset.x, currentChildNode.getSize().y - perChildSizeOffset.y), true, ImGuiWindowFlags_NoScrollbar);

		if (currentChildNode.drawContent(project, editor, removeNodeDatas, perChildSizeOffset))
			removeNodeDatas->nodeToRemove = node.m_childNodes[i];

		ImGui::EndChild();
		ImGui::PopID();

		isResizingChild |= drawSeparator(i + 1, separatorWidth, node.getParentWindow()->getSeparatorHeight(), node, editor);

		ImGui::PopID();
	}

	return false;
}

void EditorNodeVerticalDisplay::onChildAdded(EditorNode & node, int index)
{
	node.m_childNodes[index]->setWidth(node.getSize().x);
	node.m_childNodes[index]->setHeight(node.getSize().y / (float)node.getChildCount());

	const float availableHeight = node.getSize().y - node.m_childNodes[index]->getSize().y;

	int currentIndex = 0;
	for (auto& child : node.m_childNodes)
	{
		if (currentIndex != index)
		{
			float lastRelativeHeight = child->getSize().y / (float)node.getSize().y;
			child->setHeight(availableHeight * lastRelativeHeight);
		}

		currentIndex++;
	}
}

void EditorNodeVerticalDisplay::onBeforeRemoveChild(EditorNode & node, int index)
{
	const float lastAvailableHeight = node.getSize().y - node.m_childNodes[index]->getSize().y;

	int currentIndex = 0;
	for (auto& child : node.m_childNodes)
	{
		if (currentIndex != index)
		{
			float lastRelativeHeight = child->getSize().y / lastAvailableHeight;
			child->setHeight(node.getSize().y * lastRelativeHeight);
		}
		currentIndex++;
	}
}

bool EditorNodeVerticalDisplay::drawSeparator(int index, float width, float height, EditorNode& node, Editor& editor)
{
	bool isResizingChild = false;

	//Invisible button for behavior and sizes
	ImGui::InvisibleButton("##button", ImVec2(width, height));
	const ImVec2 size = ImGui::GetItemRectSize();
	const ImVec2 recMin = ImGui::GetItemRectMin();
	const ImVec2 recMax = ImVec2(recMin.x + size.x, recMin.y + size.y);
	const bool isButtonActive = ImGui::IsItemActive();
	//Display the drop zone
	int droppedWindowId = drawDropZone(node, recMin, recMax, ImGui::IsWindowHovered(), editor, ResizeMode::VERTICAL);

	//We handle the window drop : 
	if (droppedWindowId != -1)
	{
		//We get the dragged node from its parent window id
		std::shared_ptr<EditorNode> droppedNode = editor.getWindowManager()->getWindow(droppedWindowId)->getNode()->getChild(0);

		//Assure we insert horizontal node in a vertical list
		std::shared_ptr<EditorNode> newNode = droppedNode;
		if (droppedNode->getDisplayLogicType() != EditorNodeDisplayLogicType::HorizontalDisplay)
			newNode = std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), droppedNode);

		if (node.getChildCount() == 1)
		{
			std::shared_ptr<EditorNode> child = node.getChild(0);
			node.removeChild(0);
			node.addChild(std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), child));
		}

		//We insert the new node
		node.insertChild(newNode, index);

		//We delete the dragged window
		editor.getWindowManager()->removeWindow(droppedWindowId);
	}
	else
	{
		//We handle the resize 
		if (isButtonActive
			&& index > 0 && index < node.getChildCount())
		{
			if (index > 0)
			{
				const float lastHeight = node.getChild(index - 1)->getSize().y;
				node.getChild(index - 1)->setHeight(lastHeight + ImGui::GetIO().MouseDelta.y);
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
			}
			if (index <= node.getChildCount())
			{
				const float lastHeight = node.getChild(index)->getSize().y;
				node.getChild(index)->setHeight(lastHeight - ImGui::GetIO().MouseDelta.y);
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
			}

			isResizingChild = true;
			editor.getWindowManager()->setIsResizingChild(true);
		}
	}

	return isResizingChild;
}

void EditorNodeUniqueDisplay::onChildAdded(EditorNode & node, int index)
{
	assert(index == 0);
	assert(node.getChildCount() == 1);

	node.m_childNodes[index]->setWidth(node.getSize().x);
	node.m_childNodes[index]->setHeight(node.getSize().y);
}

bool EditorNodeUniqueDisplay::drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset)
{
	const ImVec2 recMin = ImGui::GetItemRectMin();
	const ImVec2 recMax = ImGui::GetItemRectMax();
	const ImVec2 recSize = ImVec2(ImGui::GetWindowContentRegionWidth(), recMax.y - recMin.y);
	const int childCount = node.m_childNodes.size();
	const float horizontalSeparatorHeight = (node.getParentWindow()->getAreSeparatorsHidden() /*| !ImGui::IsWindowHovered()*/) ? 0.f : node.getParentWindow()->getSeparatorHeight();
	const float verticalSeparatorWidth = (node.getParentWindow()->getAreSeparatorsHidden() /*| !ImGui::IsWindowHovered()*/) ? 0.f : node.getParentWindow()->getSeparatorWidth();
	const float padding = 0.f; //small top bottom right and left padding
	const float childWidth = (recSize.x - 2 * verticalSeparatorWidth - padding*2.f);
	const float childHeight = (recSize.y - 2 * horizontalSeparatorHeight - padding*2.f);
	const float verticalSeparatorHeight = childHeight;
	const float horizontalSeparatorWidth = recSize.x;

	ImVec2 currentSizeOffset(parentSizeOffset);
	currentSizeOffset.x += verticalSeparatorWidth * 2.0f;
	currentSizeOffset.y += horizontalSeparatorHeight * 2.0f;

	assert(childCount == 1);

	ImGui::PushID(this);

	//top separator
	//ImGui::BeginChild("##separator01", ImVec2(horizontalSeparatorWidth, horizontalSeparatorHeight), false);
	ImGui::InvisibleButton("##button01", ImVec2(horizontalSeparatorWidth, horizontalSeparatorHeight));
	//ImGui::EndChild();

	int droppedWindowId = drawDropZone(node, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::IsWindowHovered(), editor, ResizeMode::VERTICAL);
	if (droppedWindowId != -1)
	{
		std::shared_ptr<EditorNode> droppedNode = editor.getWindowManager()->getWindow(droppedWindowId)->getNode()->getChild(0);
		std::shared_ptr<EditorNode> singleNode = node.m_childNodes[0];
		node.removeChild(0);

		std::shared_ptr<EditorNode> newHorizontalNode01 = std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), droppedNode);
		std::shared_ptr<EditorNode> newHorizontalNode02 = std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), singleNode);
		auto newVerticalNode = std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), newHorizontalNode01);
		newVerticalNode->addChild(newHorizontalNode02);
		node.addChild(newVerticalNode);
		droppedNode = nullptr;

		//Remove the floating window we just drop on
		editor.getWindowManager()->removeWindow(droppedWindowId);

		//Change the display logic
		node.setDisplayLogic(std::make_shared<EditorNodeHorizontalDisplay>());
	}

	//left separator
	//ImGui::BeginChild("##separator02", ImVec2(verticalSeparatorWidth, verticalSeparatorHeight), false);
	ImGui::InvisibleButton("##button02", ImVec2(verticalSeparatorWidth, verticalSeparatorHeight));
	//ImGui::EndChild();

	droppedWindowId = drawDropZone(node, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::IsWindowHovered(), editor, ResizeMode::HORIZONTAL);
	if (droppedWindowId != -1)
	{
		std::shared_ptr<EditorNode> droppedNode = editor.getWindowManager()->getWindow(droppedWindowId)->getNode()->getChild(0);
		std::shared_ptr<EditorNode> singleNode = node.m_childNodes[0];
		node.removeChild(0);

		std::shared_ptr<EditorNode> newVerticalNode01 = std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), droppedNode);
		std::shared_ptr<EditorNode> newVerticalNode02 = std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), singleNode);
		auto newHorizontalNode = std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), newVerticalNode01);
		newHorizontalNode->addChild(newVerticalNode02);
		node.addChild(newHorizontalNode);
		droppedNode = nullptr;

		editor.getWindowManager()->removeWindow(droppedWindowId);

		//Change the display logic
		node.setDisplayLogic(std::make_shared<EditorNodeVerticalDisplay>());
	}

	ImGui::SameLine();
	ImGui::BeginChild("##child", ImVec2(childWidth, childHeight), true, ImGuiWindowFlags_NoScrollbar);
	node.m_childNodes[0]->drawContent(project, editor, removeNodeDatas, currentSizeOffset);
	ImGui::EndChild();

	//right separator
	ImGui::SameLine();
	//ImGui::BeginChild("##separator03", ImVec2(verticalSeparatorWidth, verticalSeparatorHeight), false);
	ImGui::InvisibleButton("##button03", ImVec2(verticalSeparatorWidth, verticalSeparatorHeight));
	//ImGui::EndChild();

	droppedWindowId = drawDropZone(node, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::IsWindowHovered(), editor, ResizeMode::HORIZONTAL);
	if (droppedWindowId != -1)
	{
		std::shared_ptr<EditorNode> droppedNode = editor.getWindowManager()->getWindow(droppedWindowId)->getNode()->getChild(0);
		std::shared_ptr<EditorNode> singleNode = node.m_childNodes[0];
		node.removeChild(0);

		std::shared_ptr<EditorNode> newVerticalNode01 = std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), singleNode);
		std::shared_ptr<EditorNode> newVerticalNode02 = std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), droppedNode);
		auto newHorizontalNode = std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), newVerticalNode01);
		newHorizontalNode->addChild(newVerticalNode02);
		node.addChild(newHorizontalNode);
		droppedNode = nullptr;

		editor.getWindowManager()->removeWindow(droppedWindowId);

		//Change the display logic
		node.setDisplayLogic(std::make_shared<EditorNodeVerticalDisplay>());
	}

	//bottom separator
	//ImGui::BeginChild("##separator04", ImVec2(horizontalSeparatorWidth, horizontalSeparatorHeight), false);
	ImGui::InvisibleButton("##button04", ImVec2(horizontalSeparatorWidth, horizontalSeparatorHeight));
	//ImGui::EndChild();

	droppedWindowId = drawDropZone(node, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::IsWindowHovered(), editor, ResizeMode::VERTICAL);
	if (droppedWindowId != -1)
	{
		std::shared_ptr<EditorNode> droppedNode = editor.getWindowManager()->getWindow(droppedWindowId)->getNode()->getChild(0);
		std::shared_ptr<EditorNode> singleNode = node.m_childNodes[0];
		node.removeChild(0);

		std::shared_ptr<EditorNode> newHorizontalNode01 = std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), singleNode);
		std::shared_ptr<EditorNode> newHorizontalNode02 = std::make_shared<EditorNode>(std::make_shared<EditorNodeHorizontalDisplay>(), droppedNode);
		auto newVerticalNode = std::make_shared<EditorNode>(std::make_shared<EditorNodeVerticalDisplay>(), newHorizontalNode01);
		newVerticalNode->addChild(newHorizontalNode02);
		node.addChild(newVerticalNode);
		droppedNode = nullptr;

		editor.getWindowManager()->removeWindow(droppedWindowId);

		//Change the display logic
		node.setDisplayLogic(std::make_shared<EditorNodeHorizontalDisplay>());
	}

	ImGui::PopID();

	return false;
}

bool EditorNodeEmptyDisplay::drawContent(EditorNode& node, Project& project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset)
{
	//TODO
	return false;
}

bool EditorNodeFrameDisplay::drawContent(EditorNode & node, Project & project, Editor& editor, RemovedNodeDatas* removeNodeDatas, const ImVec2& parentSizeOffset)
{
	bool needRemove = false;
	const float headerHeight = 20.f;
	float posOffsetY = 0.f;

	ImGui::PushID(this);

	ImVec2 windowSize(node.getSize().x - parentSizeOffset.x, node.getSize().y - parentSizeOffset.y);
	ImGui::BeginChild("##child", windowSize, true/*, ImGuiWindowFlags_NoScrollbar*/ /* ImGuiWindowFlags_HorizontalScrollbar , ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar*/);

	////// Draw header
	if (node.getParentNode()->getDisplayLogicType() != EditorNodeDisplayLogicType::UniqueDisplay)
	{
		posOffsetY += headerHeight;
		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + windowSize.x, ImGui::GetWindowPos().y + headerHeight), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TitleBg]));
		if (ImGui::Ext::ButtonWithTriangleToLeft("##detach", ImVec2(ImGui::GetWindowPos().x + 8.f, ImGui::GetWindowPos().y + 10.f ), ImVec2(20, 20), editor.getStyleSheet().getMainColor()))
		{
			needRemove = true;
		}
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(25, 0));
		ImGui::SameLine();
		ImGui::Text(node.getFrameName().c_str());
		ImGui::Dummy(ImVec2(0, 10.f));
	}
	ImGui::Separator();

	////// Update wize and position
	ImVec2 windowPos = ImGui::GetWindowPos();
	windowPos.y += posOffsetY;
	if (std::abs(windowPos.x - node.getFrame()->getPosition().x) > 0.1f || std::abs(windowPos.y - node.getFrame()->getPosition().y) > 0.1f)
	{
		node.getFrame()->setPosition(windowPos.x, windowPos.y);
	}
	if (!ImGui::IsMouseDown(0) && (std::abs(windowSize.x - node.getFrame()->getSize().x) > 0.1f || std::abs(windowSize.y - node.getFrame()->getSize().y) > 0.1f))
	{
		node.getFrame()->setSize(windowSize.x, windowSize.y);
	}

	///// Draw content
	editor.getStyleSheet().pushFramePadding();
	node.getFrame()->drawContent(project, nullptr);
	editor.getStyleSheet().popFramePadding();
	ImGui::EndChild();

	ImGui::PopID();

	return needRemove;
}


//// END : Editor nodes
///////////////////////////////////////////////////////////

