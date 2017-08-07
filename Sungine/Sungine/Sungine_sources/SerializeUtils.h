#pragma once

#include <vector>
#include <map>
#include <sstream>

#include "glew/glew.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "jsoncpp/json/json.h"
#include "ErrorHandler.h"

// Type or value to string

namespace Utils {

bool OpenFileStreamRead(const FileHandler::CompletePath& filepath, std::ifstream& outStream)
{
	assert(!Project::isPathPointingInsideProjectFolder(filePath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(filePath);

	outStream.open(absolutePath.toString());
	if (!outStream.is_open())
	{
		std::cout << "error, can't open file at path : " << absolutePath.toString() << std::endl;
		return false;
	}

	return true;
}

bool OpenFileStreamWrite(const FileHandler::CompletePath& filepath, std::ofstream& outStream)
{
	assert(!Project::isPathPointingInsideProjectFolder(filePath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(filePath);

	outStream.open(absolutePath.toString());
	if (!outStream.is_open())
	{
		std::cout << "error, can't load file at path : " << absolutePath.toString() << std::endl;
		return false;
	}
	return true;
}


template<typename T>
inline std::string valueAsString(const T& value)
{
	// Assume it is a vector by default

	std::stringstream ss;

	ss << "vec";
	ss << value.length();
	ss << '(';
	for (int i = 0; i < value.length(); i++)
	{
		ss << std::to_string(value[i]);
		if (i < value.length() - 1)
			ss << ", ";
	}
	ss << ')';

	return ss.str();
}

template<>
inline std::string valueAsString(const float& value)
{
	return std::to_string(value);
}

template<>
inline std::string valueAsString(const int& value)
{
	return std::to_string(value);
}

template<typename T>
inline std::string typeAsString()
{
	// Assume it is a vector by default
	T value;
	std::stringstream ss;
	ss << "vec";
	ss << value.length();

	return ss.str();
}

template<>
inline std::string typeAsString<float>()
{
	return "float";
}

template<typename EnumType>
EnumType stringToEnum(const std::string& enumAsString, std::vector<std::string>& enumToStringArray)
{
	auto foundEnumAsString = std::find(enumToStringArray.begin(), enumToStringArray.end(), enumAsString);
	assert(foundEnumAsString != enumToStringArray.end());
	int foundIdxEnumAsString = foundEnumAsString - enumToStringArray.begin();
	return (EnumType)foundIdxEnumAsString;
}

template<typename EnumType>
const std::string& enumToString(EnumType _enum, std::vector<std::string>& enumToStringArray)
{
	int enumAsInt = (int)_enum;
	assert(enumAsInt >= 0 && enumAsInt < enumToStringArray.size());
	return enumToStringArray[enumAsInt];
}

}


//for serialization : 

// TO JSON : 

template<typename T>
void jsonSaveValue(Json::Value& jsonValue, const std::string& key, const T& value)
{
	jsonValue[key] = toJsonValue<T>(value);
}

template<typename T>
void jsonLoadValue(const Json::Value& jsonValue, const std::string& key, T& value)
{
	value = fromJsonValue<T>(jsonValue[key]);
}


// TO JSON : 


template<typename T>
Json::Value toJsonValue(const T& value)
{
	Json::Value serializedValue;
		value.save(serializedValue);
	return serializedValue;
}

template<typename T>
Json::Value toJsonValue(const std::vector<T>& vector)
{
	Json::Value serializedValue;
	serializedValue["size"] = vector.size();
	for (int i = 0; i < vector.size(); i++)
	{
		serializedValue["data"][i] = toJsonValue<T>(vector[i]);
	}
	return serializedValue;
}

template<typename T, typename U>
Json::Value toJsonValue(const std::map<T, U>& map)
{
	Json::Value serializedValue;
	serializedValue["size"] = map.size();
	int i = 0;
	for (auto it = map.begin(); it != map.end(); it++)
	{
		serializedValue["keys"][i] = toJsonValue<T>(it.first);
		serializedValue["values"][i] = toJsonValue<T>(it.second);
		i++;
	}
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<GLuint>(const GLuint& value)
{
	return Json::Value((int)value);
}


template<>
inline Json::Value toJsonValue<float>(const float& value)
{
	return Json::Value(value);
}

template<>
inline Json::Value toJsonValue<int>(const int& value)
{
	return Json::Value(value);
}

template<>
inline Json::Value toJsonValue<std::string>(const std::string& value)
{
	return Json::Value(value);
}

template<>
inline Json::Value toJsonValue<bool>(const bool& value)
{
	return Json::Value(value);
}


template<>
inline Json::Value toJsonValue<glm::vec2>(const glm::vec2& vec)
{
	Json::Value serializedValue;
	serializedValue[0] = vec.x;
	serializedValue[1] = vec.y;
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<glm::ivec2>(const glm::ivec2& vec)
{
	Json::Value serializedValue;
	serializedValue[0] = vec.x;
	serializedValue[1] = vec.y;
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<glm::vec3>(const glm::vec3& vec)
{
	Json::Value serializedValue(Json::arrayValue);
	serializedValue[0] = vec.x;
	serializedValue[1] = vec.y;
	serializedValue[2] = vec.z;
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<glm::ivec3>(const glm::ivec3& vec)
{
	Json::Value serializedValue(Json::arrayValue);
	serializedValue[0] = vec.x;
	serializedValue[1] = vec.y;
	serializedValue[2] = vec.z;
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<glm::vec4>(const glm::vec4& vec)
{
	Json::Value serializedValue;
	serializedValue[0] = vec.x;
	serializedValue[1] = vec.y;
	serializedValue[2] = vec.z;
	serializedValue[3] = vec.w;
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<glm::ivec4>(const glm::ivec4& vec)
{
	Json::Value serializedValue;
	serializedValue[0] = vec.x;
	serializedValue[1] = vec.y;
	serializedValue[2] = vec.z;
	serializedValue[3] = vec.w;
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<glm::quat>(const glm::quat& quat)
{
	Json::Value serializedValue;
	serializedValue[0] = quat.x;
	serializedValue[1] = quat.y;
	serializedValue[2] = quat.z;
	serializedValue[3] = quat.w;
	return serializedValue;
}

template<>
inline Json::Value toJsonValue<glm::mat3>(const glm::mat3& mat)
{
	Json::Value formatedValue(Json::arrayValue);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			formatedValue.append(mat[i][j]);
		}
	}
	return formatedValue;
}

template<>
inline Json::Value toJsonValue<glm::mat4>(const glm::mat4& mat)
{
	Json::Value formatedValue(Json::arrayValue);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			formatedValue.append(mat[i][j]);
		}
	}
	return formatedValue;
}

// FROM JSON


template<typename T>
std::vector<T> fromJsonValues_vector(const Json::Value& value)
{
	std::vector<T> loadedValues;
	int size = value.get("size", 0).asInt();
	for (int i = 0; i < size; i++)
	{
		loadedValues[i] = fromJsonValue<T>(value["data"][i], T());
	}
	return loadedValues;
}

template<typename T, typename U>
std::map<T, U> fromJsonValues_map(const Json::Value& value)
{
	std::map<T> loadedValues;
	int size = value.get("size", 0).asInt();
	for (int i = 0; i < size; i++)
	{
		std::pair<T, U> newPair;
		newPair.first = fromJsonValue<T>(value["keys"][i]);
		newPair.second = fromJsonValue<T>(value["values"][i]);
		loadedValues.insert(newPair);
	}
	return loadedValues;
}

template<typename T>
T fromJsonValue(const Json::Value& value, const T& default)
{
	if (value.empty())
		return default;
	else
	{
		T loadedValue;
		loadedValue.load(value);
		return loadedValue;
	}
}

template<>
inline float fromJsonValue<float>(const Json::Value& value, const float& default)
{
	return value.asFloat();
}

template<>
inline GLuint fromJsonValue<GLuint>(const Json::Value& value, const GLuint& default)
{
	return (GLuint)value.asInt();
}

template<>
inline int fromJsonValue<int>(const Json::Value& value, const int& default)
{
	return value.asInt();
}

template<>
inline bool fromJsonValue<bool>(const Json::Value& value, const bool& default)
{
	return value.asBool();
}

template<>
inline std::string fromJsonValue<std::string>(const Json::Value& value, const std::string& default)
{
	return value.asString();
}

template<>
inline glm::quat fromJsonValue<glm::quat>(const Json::Value& value, const glm::quat& default)
{
	if (value.empty())
		return default;
	else
		return glm::quat(value[3].asFloat(), value[0].asFloat(), value[1].asFloat(), value[2].asFloat());
}

template<>
inline glm::vec2 fromJsonValue<glm::vec2>(const Json::Value& value, const glm::vec2& default)
{
	if (value.empty())
		return default;
	else
		return glm::vec2(value[0].asFloat(), value[1].asFloat());
}

template<>
inline glm::vec3 fromJsonValue<glm::vec3>(const Json::Value& value, const glm::vec3& default)
{
	if (value.empty())
		return default;
	else
		return glm::vec3(value[0].asFloat(), value[1].asFloat(), value[2].asFloat());
}

template<>
inline glm::vec4 fromJsonValue<glm::vec4>(const Json::Value& value, const glm::vec4& default)
{
	if (value.empty())
		return default;
	else
		return glm::vec4(value[0].asFloat(), value[1].asFloat(), value[2].asFloat(), value[3].asFloat());
}


template<>
inline glm::ivec2 fromJsonValue<glm::ivec2>(const Json::Value& value, const glm::ivec2& default)
{
	if (value.empty())
		return default;
	else
		return glm::ivec2(value[0].asFloat(), value[1].asFloat());
}

template<>
inline glm::ivec3 fromJsonValue<glm::ivec3>(const Json::Value& value, const glm::ivec3& default)
{
	if (value.empty())
		return default;
	else
		return glm::ivec3(value[0].asFloat(), value[1].asFloat(), value[2].asFloat());
}

template<>
inline glm::ivec4 fromJsonValue<glm::ivec4>(const Json::Value& value, const glm::ivec4& default)
{
	if (value.empty())
		return default;
	else
		return glm::ivec4(value[0].asFloat(), value[1].asFloat(), value[2].asFloat(), value[3].asFloat());
}

template<>
inline glm::mat3 fromJsonValue<glm::mat3>(const Json::Value& value, const glm::mat3& default)
{
	if (value.empty())
		return default;
	else
	{
		glm::mat3 matrix;
		for (int i = 0, k = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++, k++)
			{
				matrix[i][j] = value[k].asFloat();
			}
		}
		return matrix;
	}
}

template<>
inline glm::mat4 fromJsonValue<glm::mat4>(const Json::Value& value, const glm::mat4& default)
{
	if (value.empty())
		return default;
	else
	{
		glm::mat4 matrix;
		for (int i = 0, k = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++, k++)
			{
				matrix[i][j] = value[k].asFloat();
			}
		}
		return matrix;
	}
}

////////////////////

template<typename T>
T fromJsonValue(const Json::Value& value)
{
	assert(0 && "invalid value.");
	return T();
}

template<>
inline float fromJsonValue<float>(const Json::Value& value)
{
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return 0.f;
	}

	return value.asFloat();
}

template<>
inline GLuint fromJsonValue<GLuint>(const Json::Value& value)
{
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return 0;
	}

	return (GLuint)value.asInt();
}

template<>
inline int fromJsonValue<int>(const Json::Value& value)
{
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return 0;
	}

	return value.asInt();
}

template<>
inline bool fromJsonValue<bool>(const Json::Value& value)
{
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return false;
	}

	return value.asBool();
}

template<>
inline std::string fromJsonValue<std::string>(const Json::Value& value)
{
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return "";
	}

	return value.asString();
}

template<>
inline glm::quat fromJsonValue<glm::quat>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::quat();
	}

	return glm::quat(value[3].asFloat(), value[0].asFloat(), value[1].asFloat(), value[2].asFloat());
}

template<>
inline glm::vec2 fromJsonValue<glm::vec2>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::vec2();
	}

	return glm::vec2(value[0].asFloat(), value[1].asFloat());
}

template<>
inline glm::vec3 fromJsonValue<glm::vec3>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
		return glm::vec3();
	}

	return glm::vec3(value[0].asFloat(), value[1].asFloat(), value[2].asFloat());
}

template<>
inline glm::vec4 fromJsonValue<glm::vec4>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::vec4();
	}

	return glm::vec4(value[0].asFloat(), value[1].asFloat(), value[2].asFloat(), value[3].asFloat());
}


template<>
inline glm::ivec2 fromJsonValue<glm::ivec2>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::ivec2();
	}

	return glm::ivec2(value[0].asFloat(), value[1].asFloat());
}

template<>
inline glm::ivec3 fromJsonValue<glm::ivec3>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::ivec3();
	}

	return glm::ivec3(value[0].asFloat(), value[1].asFloat(), value[2].asFloat());
}

template<>
inline glm::ivec4 fromJsonValue<glm::ivec4>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::ivec4();
	}

	return glm::ivec4(value[0].asFloat(), value[1].asFloat(), value[2].asFloat(), value[3].asFloat());
}

template<>
inline glm::mat3 fromJsonValue<glm::mat3>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::mat3(1);
	}

	glm::mat3 matrix;
	for (int i = 0, k = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++, k++)
		{
			matrix[i][j] = value[k].asFloat();
		}
	}
	return matrix;
}

template<>
inline glm::mat4 fromJsonValue<glm::mat4>(const Json::Value& value)
{
	//assert(!value.empty());
	if (value.empty())
	{
		PRINT_WARNING("Can't load the value from JSonValue");
			return glm::mat4();
	}

	glm::mat4 matrix;
	for (int i = 0, k = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++, k++)
		{
			matrix[i][j] = value[k].asFloat();
		}
	}
	return matrix;
}


