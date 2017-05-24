#pragma once

//#include <glew/glew.h>

#include "EditorGUI.h"
#include "ISerializable.h"

#include "ResourcePointer.h"
#include "Texture.h"
#include "ICloneable.h"


//The ShaderParameter interface. 
//A ShaderParameter is an abstraction of a data which can be send to gpu.
class InternalShaderParameterBase : public ISerializable, public ICloneable<InternalShaderParameterBase>
{
protected:
	std::string m_name;

public:
	InternalShaderParameterBase(const std::string& name)
		: m_name(name)
	{}
	virtual ~InternalShaderParameterBase() {}
	virtual void init(GLuint glProgramId) = 0;
	virtual void drawUI() = 0;
	virtual void pushToGPU(int& boundTextureCount) const = 0;
	const std::string& getName() const
	{
		return m_name;
	}

	virtual void setData(const void* data)
	{
		assert(0 && "error, invalid setter function.");
	}
	virtual void getData(void* outData)
	{
		assert(0 && "error, invalid getter function.");
	}

	virtual std::string valueAsString() const
	{
		assert(false && "the value can't be represented as a string.");
		return "";
	}
};

//The ShaderParameter interface. 
//A ShaderParameter is an abstraction of a data which can be send to gpu.
class ExternalShaderParameterBase
{
protected:
	std::string m_name;
public:
	ExternalShaderParameterBase(const std::string& name)
		: m_name(name)
	{}
	virtual ~ExternalShaderParameterBase() {}
	virtual void init(GLuint glProgramId) = 0;
	const std::string& getName() const
	{
		return m_name;
	}
	virtual GLuint getUniformId() const { return 0; }
	virtual void getUniformIds(std::vector<GLuint>& outUniforms) const { return; }
};


// Helper Functions to push data to gpu
namespace GlHelper {

template<typename T>
void pushParameterToGPU(GLuint uniformId, const T& value)
{
	assert(0 && "invalid parameter type.");
}

template<>
void pushParameterToGPU<bool>(GLuint uniformId, const bool& value);

template<>
void pushParameterToGPU<int>(GLuint uniformId, const int& value);

template<>
void pushParameterToGPU<float>(GLuint uniformId, const float& value);

template<>
void pushParameterToGPU<glm::vec2>(GLuint uniformId, const glm::vec2& value);

template<>
void pushParameterToGPU<glm::ivec2>(GLuint uniformId, const glm::ivec2& value);

template<>
void pushParameterToGPU<glm::vec3>(GLuint uniformId, const glm::vec3& value);

template<>
void pushParameterToGPU<glm::ivec3>(GLuint uniformId, const glm::ivec3& value);

template<>
void pushParameterToGPU<glm::vec4>(GLuint uniformId, const glm::vec4& value);

template<>
void pushParameterToGPU<glm::ivec4>(GLuint uniformId, const glm::ivec4& value);

template<>
void pushParameterToGPU<glm::mat4>(GLuint uniformId, const glm::mat4& value);

//array version (experimental) : 

template<typename T>
void pushParametersToGPU(GLuint uniformId, int count, const std::vector<T>& values)
{
	assert(0 && "invalid parameter type.");
}


template<>
void pushParametersToGPU<float>(GLuint uniformId, int count, const std::vector<float>& values);

template<>
void pushParametersToGPU<glm::vec3>(GLuint uniformId, int count, const std::vector<glm::vec3>& values);

//TODO : complete

}


namespace ShaderParameter {

//valid types for a shader parameter.
enum ShaderParameterType {
	INT,
	INT2,
	INT3,
	INT4,
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	TEXTURE,
	CUBE_TEXTURE,
	//arrays :
	ARRAY_INT,
	ARRAY_INT2,
	ARRAY_INT3,
	ARRAY_FLOAT,
	ARRAY_FLOAT2,
	ARRAY_FLOAT3,
	ARRAY_TEXTURE,
	ARRAY_CUBE_TEXTURE,
	TYPE_COUNT
};

//these types will appear with these names in .glProg files.
static std::vector<std::string> LiteralShaderParameterType{
	"int",
	"int2",
	"int3",
	"int4",
	"float",
	"float2",
	"float3",
	"float4",
	"texture",
	"cubeTexture",
	//arrays :
	"int[]",
	"int2[]",
	"int3[]",
	"float[]",
	"float2[]",
	"float2[]",
	"texture[]",
	"cubeTexture[]",
};

struct IsArray {};
struct IsNotArray {};

}


//////////////////////////////////////////////////////////////////////////////////////////
////////// BEGIN : internal parameters


template<typename T, typename U>
class InternalShaderParameter : public InternalShaderParameterBase
{
	//UNDEFINED FOR U != (IsArray | IsNotArray) !!!
};

template<typename T>
class InternalShaderParameter<T, ShaderParameter::IsNotArray> : public InternalShaderParameterBase
{
private:
	T m_data;
	GLuint m_uniformId;
	bool m_isEditable;
	EditorGUI::FieldDisplayType m_displayType;

public:
	InternalShaderParameter(const std::string& name, bool isEditable, const T& defaultValue, const EditorGUI::FieldDisplayType& displayType)
		: InternalShaderParameterBase(name)
		, m_isEditable(isEditable)
		, m_uniformId(0)
		, m_displayType(displayType)
	{
		m_data = defaultValue;
	}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void drawUI() override
	{
		if (m_isEditable)
			EditorGUI::ValueField<T>(m_name, m_data, m_displayType); //TODO 10
	}

	void pushToGPU(int& boundTextureCount) const override
	{
		GlHelper::pushParameterToGPU<T>(m_uniformId, m_data);
	}

	void save(Json::Value & entityRoot) const override
	{
		jsonSaveValue<T>(entityRoot, "data", m_data);
	}
	void load(const Json::Value & entityRoot) override
	{
		jsonLoadValue<T>(entityRoot, "data", m_data);
	}

	void setData(const void* data) override
	{
		m_data = *static_cast<const T*>(data);
	}
	void getData(void* outData) override
	{
		outData = (void*)(&m_data);
	}

	virtual std::string valueAsString() const override
	{
		return Utils::valueAsString(m_data);
	}

	virtual InternalShaderParameterBase* clone() const override
	{
		return new InternalShaderParameter<T, ShaderParameter::IsNotArray>(*this);
	}
	virtual std::shared_ptr<InternalShaderParameterBase> cloneShared() const override
	{
		return std::make_shared<InternalShaderParameter<T, ShaderParameter::IsNotArray>>(*this);
	}

};

//array version isn't editable for now
//array version, multiple uniforms and data. By convention the name of uniform[i] is m_name[i]
template<typename T>
class InternalShaderParameter<T, ShaderParameter::IsArray> : public InternalShaderParameterBase
{
private:
	std::vector<GLuint> m_uniformId;
	std::vector<T> m_datas;

public:
	InternalShaderParameter(const std::string& name)
		: InternalShaderParameterBase(name)
	{}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		for (int i = 0; i < m_datas.size(); i++)
			m_uniformIds[i] = glGetUniformLocation(glProgram, (m_name+"[" + std::to_string(i) + "]").c_str());
	}

	void drawUI() override
	{
		//Not editable for now. TODO
		//EditorGUI::ValueField<T>(m_name, m_data);
	}

	void pushToGPU(int& boundTextureCount) const override
	{
		for (int i = 0; i < m_datas.size(); i++)
		{
			GlHelper::pushParameterToGPU<T>(m_uniformIds[i], m_datas[i]);
		}
	}

	void save(Json::Value & entityRoot) const override
	{
		entityRoot["count"] = m_datas.size();
		for(int i = 0; i < m_datas.size(); i ++)
			jsonSaveValue<T>(entityRoot, "data["+i+"]", m_datas[i]);
	}
	void load(const Json::Value & entityRoot) override
	{
		int count = entityRoot["count"];
		m_datas.resize(count);
		for (int i = 0; i < m_datas.size(); i++)
			jsonLoadValue<T>(entityRoot, "data["+i+"]", m_datas[i]);
	}

	void setData(const void* data) override
	{
		m_data = *static_cast<std::vector<T>*>(data);
	}
	void getData(void* outData) override
	{
		outData = (void*)(&m_data);
	}

	virtual InternalShaderParameterBase* clone() const override
	{
		return new InternalShaderParameter<T, ShaderParameter::IsArray>(*this);
	}
	virtual std::shared_ptr<InternalShaderParameterBase> cloneShared() const override
	{
		return std::make_shared<InternalShaderParameter<T, ShaderParameter::IsArray>>(*this);
	}
};

//Array version not allowed for now
template<>
class InternalShaderParameter<Texture, ShaderParameter::IsNotArray> : public InternalShaderParameterBase
{
private:
	ResourcePtr<Texture> m_data;
	GLuint m_uniformId;
	bool m_isEditable;
	EditorGUI::FieldDisplayType m_displayType;

public:
	InternalShaderParameter(const std::string& name, bool isEditable, ResourcePtr<Texture> defaultValue, const EditorGUI::FieldDisplayType& displayType);

	//init unifom id
	void init(GLuint glProgramId);
	void drawUI() override;
	void pushToGPU(int& boundTextureCount) const override;
	void save(Json::Value & entityRoot) const override;
	void load(const Json::Value & entityRoot) override;
	void setData(const void* data) override;
	void getData(void* outData) override;
	virtual InternalShaderParameterBase* clone() const override
	{
		return new InternalShaderParameter<Texture, ShaderParameter::IsNotArray>(*this);
	}
	virtual std::shared_ptr<InternalShaderParameterBase> cloneShared() const override
	{
		return std::make_shared<InternalShaderParameter<Texture, ShaderParameter::IsNotArray>>(*this);
	}
};

//Array version not allowed for now
template<>
class InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray> : public InternalShaderParameterBase
{
private:
	ResourcePtr<CubeTexture> m_data;
	GLuint m_uniformId;
	bool m_isEditable;
	EditorGUI::FieldDisplayType m_displayType;

public:
	InternalShaderParameter(const std::string& name, bool isEditable, ResourcePtr<CubeTexture> defaultValue, const EditorGUI::FieldDisplayType& displayType);
	void init(GLuint glProgramId);
	void drawUI() override;
	void pushToGPU(int& boundTextureCount) const override;
	void save(Json::Value & entityRoot) const override;
	void load(const Json::Value & entityRoot) override;
	void setData(const void* data) override;
	void getData(void* outData) override;
	virtual InternalShaderParameterBase* clone() const override
	{
		return new InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>(*this);
	}
	virtual std::shared_ptr<InternalShaderParameterBase> cloneShared() const override
	{
		return std::make_shared<InternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray>>(*this);
	}
};

//utility function to make a shader from its type
std::shared_ptr<InternalShaderParameterBase> MakeNewInternalShaderParameter(const Json::Value& parameterAsJsonValue);

//
//template<>
//InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::InternalShaderParameter(const std::string& name);
//template<>
//void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::init(GLuint glProgramId);
//template<>
//void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::drawUI() override;
//template<>
//void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::pushToGPU(int& boundTextureCount) override;
//template<>
//void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::save(Json::Value & entityRoot) const override;
//template<>
//void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::load(const Json::Value & entityRoot) override;
//template<>
//void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::setData(const void* data) override;
//template<>
//void InternalShaderParameter<Texture, ShaderParameter::IsNotArray>::getData(void* outData) override;
//

////////// END : internal parameters
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
////////// BEGIN : external parameters

template<typename T, typename U = ShaderParameter::IsNotArray>
class ExternalShaderParameter : public ExternalShaderParameterBase
{
	//UNDEFINED FOR U != (IsArray | IsNotArray) !!!
public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
	{}
};

template<typename T>
class ExternalShaderParameter<T, ShaderParameter::IsNotArray> : public ExternalShaderParameterBase
{
private:
	GLuint m_uniformId;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void pushToGPU(T& externalData) const
	{
		GlHelper::pushParameterToGPU<T>(m_uniformId, externalData);
	}

	GLuint getUniformId() const override
	{ 
		return m_uniformId;
	}

};

//array version isn't editable for now
//array version, multiple uniforms and data. By convention the name of uniform[i] is m_name[i]
template<typename T>
class ExternalShaderParameter<T, ShaderParameter::IsArray> : public ExternalShaderParameterBase
{
private:
	std::vector<GLuint> m_uniformIds;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
	{}

	int getUniformCount() const
	{
		return m_uniformIds.size();
	}

	//init unifom id
	void init(GLuint glProgramId) override
	{
		for (int i = 0; i < m_data.size(); i++)
			m_uniformIds[i] = glGetUniformLocation(glProgram, (m_name + "[" + std::to_string(i) + "]").c_str());
	}

	void pushToGPU(std::vector<T&>& externalDatas) const override
	{
		for (int i = 0; i < m_datas.size(); i++)
		{
			GlHelper::pushParameterToGPU<T>(m_uniformIds[i], externalDatas[i]);
		}
	}

	void pushToGPU(T& externalData, int uniformIdx) const override
	{
		assert(uniformIdx < m_uniformIds.size());
		GlHelper::pushParameterToGPU<T>(m_uniformIds[uniformIdx], externalData[uniformIdx]);
	}

	void getUniformIds(std::vector<GLuint>& outUniforms) const override
	{
		outUniforms = m_uniformIds;
	}

};

//Array version not allowed for now
template<>
class ExternalShaderParameter<Texture, ShaderParameter::IsNotArray> : public ExternalShaderParameterBase
{
private:
	GLuint m_uniformId;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId)
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void pushToGPU(GLint textureUnit) const
	{
		glUniform1i(m_uniformId, textureUnit);
	}

	GLuint getUniformId() const override
	{
		return m_uniformId;
	}
};

//Array version not allowed for now
template<>
class ExternalShaderParameter<CubeTexture, ShaderParameter::IsNotArray> : public ExternalShaderParameterBase
{
private:
	GLuint m_uniformId;

public:
	ExternalShaderParameter(const std::string& name)
		: ExternalShaderParameterBase(name)
		, m_uniformId(0)
	{}

	//init unifom id
	void init(GLuint glProgramId)
	{
		m_uniformId = glGetUniformLocation(glProgramId, m_name.data());
	}

	void pushToGPU(GLint textureUnit) const
	{
		glUniform1i(m_uniformId, textureUnit);
	}

	GLuint getUniformId() const override
	{
		return m_uniformId;
	}
};

//utility function to make a shader from its type
std::shared_ptr<ExternalShaderParameterBase> MakeNewExternalShaderParameter(const std::string& literaltype, std::string& name);

////////// END : internal parameters
//////////////////////////////////////////////////////////////////////////////////////////



