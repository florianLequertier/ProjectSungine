#include <memory>
#include "glm/common.hpp"
#include "glm/glm.hpp"

#include "ShaderParameters.h"
#include "Factories.h" //forward


namespace GlHelper {

template<>
void pushParameterToGPU<bool>(GLuint uniformId, const bool& value)
{
	glUniform1i(uniformId, value);
}

template<>
void pushParameterToGPU<int>(GLuint uniformId, const int& value)
{
	glUniform1i(uniformId, value);
}

template<>
void pushParameterToGPU<float>(GLuint uniformId, const float& value)
{
	glUniform1f(uniformId, value);
}

template<>
void pushParameterToGPU<glm::vec2>(GLuint uniformId, const glm::vec2& value)
{
	glUniform2f(uniformId, value.x, value.y);
}

template<>
void pushParameterToGPU<glm::ivec2>(GLuint uniformId, const glm::ivec2& value)
{
	glUniform2i(uniformId, value.x, value.y);
}

template<>
void pushParameterToGPU<glm::vec3>(GLuint uniformId, const glm::vec3& value)
{
	glUniform3f(uniformId, value.x, value.y, value.z);
}

template<>
void pushParameterToGPU<glm::ivec3>(GLuint uniformId, const glm::ivec3& value)
{
	glUniform3i(uniformId, value.x, value.y, value.z);
}

template<>
void pushParameterToGPU<glm::vec4>(GLuint uniformId, const glm::vec4& value)
{
	glUniform4f(uniformId, value.x, value.y, value.z, value.w);
}

template<>
void pushParameterToGPU<glm::ivec4>(GLuint uniformId, const glm::ivec4& value)
{
	glUniform4i(uniformId, value.x, value.y, value.z, value.w);
}

template<>
void pushParameterToGPU<glm::mat4>(GLuint uniformId, const glm::mat4& value)
{
	glUniformMatrix4fv(uniformId, 1, false, glm::value_ptr(value));
}

//array version (experimental) : 

template<>
void pushParametersToGPU<float>(GLuint uniformId, int count, const std::vector<float>& values)
{
	glUniform1fv(uniformId, count, &values[0]);
}

template<>
void pushParametersToGPU<glm::vec3>(GLuint uniformId, int count, const std::vector<glm::vec3>& values)
{
	glUniform3fv(uniformId, count, &values[0][0]);
}

//TODO : complete

}


//utility function to make a shader from its type
std::shared_ptr<InternalShaderParameterBase> MakeNewInternalShaderParameter(const Json::Value& parameterAsJsonValue)
{
	std::string literaltype = parameterAsJsonValue.get("type", "").asString();
	std::string name = parameterAsJsonValue.get("name", "").asString();
	bool isEditable = parameterAsJsonValue.get("editable", true).asBool();
	std::string displayTypeAsString = parameterAsJsonValue.get("displayType", "default").asString();

	EditorGUI::FieldDisplayType displayType = EditorGUI::FieldDisplayType::DEFAULT;
	auto foundDisplayTypeIt = std::find(EditorGUI::LiteralFieldDisplayType.begin(), EditorGUI::LiteralFieldDisplayType.end(), displayTypeAsString);
	if (foundDisplayTypeIt != EditorGUI::LiteralFieldDisplayType.end())
		displayType = (EditorGUI::FieldDisplayType)std::distance(EditorGUI::LiteralFieldDisplayType.begin(), foundDisplayTypeIt);

	ShaderParameter::ShaderParameterType parameterType = ShaderParameter::ShaderParameterType::TYPE_COUNT;
	auto foundTypeIt = std::find(ShaderParameter::LiteralShaderParameterType.begin(), ShaderParameter::LiteralShaderParameterType.end(), literaltype);
	if (foundTypeIt != ShaderParameter::LiteralShaderParameterType.end())
		parameterType = (ShaderParameter::ShaderParameterType)std::distance(ShaderParameter::LiteralShaderParameterType.begin(), foundTypeIt);
	else
		return nullptr;


	switch (parameterType)
	{
	case ShaderParameter::ShaderParameterType::INT:
	{
		//int defaultValue = parameterAsJsonValue.get("default", 0).asInt();
		//return std::make_shared<InternalShaderParameter<int, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<int, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::INT2:
	{
		//glm::ivec2 defaultValue(0, 0);
		//Json::Value arrayValue = parameterAsJsonValue.get("default", Json::Value(Json::ValueType::arrayValue));
		//if (arrayValue.size() == 2)
		//{
		//	defaultValue.x = arrayValue[0].asInt();
		//	defaultValue.y = arrayValue[1].asInt();
		//}
		//return std::make_shared<InternalShaderParameter<glm::ivec2, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<glm::ivec2, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::INT3:
	{
		//glm::ivec3 defaultValue(0, 0, 0);
		//Json::Value arrayValue = parameterAsJsonValue.get("default", Json::Value(Json::ValueType::arrayValue));
		//if (arrayValue.size() == 3)
		//{
		//	defaultValue.x = arrayValue[0].asInt();
		//	defaultValue.y = arrayValue[1].asInt();
		//	defaultValue.z = arrayValue[2].asInt();
		//}
		//return std::make_shared<InternalShaderParameter<glm::ivec3, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<glm::ivec3, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::INT4:
	{
		//glm::ivec4 defaultValue(0, 0, 0, 0);
		//Json::Value arrayValue = parameterAsJsonValue.get("default", Json::Value(Json::ValueType::arrayValue));
		//if (arrayValue.size() == 4)
		//{
		//	defaultValue.x = arrayValue[0].asInt();
		//	defaultValue.y = arrayValue[1].asInt();
		//	defaultValue.z = arrayValue[2].asInt();
		//	defaultValue.w = arrayValue[3].asInt();
		//}
		//return std::make_shared<InternalShaderParameter<glm::ivec4, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<glm::ivec4, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::FLOAT:
	{
		//float defaultValue = parameterAsJsonValue.get("default", 0).asFloat();
		//return std::make_shared<InternalShaderParameter<float, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<float, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::FLOAT2:
	{
		//glm::vec2 defaultValue(0.f, 0.f);
		//Json::Value arrayValue = parameterAsJsonValue.get("default", Json::Value(Json::ValueType::arrayValue));
		//if (arrayValue.size() == 2)
		//{
		//	defaultValue.x = arrayValue[0].asFloat();
		//	defaultValue.y = arrayValue[1].asFloat();
		//}
		//return std::make_shared<InternalShaderParameter<glm::vec2, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<glm::vec2, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::FLOAT3:
	{
		//glm::vec3 defaultValue(0.f, 0.f, 0.f);
		//Json::Value arrayValue = parameterAsJsonValue.get("default", Json::Value(Json::ValueType::arrayValue));
		//if (arrayValue.size() == 3)
		//{
		//	defaultValue.x = arrayValue[0].asFloat();
		//	defaultValue.y = arrayValue[1].asFloat();
		//	defaultValue.z = arrayValue[2].asFloat();
		//}
		//return std::make_shared<InternalShaderParameter<glm::vec3, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<glm::vec3, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::FLOAT4:
	{
		//glm::vec4 defaultValue(0.f, 0.f, 0.f, 0.f);
		//Json::Value arrayValue = parameterAsJsonValue.get("default", Json::Value(Json::ValueType::arrayValue));
		//if (arrayValue.size() == 4)
		//{
		//	defaultValue.x = arrayValue[0].asFloat();
		//	defaultValue.y = arrayValue[1].asFloat();
		//	defaultValue.z = arrayValue[2].asFloat();
		//	defaultValue.w = arrayValue[3].asFloat();
		//}
		//return std::make_shared<InternalShaderParameter<glm::vec4, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<glm::vec4, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::TEXTURE:
	{
		//ResourcePtr<Texture> defaultValue = getTextureFactory().getDefault(parameterAsJsonValue.get("default", "default").asString());
		//return std::make_shared<InternalShaderParameter<Texture, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<Texture, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	case ShaderParameter::ShaderParameterType::CUBE_TEXTURE:
	{
		//ResourcePtr<CubeTexture> defaultValue = getCubeTextureFactory().getDefault(parameterAsJsonValue.get("default", "default").asString());
		//return std::make_shared<InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>>(name, isEditable, defaultValue, displayType);

		auto newParameter = std::make_shared<InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>>();
		newParameter->load(parameterAsJsonValue);
		return newParameter;
		break;
	}
	default:
		return nullptr;
		break;
	}
}


//utility function to make a shader from its type
std::shared_ptr<ExternalShaderParameterBase> MakeNewExternalShaderParameter(const std::string& literaltype, std::string& name)
{
	ShaderParameter::ShaderParameterType parameterType = ShaderParameter::ShaderParameterType::TYPE_COUNT;
	auto foundTypeIt = std::find(ShaderParameter::LiteralShaderParameterType.begin(), ShaderParameter::LiteralShaderParameterType.end(), literaltype);
	if (foundTypeIt != ShaderParameter::LiteralShaderParameterType.end())
		parameterType = (ShaderParameter::ShaderParameterType)std::distance(ShaderParameter::LiteralShaderParameterType.begin(), foundTypeIt);
	else
		return nullptr;


	switch (parameterType)
	{
	case ShaderParameter::ShaderParameterType::INT:
		return std::make_shared<ExternalShaderParameter<int, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::INT2:
		return std::make_shared<ExternalShaderParameter<glm::ivec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::INT3:
		return std::make_shared<ExternalShaderParameter<glm::ivec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT:
		return std::make_shared<ExternalShaderParameter<float, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT2:
		return std::make_shared<ExternalShaderParameter<glm::vec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::FLOAT3:
		return std::make_shared<ExternalShaderParameter<glm::vec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::TEXTURE:
		return std::make_shared<ExternalShaderParameter<Texture, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::CUBE_TEXTURE:
		return std::make_shared<ExternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>>(name);
		break;
		//array versions :
	case ShaderParameter::ShaderParameterType::ARRAY_INT:
		return std::make_shared<ExternalShaderParameter<int, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_INT2:
		return std::make_shared<ExternalShaderParameter<glm::ivec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_INT3:
		return std::make_shared<ExternalShaderParameter<glm::ivec3, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_FLOAT:
		return std::make_shared<ExternalShaderParameter<float, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_FLOAT2:
		return std::make_shared<ExternalShaderParameter<glm::vec2, ShaderParameter::IsNotArray>>(name);
		break;
	case ShaderParameter::ShaderParameterType::ARRAY_FLOAT3:
		return std::make_shared<ExternalShaderParameter<glm::vec3, ShaderParameter::IsNotArray>>(name);
		break;
	default:
		return nullptr;
		break;
	}
}

InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::InternalShaderParameter(const std::string& name, bool isEditable, const AssetHandle<Texture>& defaultValue, const EditorGUI::FieldDisplayType& displayType)
	: InternalShaderParameterBase(name)
	, m_uniformId(-1)
	, m_isEditable(isEditable)
	, m_data(defaultValue)
	, m_displayType(displayType)
{}

//init unifom id
void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::init(GLuint glProgramId)
{
	m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
}

void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::drawUI()
{
	if (m_isEditable)
		EditorGUI::ResourceField<Texture>(m_name, m_data); //TODO 10
}

void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::pushToGPU(int& boundTextureCount) const
{
	if (!m_data.isValid())
		return;

	glActiveTexture(GL_TEXTURE0 + boundTextureCount);
	glBindTexture(GL_TEXTURE_2D, m_data->glId);
	glUniform1i(m_uniformId, boundTextureCount);
	boundTextureCount++;
}

void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::save(Json::Value & root) const
{
	root["type"] = ShaderParameter::LiteralShaderParameterType[(int)ShaderParameter::ShaderParameterType::TEXTURE];
	root["name"] = m_name;
	root["default"] = "default";
	m_data.save(root["value"]);
}

void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::load(const Json::Value & root)
{
	m_name = root.get("name", "").asString();
	if (root.isMember("value"))
	{
		m_data.load(root["value"]);
	}
	else
	{
		m_data.load(root["default"]);
	}
}

void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::setData(const void* data)
{
	m_data = static_cast<const Texture*>(data);
}

void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::getData(void* outData)
{
	outData = (void*)(m_data.get());
}


// Cube texture : 

InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::InternalShaderParameter(const std::string& name, bool isEditable, const AssetHandle<CubeTexture>& defaultValue, const EditorGUI::FieldDisplayType& displayType)
	: InternalShaderParameterBase(name)
	, m_uniformId(-1)
	, m_isEditable(isEditable)
	, m_data(defaultValue)
	, m_displayType(displayType)
{}

//init unifom id
void InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::init(GLuint glProgramId)
{
	m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	//m_data = getCubeTextureFactory().getDefault("default");
}

void InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::drawUI()
{
	if(m_isEditable)
		EditorGUI::ResourceField<CubeTexture>(m_name, m_data); //TODO 10
}

void InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::pushToGPU(int& boundTextureCount) const
{
	glActiveTexture(GL_TEXTURE0 + boundTextureCount);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_data->glId);
	glUniform1i(m_uniformId, boundTextureCount);
	boundTextureCount++;
}

void InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::save(Json::Value & root) const
{
	root["type"] = ShaderParameter::LiteralShaderParameterType[(int)ShaderParameter::ShaderParameterType::CUBE_TEXTURE];
	root["name"] = m_name;
	root["default"] = "default";
	m_data.save(root["value"]);
}

void InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::load(const Json::Value & root)
{
	m_name = root.get("name", "").asString();
	if (root.isMember("value"))
	{
		m_data.load(root["value"]);
	}
	else
	{
		m_data.load(root["default"]);
	}
}

void InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::setData(const void* data)
{
	m_data = static_cast<const CubeTexture*>(data);
}
void InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>::getData(void* outData)
{
	outData = (void*)(&m_data);
}

// Specializations : 
// Only not array version for now
#define SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(type, shaderParamTypeEnum)\
void InternalShaderParameter<type, ShaderParameter::IsNotArray>::save(Json::Value & root) const\
{\
	root["type"] = ShaderParameter::LiteralShaderParameterType[(int)shaderParamTypeEnum];\
	root["name"] = m_name;\
	root["default"] = "default";\
	root["value"] = toJsonValue<type>(m_data);\
}\
void InternalShaderParameter<type, ShaderParameter::IsNotArray>::load(const Json::Value & root)\
{\
	m_name = root.get("name", "").asString();\
	if (root.isMember("value"))\
	{\
		m_data = fromJsonValue<type>(root["value"]);\
	}\
	else\
	{\
		m_data = fromJsonValue<type>(root["default"]);\
	}\
}

SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(float, ShaderParameter::ShaderParameterType::FLOAT)
SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(glm::vec2, ShaderParameter::ShaderParameterType::FLOAT2)
SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(glm::vec3, ShaderParameter::ShaderParameterType::FLOAT3)
SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(glm::vec4, ShaderParameter::ShaderParameterType::FLOAT4)
SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(int, ShaderParameter::ShaderParameterType::INT)
SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(glm::ivec2, ShaderParameter::ShaderParameterType::INT2)
SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(glm::ivec3, ShaderParameter::ShaderParameterType::INT3)
SHADER_PARAMETER_SPECIALISATION_NOT_ARRAY(glm::ivec4, ShaderParameter::ShaderParameterType::INT4)
