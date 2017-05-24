#pragma once
#include <cmath>
#include <algorithm>
#include "imgui/imgui.h"

class ImGuiWindow;

namespace ImGui {

	bool MyTreeNode(const char* label, ImVec2& itemPos, ImVec2& itemSize);
	bool MyTreeNode2(const char* ptr_id);
	bool MyCollapsingHeader(const char* label, const char* str_id = NULL, bool display_frame = true, bool default_open = false);
	bool InputFilePath(const char* label, char* entry, size_t stringSize);
	bool IsMouseClickedAnyButton();

	namespace Ext {
		bool collapsingLabelWithRemoveButton(const char* label, bool& shouldRemove);
		ImGuiWindow* FindHoveredWindow(ImVec2 pos, bool excluding_childs, int offsetIndex);
		// Sixed size version : pos is the center of the cross :
		bool SquaredCloseButton(const char* str_id, const ImVec2& pos, float width, bool* isHovered = nullptr);
		// Expandable version :
		bool SquaredCloseButton(const char* str_id, bool* isHovered = nullptr);
		bool ButtonWithTriangleToLeft(const char* str_id, const ImVec2& pos, const ImVec2& size, const ImColor& triangleColor);
		void openStackingPopUp(const char* str_id);
		bool removableTreeNode(const char* label, bool& shouldRemove);
	}
}
