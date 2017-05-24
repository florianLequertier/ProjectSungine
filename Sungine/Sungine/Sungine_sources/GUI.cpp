

#include "GUI.h"

bool DragAndDropManager::beginDragAndDrop(std::shared_ptr<DragAndDropOperation> operation)
{
	auto& This = DragAndDropManager::instance();

	if (This.m_isDragAndDropping)
		return false;

	This.m_currentDragAndDropOperation = operation;
	This.m_currentDragAndDropOperation->dragOperation();
	This.m_isDragAndDropping = true;

	return true;
}

DragAndDropOperation& DragAndDropManager::cancelDragAndDrop()
{
	auto& This = DragAndDropManager::instance();

	if (This.m_isDragAndDropping)
		This.m_currentDragAndDropOperation->cancelOperation();
	This.m_isDragAndDropping = false;

	return *This.m_currentDragAndDropOperation;

}

DragAndDropOperation& DragAndDropManager::dropDraggedItem(void* customData, int dropContext)
{
	auto& This = DragAndDropManager::instance();

	if ((dropContext & This.getValidDropContext()) == 0)
		return cancelDragAndDrop();

	if (This.m_isDragAndDropping)
		This.m_currentDragAndDropOperation->dropOperation(customData, dropContext);
	This.m_isDragAndDropping = false;

	return *This.m_currentDragAndDropOperation;
}

DragAndDropOperation& DragAndDropManager::updateDragAndDrop()
{
	auto& This = DragAndDropManager::instance();

	if (This.m_isDragAndDropping)
	{
		if (This.m_currentDragAndDropOperation->dragNeedsToHoldMouse() && ImGui::IsMouseReleased(0))
		{
			This.cancelDragAndDrop();
		}
		else if (!This.m_currentDragAndDropOperation->dragNeedsToHoldMouse() && ImGui::IsMouseClicked(0))
		{
			This.cancelDragAndDrop();
		}
		else
		{
			This.m_currentDragAndDropOperation->updateOperation();
		}
	}


	return *This.m_currentDragAndDropOperation;
}

bool DragAndDropManager::isDragAndDropping()
{
	auto& This = DragAndDropManager::instance();

	return This.m_isDragAndDropping;
}

int DragAndDropManager::getOperationType()
{
	auto& This = DragAndDropManager::instance();

	return This.m_currentDragAndDropOperation ? This.m_currentDragAndDropOperation->getType() : 0;
}

int DragAndDropManager::getValidDropContext()
{
	auto& This = DragAndDropManager::instance();

	return This.m_currentDragAndDropOperation ? This.m_currentDragAndDropOperation->getValidDropContext() : 0;
}

bool DragAndDropManager::canDropInto(void* customData, int dropContext)
{
	auto& This = DragAndDropManager::instance();

	return (
		DragAndDropManager::isDragAndDropping()
		&& (This.m_currentDragAndDropOperation->canDropInto(customData, dropContext))
		);
}