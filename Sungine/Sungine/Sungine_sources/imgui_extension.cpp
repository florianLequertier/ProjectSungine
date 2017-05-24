

#include "imgui_extension.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
//forwards : 
#include "dirent.h"
#include "Utils.h"
#include "FileHandler.h"

namespace ImGui {

	bool MyTreeNode(const char* label, ImVec2& itemPos, ImVec2& itemSize)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const ImGuiStyle& style = g.Style;

		ImU32 id = window->GetID(label);
		bool opened = ImGui::TreeNodeBehaviorIsOpen(id);

		float circle_radius = 8.f;

		ImVec2 pos = window->DC.CursorPos;
		ImRect bb(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + g.FontSize + g.Style.FramePadding.y * 2));
		const ImVec2 padding = opened ? style.FramePadding : ImVec2(style.FramePadding.x, 0.0f);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const float collapser_width = style.FramePadding.x + g.FontSize*0.5f + circle_radius;
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);   // Include collapser
		const float total_width = collapser_width + text_width;

		bb.Max = ImVec2(pos.x + total_width, bb.Max.y);

		itemPos = pos;
		itemSize = ImVec2(ImGui::GetContentRegionAvail().x - bb.GetWidth(), bb.GetHeight());

		bool hovered, held;
		if (ImGui::ButtonBehavior(bb, id, &hovered, &held, true))
			window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
		if (hovered || held)
			window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(1, 0, 0, 1)/*window->Color(held ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered)*/);

		// Icon, text
		float button_sz = g.FontSize + g.Style.FramePadding.y * 2;
		//window->DrawList->AddRectFilled(pos, ImVec2(pos.x + button_sz, pos.y + button_sz), opened ? ImColor(255, 0, 0) : ImColor(0, 255, 0));
		
		ImGuiCol circle_enum_color = hovered ? ImGuiCol_ButtonHovered : (held ? ImGuiCol_ButtonActive : ImGuiCol_Button);

		const float line_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
		window->DrawList->AddCircleFilled(pos + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f), circle_radius, GetColorU32(circle_enum_color));
		RenderCollapseTriangle(pos + ImVec2(3.f,0.f), opened, 1.0f);
		ImGui::RenderText(ImVec2(pos.x + button_sz + g.Style.ItemInnerSpacing.x, pos.y + g.Style.FramePadding.y), label);

		ImGui::ItemSize(bb, g.Style.FramePadding.y);
		ImGui::ItemAdd(bb, &id);

		if (opened)
			ImGui::TreePush(label);
		return opened;
	}


	bool ImGui::MyTreeNode2(const char* ptr_id)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;

		ImGui::PushID(ptr_id);
		const bool opened = ImGui::MyCollapsingHeader(ptr_id, "", false);
		ImGui::PopID();

		if (opened)
			ImGui::TreePush(ptr_id);

		return opened;
	}

	bool ImGui::MyCollapsingHeader(const char* label, const char* str_id, bool display_frame, bool default_open)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImVec2 padding = display_frame ? style.FramePadding : ImVec2(style.FramePadding.x, 0.0f);

		IM_ASSERT(str_id != NULL || label != NULL);
		if (str_id == NULL)
			str_id = label;
		if (label == NULL)
			label = str_id;
		const bool label_hide_text_after_double_hash = (label == str_id); // Only search and hide text after ## if we have passed label and ID separately, otherwise allow "##" within format string.
		const ImGuiID id = window->GetID(str_id);
		const ImVec2 label_size = CalcTextSize(label, NULL, label_hide_text_after_double_hash);

		const float text_base_offset_y = ImMax(0.0f, window->DC.CurrentLineTextBaseOffset - padding.y); // Latch before ItemSize changes it
		const float frame_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y * 2), label_size.y + padding.y * 2);
		/////////
		const float collapser_width = g.FontSize + (display_frame ? padding.x * 2 : padding.x);
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);   // Include collapser
		ItemSize(ImVec2(text_width, frame_height), text_base_offset_y);
		/////////
		ImRect bb = ImRect(window->DC.CursorPos, ImVec2(window->Pos.x + 50/*GetContentRegionMax().x*/, window->DC.CursorPos.y + frame_height));
		if (display_frame)
		{
			// Framed header expand a little outside the default padding
			bb.Min.x -= (float)(int)(window->WindowPadding.x*0.5f) - 1;
			bb.Max.x += (float)(int)(window->WindowPadding.x*0.5f) - 1;
		}

		// For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
		// (Ideally we'd want to add a flag for the user to specify we want want the hit test to be done up to the right side of the content or not)
		const ImRect interact_bb = display_frame ? bb : ImRect(bb.Min.x, bb.Min.y, bb.Min.x + text_width + style.ItemSpacing.x * 2, bb.Max.y);
		bb = interact_bb;
		bool opened = TreeNodeBehaviorIsOpen(id, (default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0) | (display_frame ? ImGuiTreeNodeFlags_NoAutoOpenOnLog : 0));
		if (!ItemAdd(interact_bb, &id))
			return opened;

		bool hovered, held;
		bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, ImGuiButtonFlags_NoKeyModifiers);
		if (pressed)
		{
			opened = !opened;
			window->DC.StateStorage->SetInt(id, opened);
		}

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
		const ImVec2 text_pos = bb.Min + padding + ImVec2(collapser_width, text_base_offset_y);
		if (display_frame)
		{
			// Framed type
			RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
			RenderCollapseTriangle(bb.Min + padding + ImVec2(0.0f, text_base_offset_y), opened, 1.0f);

				RenderTextClipped(text_pos, bb.Max, label, NULL, &label_size);
		}
		else
		{
			// Unframed typed for tree nodes
			if (hovered)
				RenderFrame(bb.Min, bb.Max, col, false);

			RenderCollapseTriangle(bb.Min + ImVec2(padding.x, g.FontSize*0.15f + text_base_offset_y), opened, 0.70f);

			RenderText(text_pos, label, NULL, label_hide_text_after_double_hash);
		}

		return opened;
	}

	bool InputFilePath(const char* label, char* entry, size_t stringSize)
	{
		bool shouldCloseAutocompletionWindow = false;
		bool resetItemSelected = false;
		float posAutoCompletionX = ImGui::GetCursorPosX() + ImGui::GetWindowPos().x;
		
		
		bool textEntered = ImGui::InputText(label, entry, stringSize, 0&(ImGuiInputTextFlags_AutoSelectAll));
		if (textEntered) {
			AutoCompletion::get().setIsActive(true);
			AutoCompletion::get().setHasFocus(false);
		}

		if (!ImGui::IsItemActive()) {
			if (!AutoCompletion::get().getItemSelected())
			{
				shouldCloseAutocompletionWindow = true;
				AutoCompletion::get().setIsActive(false);
			}
		}

		if (AutoCompletion::get().getItemSelected()) {
			ImGui::SetKeyboardFocusHere(-1);
			resetItemSelected = true;
		}

		float posAutoCompletionY = ImGui::GetCursorPosY() + ImGui::GetWindowPos().y;

		ImGui::SetNextWindowPos(ImVec2(posAutoCompletionX, posAutoCompletionY));
		if (AutoCompletion::get().getIsOpen())
		{
			ImGui::BeginTooltip();
			
			std::string path;
			std::string filename;
			std::size_t splitLocation = FileHandler::splitPathFileName(entry, path, filename);
			if (path == "")
				path = "." + path;
			else
				path += "/";

			std::vector<std::string> fileAndDirNames = FileHandler::getAllFileAndDirNames( path );
			AutoCompletion::get().clearWords();

			for (auto& name : fileAndDirNames) {
				if (name.compare(0, filename.size(), filename)==0 || filename.size() == 0) {
					AutoCompletion::get().addWord(name);
				}
			}

			bool autocompletionApplied = AutoCompletion::get().apply(filename);
			if (autocompletionApplied)
			{
				if (path[0] == '.')
					entry[splitLocation] = '\0';
				else {
					entry[splitLocation] = '/';
					entry[splitLocation+1] = '\0';
				}
				std::strcat(entry, filename.c_str());
			}

			AutoCompletion::get().setIsOpen(false);
			ImGui::EndTooltip();
		}

		AutoCompletion::get().setIsOpen(!shouldCloseAutocompletionWindow);
		if(resetItemSelected)
			AutoCompletion::get().setItemSelected(false);

		return textEntered;

	}

	bool IsMouseClickedAnyButton()
	{
		ImGuiContext& g = *GImGui;
		int arraySize = IM_ARRAYSIZE(g.IO.MouseDown);
		for (int i = 0; i < arraySize; i++)
		{
			if (IsMouseClicked(i))
				return true;
		}
		return false;
	}

namespace Ext {

	bool collapsingLabelWithRemoveButton(const char* label, bool& shouldRemove)
	{
		const float btnWidth = 18.f;

		ImGui::PushID(label);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - btnWidth);
		bool clicked = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_AllowOverlapMode);
		const float itemHeight = ImGui::GetItemRectSize().y;
		const ImVec2 pos = ImGui::GetWindowPos() + ImGui::GetCursorPos() + ImVec2(ImGui::GetContentRegionAvailWidth() - 50, 0) - ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
		if (ImGui::CloseButton(ImGui::GetID("closeBtn"), pos + ImVec2(btnWidth * 0.5, - itemHeight*0.5f - ImGui::GetStyle().ItemSpacing.y), btnWidth*0.5))
			shouldRemove = true;
		ImGui::PopID();

		return clicked;
	}

	ImGuiWindow* FindHoveredWindow(ImVec2 pos, bool excluding_childs, int offsetIndex)
	{
		ImGuiContext& g = *GImGui;

		if (offsetIndex > g.Windows.Size - 1)
			return NULL;
		int ignoredWindowCount = 0;
		int finalOffsetIndex = offsetIndex;

		int idx = 0;
		while (ignoredWindowCount != offsetIndex)
		{
			ImGuiWindow* window = g.Windows[idx];
			while ((window->Flags & ImGuiWindowFlags_ChildWindow) != 0)
			{
				window = g.Windows[idx];
				finalOffsetIndex++;
				idx++;
			}
			ignoredWindowCount++;
			idx++;
		}

		for (int i = g.Windows.Size - 1 - finalOffsetIndex; i >= 0; i--)
		{
			ImGuiWindow* window = g.Windows[i];

			if (!window->Active)
				continue;
			if (window->Flags & ImGuiWindowFlags_NoInputs)
				continue;
			if (excluding_childs && (window->Flags & ImGuiWindowFlags_ChildWindow) != 0)
				continue;

			// Using the clipped AABB so a child window will typically be clipped by its parent.
			ImRect bb(window->WindowRectClipped.Min - g.Style.TouchExtraPadding, window->WindowRectClipped.Max + g.Style.TouchExtraPadding);
			if (bb.Contains(pos))
				return window;
		}
		return NULL;
	}

	bool ButtonWithTriangleToLeft(const char* str_id, const ImVec2& pos, const ImVec2& size, const ImColor& triangleColor)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiStyle& style = ImGui::GetStyle();
		const ImGuiID id = window->GetID(str_id);

		const ImRect bb(pos - size*0.5f, pos + size*0.5f);

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		// Render
		const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_CloseButtonHovered : ImGuiCol_CloseButton);
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding);
		const ImVec2 offset(3.f, 3.f);
		ImVec2 a = bb.Min + ImVec2(offset.x, bb.GetSize().y * 0.5f);
		ImVec2 b = bb.Min + ImVec2(size.x - offset.x, offset.y);
		ImVec2 c = b + ImVec2(0.f, bb.GetSize().y - 2.f * offset.y);

		window->DrawList->AddTriangleFilled(a, b, c, triangleColor);

		return pressed;

	}

	bool SquaredCloseButton(const char* str_id, const ImVec2& pos, float width, bool* isHovered)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiStyle& style = ImGui::GetStyle();
		const ImGuiID id = window->GetID(str_id);

		const ImRect bb(pos - ImVec2(width*0.5f, width*0.5f), pos + ImVec2(width*0.5f, width*0.5f));

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		// Render
		const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_CloseButtonHovered : ImGuiCol_CloseButton);
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding);

		const float cross_extent = (width * 0.35355f) - 1.0f;
		if (hovered)
		{
			window->DrawList->AddLine(pos + ImVec2(+cross_extent, +cross_extent), pos + ImVec2(-cross_extent, -cross_extent), ImGui::GetColorU32(ImGuiCol_Text));
			window->DrawList->AddLine(pos + ImVec2(+cross_extent, -cross_extent), pos + ImVec2(-cross_extent, +cross_extent), ImGui::GetColorU32(ImGuiCol_Text));
		}

		if (isHovered != nullptr)
			*isHovered = hovered;
		return pressed;
	}

	bool SquaredCloseButton(const char* str_id, bool* isHovered)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiStyle& style = ImGui::GetStyle();
		const ImGuiID id = window->GetID(str_id);

		const ImVec2 pos = ImGui::GetCursorPos();
		const ImVec2 availableContentSize = ImGui::GetContentRegionAvail();
		const ImRect bb(pos, pos + availableContentSize);//(pos - ImVec2(width*0.5f, width*0.5f), pos + ImVec2(width*0.5f, width*0.5f));

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		// Render background : 
		const ImU32 bgCol = ImGui::GetColorU32((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_CloseButtonHovered : ImGuiCol_CloseButton);
		window->DrawList->AddRectFilled(bb.Min, bb.Max, bgCol, style.FrameRounding);

		// Render cross : 
		const ImU32 crossCol = ImGui::GetColorU32((held && hovered) ? ImGuiCol_CloseButtonHovered : hovered ? ImGuiCol_CloseButton : ImGuiCol_CloseButtonActive);
		const float cross_offset = 2.f;
		if (hovered)
		{
			window->DrawList->AddLine(bb.Min + ImVec2(cross_offset, cross_offset), bb.Max + ImVec2(-cross_offset, cross_offset), crossCol);
			window->DrawList->AddLine(bb.Min + ImVec2(cross_offset, -cross_offset), bb.Max + ImVec2(-cross_offset, -cross_offset), crossCol);
		}

		ImGui::ItemSize(bb);
		ImGui::ItemAdd(bb, &id);

		if (isHovered != nullptr)
			*isHovered = hovered;
		return pressed;
	}

	void openStackingPopUp(const char* str_id)
	{
		ImGui::OpenPopupEx(str_id, true);
	}

	bool removableTreeNode(const char* label, bool& shouldRemove)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const ImGuiStyle& style = g.Style;

		ImU32 id = window->GetID(label);
		bool opened = ImGui::TreeNodeBehaviorIsOpen(id);

		float arrow_offset = 8.f;

		float removeBtnWidth = 20.f;
		ImVec2 pos = window->DC.CursorPos;
		ImRect bb(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x - removeBtnWidth, pos.y + g.FontSize + g.Style.FramePadding.y * 2));
		const ImVec2 padding = opened ? style.FramePadding : ImVec2(style.FramePadding.x, 0.0f);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const float collapser_width = style.FramePadding.x + g.FontSize*0.5f + arrow_offset;
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);   // Include collapser
		const float total_width = collapser_width + text_width;

		bb.Max = ImVec2(pos.x + total_width, bb.Max.y);

		bool hovered, held;
		if (ImGui::ButtonBehavior(bb, id, &hovered, &held))
			window->DC.StateStorage->SetInt(id, opened ? 0 : 1);
		if (hovered || held)
			window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(held ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered) );

		// Icon, text
		float button_sz = g.FontSize + g.Style.FramePadding.y * 2;
		window->DrawList->AddRectFilled(pos, ImVec2(pos.x + button_sz, pos.y + button_sz), opened ? ImColor(255, 0, 0) : ImColor(0, 255, 0));

		ImGuiCol circle_enum_color = hovered ? ImGuiCol_ButtonHovered : (held ? ImGuiCol_ButtonActive : ImGuiCol_Button);

		const float line_height = ImMax(ImMin(window->DC.CurrentLineHeight, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
		//window->DrawList->AddCircleFilled(pos + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f), circle_radius, GetColorU32(circle_enum_color));
		RenderCollapseTriangle(pos + ImVec2(3.f, 0.f), opened, 1.0f);
		ImGui::RenderText(ImVec2(pos.x + button_sz + g.Style.ItemInnerSpacing.x, pos.y + g.Style.FramePadding.y), label);

		ImGui::ItemSize(bb, g.Style.FramePadding.y);
		ImGui::ItemAdd(bb, &id);

		if (opened)
			ImGui::TreePush(label);


		// Remove button : 
		//window->DrawList->AddLine(ImVec2(bb.Max.x + 1.f, bb.Min.y), ImVec2(bb.Max.x + 1.f, bb.Max.y), GetColorU32(ImGuiCol_Border));
		//pos = window->DC.CursorPos;
		//ImRect bbBtn(ImVec2(pos.x + 2.f, pos.y), ImVec2(pos.x + removeBtnWidth, pos.y + removeBtnWidth));
		//shouldRemove = ImGui::ButtonBehavior(bbBtn, id, &hovered, &held);

		//ImGui::SameLine();
		const float offsetBeforeCloseBtnCenter = 2.f + removeBtnWidth * 0.5f;
		ImGui::PushID("removeBtn");
		shouldRemove = ImGui::Ext::SquaredCloseButton(label, ImVec2(bb.Max.x + offsetBeforeCloseBtnCenter, bb.GetHeight()*0.5f), removeBtnWidth);
		ImGui::PopID();
		return opened;
	}


}}

