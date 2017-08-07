#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include "imgui.h"
#include "glm/glm.hpp"
#include "glm/common.hpp"

// Vectors
#define VECTOR_PROPERTY_FIELD(VecEntryCount)\
template<>\
void Field::PropertyField<glm::vec##VecEntryCount>(const std::string& propertyName, const std::vector<glm::vec##VecEntryCount*>& properties, const MetasFilter<glm::vec##VecEntryCount>& metasFilter)\
{\
	if (metasFilter.isSlider)\
	{\
		glm::vec##VecEntryCount tmpValue;\
		if (ImGui::SliderFloat##VecEntryCount(propertyName.c_str(), &tmpValue[0], metasFilter.valueMin, metasFilter.valueMax))\
		{\
			for (int i = 0; i < properties.size(); i++)\
			{\
				glm::vec##VecEntryCount* member = properties[i];\
				*member = tmpValue;\
			}\
		}\
	}\
	else\
	{\
		glm::vec##VecEntryCount tmpValue;\
		if (ImGui::InputFloat##VecEntryCount(propertyName.c_str(), &tmpValue[0]))\
		{\
			for (int i = 0; i < properties.size(); i++)\
			{\
				glm::vec##VecEntryCount* member = properties[i];\
				*member = tmpValue;\
			}\
		}\
	}\
}

#define VECTOR_INT_PROPERTY_FIELD(VecEntryCount)\
template<>\
void Field::PropertyField<glm::ivec##VecEntryCount>(const std::string& propertyName, const std::vector<glm::ivec##VecEntryCount*>& properties, const MetasFilter<glm::ivec##VecEntryCount>& metasFilter)\
{\
	if (metasFilter.isSlider)\
	{\
		glm::ivec##VecEntryCount tmpValue;\
		if (ImGui::SliderInt##VecEntryCount(propertyName.c_str(), &tmpValue[0], metasFilter.valueMin, metasFilter.valueMax))\
		{\
			for (int i = 0; i < properties.size(); i++)\
			{\
				glm::ivec##VecEntryCount* member = properties[i];\
				*member = tmpValue;\
			}\
		}\
	}\
	else\
	{\
		glm::ivec##VecEntryCount tmpValue;\
		if (ImGui::InputInt##VecEntryCount(propertyName.c_str(), &tmpValue[0]))\
		{\
			for (int i = 0; i < properties.size(); i++)\
			{\
				glm::ivec##VecEntryCount* member = properties[i];\
				*member = tmpValue;\
			}\
		}\
	}\
}

namespace Metas
{
	std::unordered_map<std::string, std::string> formatMetas(const std::string& metas)
	{
		std::unordered_map<std::string, std::string> outRes;

		std::string res(metas);

		// remove spaces
		res.erase(std::remove(res.begin(), res.end(), ::isspace), res.end());
		// to lower
		std::transform(res.begin(), res.end(), res.begin(), ::tolower);

		size_t offset_0 = 0;
		size_t offset_1 = 0;
		std::string key;
		std::string value;
		while (offset_1 < metas.size() && offset_0 < metas.size())
		{
			offset_1 = metas.find("=", offset_0);
			key = metas.substr(offset_0, offset_1 - offset_0);
			offset_1++;
			offset_0 = metas.find(",", offset_1);
			value = metas.substr(offset_1, offset_0 - offset_1);

			if (key != "" && value != "")
				outRes[key] = value;
		}
	}
}


struct BaseMetasFilter
{
	bool hide;
	bool nosave;

	BaseMetasFilter()
		: hide(false)
		, nosave(false)
	{}

	BaseMetasFilter(const std::unordered_map<std::string, std::string>& formatedMetas)
	{
		auto it = formatedMetas.find("hide");
		if (it != formatedMetas.end())
		{
			hide = it->second == "true" ? true : false;
		}
		auto it = formatedMetas.find("nosave");
		if (it != formatedMetas.end())
		{
			nosave = it->second == "true" ? true : false;
		}
	}
};

template<typename T>
struct MetasFilter
{
	MetasFilter()
	{}

	MetasFilter(const std::unordered_map<std::string, std::string>& formatedMetas)
	{
	}
};


template<>
struct MetasFilter<std::string> : public BaseMetasFilter
{
	bool isReadOnly;

	MetasFilter()
	{}

	MetasFilter(const std::unordered_map<std::string, std::string>& formatedMetas)
	{
		auto it = formatedMetas.find("isreadonly");
		if (it != formatedMetas.end())
		{
			isReadOnly = it->second == "true" ? true : false;
		}
	}
};


#define METAS_FILTER_INT()\
int valueMin;\
int valueMax;\
bool isReadOnly;\
bool isSlider;\
\
MetasFilter()\
{}\
\
MetasFilter(const std::unordered_map<std::string, std::string>& formatedMetas)\
{\
	auto it = formatedMetas.find("valuemin");\
	if (it != formatedMetas.end())\
	{\
		valueMin = std::stoi(it->second);\
	}\
	it = formatedMetas.find("valuemax");\
	if (it != formatedMetas.end())\
	{\
		valueMax = std::stoi(it->second);\
	}\
	it = formatedMetas.find("isreadonly");\
	if (it != formatedMetas.end())\
	{\
		isReadOnly = it->second == "true" ? true : false;\
	}\
	it = formatedMetas.find("isSlider");\
	if (it != formatedMetas.end())\
	{\
		isSlider = it->second == "true" ? true : false;\
	}\
}


#define METAS_FILTER_FLOAT()\
float valueMin;\
float valueMax;\
bool isReadOnly;\
bool isSlider;\
\
MetasFilter()\
{}\
\
MetasFilter(const std::unordered_map<std::string, std::string>& formatedMetas)\
{\
	auto it = formatedMetas.find("valuemin");\
	if (it != formatedMetas.end())\
	{\
		valueMin = std::stof(it->second);\
	}\
	it = formatedMetas.find("valuemax");\
	if (it != formatedMetas.end())\
	{\
		valueMax = std::stof(it->second);\
	}\
	it = formatedMetas.find("isreadonly");\
	if (it != formatedMetas.end())\
	{\
		isReadOnly = it->second == "true" ? true : false;\
	}\
	it = formatedMetas.find("isSlider");\
	if (it != formatedMetas.end())\
	{\
		isSlider = it->second == "true" ? true : false;\
	}\
}


template<>
struct MetasFilter<int> : public BaseMetasFilter
{
	METAS_FILTER_INT()
};

template<>
struct MetasFilter<glm::ivec2> : public BaseMetasFilter
{
	METAS_FILTER_INT()
};

template<>
struct MetasFilter<glm::ivec3> : public BaseMetasFilter
{
	METAS_FILTER_INT()
};

template<>
struct MetasFilter<glm::ivec4> : public BaseMetasFilter
{
	METAS_FILTER_INT()
};

template<>
struct MetasFilter<float> : public BaseMetasFilter
{
	METAS_FILTER_FLOAT()
};

template<>
struct MetasFilter<glm::vec2> : public BaseMetasFilter
{
	METAS_FILTER_FLOAT()
};

template<>
struct MetasFilter<glm::vec3> : public BaseMetasFilter
{
	METAS_FILTER_FLOAT()
};

template<>
struct MetasFilter<glm::vec4> : public BaseMetasFilter
{
	METAS_FILTER_FLOAT()
};


template<>
template<typename T>
struct MetasFilter<AssetHandle<T>> : public BaseMetasFilter
{
	
};

template<>
template<typename T>
struct MetasFilter<ObjectPtr<T>> : public BaseMetasFilter
{

};

namespace Field
{

	// Properties
	template<typename T>
	bool PropertyField(const std::string& propertyName, const T& property, const MetasFilter<T>& metasFilter)
	{
		assert(false);
	}

	template<typename T>
	void PropertyField(const std::string& propertyName, const std::vector<T*>& properties, const MetasFilter<T>& metasFilter)
	{
		assert(false);
	}

	template<typename T>
	bool AssetField(const std::string& propertyName, const std::vector<AssetHandle<T>>& properties, const MetasFilter<T>& metasFilter)
	{
		if (properties.size() == 0)
			return;

		const int bufSize = 100;
		std::string currentResourceName(properties[0].isValid() ? properties[0]->getName() : "INVALID");
		bool sameNames = true;
		for (auto property : properties)
		{
			if (currentResourceName != property->getName())
			{
				sameNames = false;
				break;
			}
		}
		if (!sameNames)
			currentResourceName = "...";
		currentResourceName.reserve(bufSize);

		int resourceType = Object::getStaticClassId<T>();
		bool canDropIntoField = DragAndDropManager::canDropInto(&resourceType, EditorDropContext::DropIntoAssetField);
		bool isTextEdited = false;
		bool needClearPtr = false;

		int colStyleCount = 0;
		if (canDropIntoField)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 255, 0, 255));
			colStyleCount++;
		}

		if (properties.size() == 1 && properties[0].isValid())
		{
			properties[0]->drawIconeInResourceField();
			const bool iconHovered = ImGui::IsItemHovered();
			ImGui::SameLine();
			if (iconHovered)
				properties[0]->drawUIOnHovered();
		}
		ImGui::InputText(label.c_str(), &currentResourceName[0], bufSize, ImGuiInputTextFlags_ReadOnly);
		//isTextEdited = (enterPressed || ImGui::IsKeyPressed(GLFW_KEY_TAB) || (!ImGui::IsItemHovered() && ImGui::IsMouseClickedAnyButton()));
		ImVec2 dropRectMin = ImGui::GetItemRectMin();
		ImVec2 dropRectMax = ImGui::GetItemRectMax();

		//borders if can drop here : 
		if (ImGui::IsMouseHoveringRect(dropRectMin, dropRectMax) && canDropIntoField)
		{
			ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 2.f);
		}

		//drop resource : 
		AssetId droppedAssetId;
		if (ImGui::IsMouseHoveringRect(dropRectMin, dropRectMax) && ImGui::IsMouseReleased(0) && canDropIntoField)
		{
			DragAndDropManager::dropDraggedItem(&droppedAssetId, (EditorDropContext::DropIntoAssetField));
			isTextEdited = true;
		}

		ImGui::SameLine();
		needClearPtr = ImGui::SmallButton(std::string("<##" + label).data());

		ImGui::PopStyleColor(colStyleCount);

		if (needClearPtr)
		{
			for (auto property : properties)
			{
				property.reset();
			}
		}
		else if (isTextEdited)
		{
			AssetHandle<T> foundProp;
			const bool assetFound = AssetManager::instance().getAsset<T>(droppedAssetId, foundProp);
			if (assetFound)
			{
				for (auto property : properties)
				{
					property = foundProp;
				}
			}
		}

		return isTextEdited;
	}

	template<typename T>
	bool ObjectField(const std::string& propertyName, const std::vector<ObjectPtr<T>>& properties, const MetasFilter<T>& metasFilter)
	{
		if (properties.size() == 0)
			return;

		const int bufSize = 100;
		std::string currentResourceName(properties[0].isValid() ? properties[0]->getName() : "INVALID");
		bool sameNames = true;
		for (auto property : properties)
		{
			if (currentResourceName != property->getName())
			{
				sameNames = false;
				break;
			}
		}
		if (!sameNames)
			currentResourceName = "...";
		currentResourceName.reserve(bufSize);

		int resourceType = Object::getStaticClassId<T>();
		bool canDropIntoField = DragAndDropManager::canDropInto(&resourceType, EditorDropContext::DropIntoObjectField);
		bool isTextEdited = false;
		bool needClearPtr = false;

		int colStyleCount = 0;
		if (canDropIntoField)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 255, 0, 255));
			colStyleCount++;
		}

		if (properties.size() == 1 && properties[0].isValid())
		{
			properties[0]->drawIconeInResourceField();
			const bool iconHovered = ImGui::IsItemHovered();
			ImGui::SameLine();
			if (iconHovered)
				properties[0]->drawUIOnHovered();
		}
		ImGui::InputText(label.c_str(), &currentResourceName[0], bufSize, ImGuiInputTextFlags_ReadOnly);
		//isTextEdited = (enterPressed || ImGui::IsKeyPressed(GLFW_KEY_TAB) || (!ImGui::IsItemHovered() && ImGui::IsMouseClickedAnyButton()));
		ImVec2 dropRectMin = ImGui::GetItemRectMin();
		ImVec2 dropRectMax = ImGui::GetItemRectMax();

		//borders if can drop here : 
		if (ImGui::IsMouseHoveringRect(dropRectMin, dropRectMax) && canDropIntoField)
		{
			ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 2.f);
		}

		//drop resource : 
		ObjectPtr<T> draggedObject;
		if (ImGui::IsMouseHoveringRect(dropRectMin, dropRectMax) && ImGui::IsMouseReleased(0) && canDropIntoField)
		{
			DragAndDropManager::dropDraggedItem(&draggedObject, (EditorDropContext::DropIntoResourceField));
			isTextEdited = true;
		}

		ImGui::SameLine();
		needClearPtr = ImGui::SmallButton(std::string("<##" + label).data());

		ImGui::PopStyleColor(colStyleCount);

		if (needClearPtr)
		{
			for (auto property : properties)
			{
				property.reset();
			}
		}
		else if (isTextEdited)
		{
			for (auto property : properties)
			{
				property = draggedObject;
			}
		}

		return isTextEdited;
	}

	// Enum
	template<typename T>
	bool EnumField(const std::string& label, T value);
}

// vec
VECTOR_PROPERTY_FIELD(2)
VECTOR_PROPERTY_FIELD(3)
VECTOR_PROPERTY_FIELD(4)

// ivec
VECTOR_INT_PROPERTY_FIELD(2)
VECTOR_INT_PROPERTY_FIELD(3)
VECTOR_INT_PROPERTY_FIELD(4)

// String
template<>
void Field::PropertyField<std::string>(const std::string& propertyName, const std::vector<std::string*>& properties, const MetasFilter<std::string>& metasFilter)
{
	if (!metasFilter.isReadOnly)
	{
		std::string tmpName(20, '\0');
		if (ImGui::InputText(propertyName.c_str(), &tmpName[0], 20))
		{
			for (int i = 0; i < properties.size(); i++)
			{
				std::string* member = properties[i];
				*member = tmpName;
			}
		}
	}
	else
	{
		bool sameValue = true;
		std::string tmpValue = properties.size() > 0 ? *properties[0] : "";
		for (int i = 1; i < properties.size(); i++)
		{
			const std::string* member = properties[i];
			if (tmpValue != *member)
			{
				sameValue = false;
				break;
			}
			tmpValue = *member;
		}
		
		if (sameValue)
			ImGui::Text(propertyName.c_str(), tmpValue, 20);
		else
			ImGui::Text(propertyName.c_str(), "...", 4);
	}
}

// Float
template<>
void Field::PropertyField<float>(const std::string& propertyName, const std::vector<float*>& properties, const MetasFilter<float>& metasFilter)
{
	if (metasFilter.isSlider)
	{
		float tmpValue;
		if (ImGui::SliderFloat(propertyName.c_str(), &tmpValue, metasFilter.valueMin, metasFilter.valueMax))
		{
			for (int i = 0; i < properties.size(); i++)
			{
				float* member = properties[i];
				*member = tmpValue;
			}
		}
	}
	else
	{
		float tmpValue;
		if (ImGui::InputFloat(propertyName.c_str(), &tmpValue))
		{
			for (int i = 0; i < properties.size(); i++)
			{
				float* member = properties[i];
				*member = tmpValue;
			}
		}
	}
}

// AssetHandle
template<>
template<typename T>
bool PropertyField<AssetHandle<T>>(const std::string& propertyName, const std::vector<AssetHandle<T>>& properties, const MetasFilter<AssetHandle<T>>& metasFilter)
{
	AssetField<AssetHandle<T>>(propertyName, properties, metasFilter);
}

// ObjectPtr
template<>
template<typename T>
bool PropertyField<ObjectPtr<T>>(const std::string& propertyName, const std::vector<AssetHandle<T>>& properties, const MetasFilter<ObjectPtr<T>>& metasFilter)
{
	ObjectField<ObjectPtr<T>>(propertyName, properties, metasFilter);
}

//VECTOR_PROPERTY_FIELD(2)
//VECTOR_PROPERTY_FIELD(3)
//VECTOR_PROPERTY_FIELD(4)
//
//VECTOR_INT_PROPERTY_FIELD(2)
//VECTOR_INT_PROPERTY_FIELD(3)
//VECTOR_INT_PROPERTY_FIELD(4)

// Enum
template<typename T>
bool Field::EnumField(const std::string& label, T value)
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

