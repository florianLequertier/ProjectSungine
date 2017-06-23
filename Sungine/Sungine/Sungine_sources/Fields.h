#pragma once

#include "imgui.h"

namespace Field {

enum FieldDisplayType{
	DEFAULT,
	SLIDER
};

template<typename T, FieldDisplayType Display>
bool PropertyField(const std::string& label, T& value)
{
	assert(false);
}

template<,>
inline bool PropertyField<float, FieldDisplayType::DEFAULT>(const std::string& label, float& value)
{
	return ImGui::InputFloat(label.c_str(), &value);
}

template<, >
inline bool PropertyField<float, FieldDisplayType::SLIDER>(const std::string& label, float& value)
{
	return ImGui::SliderFloat(label.c_str(), &value, 0, 100);
}

template<typename T>
bool EnumField(const std::string& label, T value)
{
	int currentCasteddValue = (int)value;
	if (ImGui::ListBox(label.c_str(), &currentCasteddValue, &Enum::GetEnumName<T>, (void*)nullptr, (int)Enum::EnumNames<T>().size()))
	{
		value = (T)currentCasteddValue;
		return true;
	}
	else
		return false;
}

}

