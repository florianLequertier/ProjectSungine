#pragma once

#include <string>
#include <memory>

#include "imgui/imgui.h"

#include "ISingleton.h"

class DragAndDropOperation
{
protected:
	int m_type;
	int m_validDropContext;

public:
	DragAndDropOperation()
		:m_type(0)
	{ }

	DragAndDropOperation(int operationType, int operationValidDropContext)
		: m_type(operationType)
		, m_validDropContext(operationValidDropContext)
	{ }

	virtual void dragOperation() = 0;
	virtual void dropOperation(void* customData, int dropContext) = 0;
	virtual void cancelOperation() = 0;
	virtual void updateOperation() = 0;
	virtual bool dragNeedsToHoldMouse() { return true; };
	int getType() const { return m_type; }
	int getValidDropContext() const { return m_validDropContext; }
	virtual bool canDropInto(void* customData, int dropContext) { return true; }
};


class DragAndDropManager : public ISingleton<DragAndDropManager>
{
private:
	std::shared_ptr<DragAndDropOperation> m_currentDragAndDropOperation;
	bool m_isDragAndDropping;

public:
	static bool beginDragAndDrop(std::shared_ptr<DragAndDropOperation> operation);
	static DragAndDropOperation& cancelDragAndDrop();
	static DragAndDropOperation& dropDraggedItem(void* customData, int dropContext);
	static DragAndDropOperation& updateDragAndDrop();
	static bool isDragAndDropping();
	static int getOperationType();
	static int getValidDropContext();
	static bool canDropInto(void* customData, int dropContext);
};