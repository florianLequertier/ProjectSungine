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

template<typename T>
struct MetasFilter
{
	MetasFilter()
	{}

	MetasFilter(const std::string& metas)
	{
	}
};


template<>
struct MetasFilter<std::string>
{
	bool isReadOnly;

	MetasFilter()
	{}

	MetasFilter(const std::string& metas)
	{
		std::unordered_map<std::string, std::string> formated = Metas::formatMetas(metas);
		auto it = formated.find("isreadonly");
		if (it != formated.end())
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
MetasFilter(const std::string& metas)\
{\
	std::unordered_map<std::string, std::string> formated = Metas::formatMetas(metas);\
	auto it = formated.find("valuemin");\
	if (it != formated.end())\
	{\
		valueMin = std::stoi(it->second);\
	}\
	it = formated.find("valuemax");\
	if (it != formated.end())\
	{\
		valueMax = std::stoi(it->second);\
	}\
	it = formated.find("isreadonly");\
	if (it != formated.end())\
	{\
		isReadOnly = it->second == "true" ? true : false;\
	}\
	it = formated.find("isSlider");\
	if (it != formated.end())\
	{\
		isSlider = it->second == "true" ? true : false;\
	}\
}


template<>
struct MetasFilter<int>
{
	METAS_FILTER_INT()
};

template<>
struct MetasFilter<glm::ivec2>
{
	METAS_FILTER_INT()
};

template<>
struct MetasFilter<glm::ivec3>
{
	METAS_FILTER_INT()
};

template<>
struct MetasFilter<glm::ivec4>
{
	METAS_FILTER_INT()
};

#define METAS_FILTER_FLOAT()\
float valueMin;\
float valueMax;\
bool isReadOnly;\
bool isSlider;\
\
MetasFilter()\
{}\
\
MetasFilter(const std::string& metas)\
{\
	std::unordered_map<std::string, std::string> formated = Metas::formatMetas(metas);\
	auto it = formated.find("valuemin");\
	if (it != formated.end())\
	{\
		valueMin = std::stof(it->second);\
	}\
	it = formated.find("valuemax");\
	if (it != formated.end())\
	{\
		valueMax = std::stof(it->second);\
	}\
	it = formated.find("isreadonly");\
	if (it != formated.end())\
	{\
		isReadOnly = it->second == "true" ? true : false;\
	}\
	it = formated.find("isSlider");\
	if (it != formated.end())\
	{\
		isSlider = it->second == "true" ? true : false;\
	}\
}

template<>
struct MetasFilter<float>
{
	METAS_FILTER_FLOAT()
};

template<>
struct MetasFilter<glm::vec2>
{
	METAS_FILTER_FLOAT()
};

template<>
struct MetasFilter<glm::vec3>
{
	METAS_FILTER_FLOAT()
};

template<>
struct MetasFilter<glm::vec4>
{
	METAS_FILTER_FLOAT()
};

namespace Field
{

	// Properties
	template<typename T>
	bool PropertyField(const std::string& propertyName, const T& property, const MetasFilter<T>& metasFilter);

	template<typename T>
	void PropertyField(const std::string& propertyName, const std::vector<T*>& properties, const MetasFilter<T>& metasFilter);

	// Enum
	template<typename T>
	bool EnumField(const std::string& label, T value);
}


// Properties
template<typename T>
bool Field::PropertyField(const std::string& propertyName, const T& property, const MetasFilter<T>& metasFilter)
{
	assert(false);
}

template<typename T>
void Field::PropertyField(const std::string& propertyName, const std::vector<T*>& properties, const MetasFilter<T>& metasFilter)
{
	assert(false);
}

VECTOR_PROPERTY_FIELD(2)
VECTOR_PROPERTY_FIELD(3)
VECTOR_PROPERTY_FIELD(4)

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

