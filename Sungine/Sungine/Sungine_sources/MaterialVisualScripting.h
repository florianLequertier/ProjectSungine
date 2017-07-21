#pragma once

#include <assert.h>
#include <vector>
#include <sstream>
#include <memory>
#include <unordered_map>

#include "glm/common.hpp"

#include "ShaderParameters.h"
#include "EditorNodes.h"
#include "MeshVisualizer.h"

class Mesh;
class Material;

namespace MVS {

class NodeManager;
struct Node;
struct Link;
struct Input;
struct Output;

enum class NodeType {
	OPERATOR,
	PARAMETER,
	FUNCTION,
	FINAL,
	CUSTOM,
};

static std::vector<std::string> NodeTypeToString = {
	"operator",
	"parameter",
	"function",
	"final",
	"custom",
};

enum class ParameterType {
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	TEXTURE,
};

enum class FlowType {
	UNDEFINED = 0,
	FLOAT = 1,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	CUSTOM,
};

struct CompilationErrorCheck
{
	bool compilationFailed;
	std::string errorMsg;

	CompilationErrorCheck()
		: compilationFailed(false)
		, errorMsg("")
	{}
};

struct Input
{
	std::string name;
	int outputIdx;
	FlowType desiredType;

	Node* parentNode;
	Link* link;
	glm::vec2 position;

	Input(Node* _parentNode, const std::string& _name, FlowType _desiredType);
	void resolveUndeterminedTypes(FlowType _desiredType);
	void compile(std::stringstream& stream, CompilationErrorCheck& errorCheck);
};

struct Output
{
	std::string name;
	FlowType outputType;
	std::string valueStr;

	Node* parentNode;
	std::vector<Link*> links;
	glm::vec2 position;

	Output(Node* _parentNode, const std::string& _name, FlowType _outputType);
};

void formatAndOutputResult(std::stringstream& stream, const Output& output, FlowType desiredType, CompilationErrorCheck& errorCheck);

void printDefaultValue(std::stringstream& nodeCompileResult, FlowType desiredType);

#define INHERIT_FROM_NODE(RealNodeType)\
virtual std::shared_ptr<Node> cloneShared() override\
{\
	return std::make_shared<RealNodeType>(*this);\
}

struct Node 
{
	NodeType type;
	std::string name;
	ID nodeID;

	std::vector<std::shared_ptr<Input>> inputs;
	std::vector<std::shared_ptr<Output>> outputs;

	glm::vec2 position;
	glm::vec2 size;

	virtual void onBeforeCompile(CompilationErrorCheck& errorCheck) {};
	virtual void defineParameter(std::stringstream& stream, std::vector<std::string>& usedParameterNames, CompilationErrorCheck& errorCheck);
	virtual void compile(CompilationErrorCheck& errorCheck) = 0;

	Node(NodeType _type, const std::string& _name);
	Node();
	Node(const Node& other);
	Node& operator=(const Node& other);

	void resolveUndeterminedTypes();
	virtual std::shared_ptr<Node> cloneShared() = 0;
	
	void save(Json::Value& root) const;
	void load(const Json::Value& root);

	void drawUI(NodeManager& manager);
	virtual void drawUIParameters() {}
	void setPosition(const glm::vec2& pos);
	const glm::vec2& getPosition() const;
};

struct Link
{
	ID inputNodeID;
	int inputIdx;
	ID outputNodeID;
	int outputIdx;

	Output* input;
	Input* output;

	Link(Output* _input, Input* _output);

	void save(Json::Value& root) const;
	void load(const Json::Value& root);

	void drawUI(NodeManager& manager);
};

class NodeFactory : public ISingleton<NodeFactory>
{
	SINGLETON_IMPL(NodeFactory);

private:
	std::vector<std::shared_ptr<Node>> m_container;

public:

	NodeFactory()
	{}

	bool registerNode(const std::string& nodeName, std::shared_ptr<Node> node)
	{
		if (std::find_if(m_container.begin(), m_container.end(), [&nodeName](const std::shared_ptr<Node>& item) { return item->name == nodeName; }) != m_container.end())
			return false;

		auto it = std::lower_bound(m_container.begin(), m_container.end(), node, [](const std::shared_ptr<Node>& itemA, const std::shared_ptr<Node>& itemB) { return (int)itemA->type > (int)itemB->type; });
		m_container.insert(it, node);
		return true;
	}

	std::shared_ptr<Node> getSharedNodeInstance(const std::string& nodeName)
	{
		assert(std::find_if(m_container.begin(), m_container.end(), [&nodeName](const std::shared_ptr<Node>& item) { return item->name == nodeName; }) != m_container.end());

		auto foundIt = std::find_if(m_container.begin(), m_container.end(), [&nodeName](const std::shared_ptr<Node>& item) { return item->name == nodeName; });
		return (*foundIt)->cloneShared();
	}

	std::vector<std::shared_ptr<Node>>::const_iterator begin() const
	{
		return m_container.begin();
	}

	std::vector<std::shared_ptr<Node>>::const_iterator end() const
	{
		return m_container.end();
	}
};

#define REGISTER_NODE(NodeName, NodeType) const static bool initialized##NodeType = NodeFactory::instance().registerNode(NodeName, std::make_shared<NodeType>());
#define REGISTER_TEMPLATED_NODE(NodeName, NodeType, templateType, uniqueName) const static bool initialized##uniqueName = NodeFactory::instance().registerNode(NodeName, std::make_shared<NodeType<templateType>>());

struct FinalNode : public Node
{
private:
	glm::vec3 defaultDiffuse;
	float defaultSpecular;
	glm::vec3 defaultEmissive;
	glm::vec3 defaultNormals;
	float defaultSpecularPower;

public:
	std::string finalValueStr;

	INHERIT_FROM_NODE(FinalNode)

	FinalNode()
		: Node(NodeType::FINAL, "result")
	{
		defaultDiffuse = glm::vec3(1, 1, 1);
		defaultSpecular = 0.5f;
		defaultEmissive = glm::vec3(0, 0, 0);
		defaultNormals = glm::vec3(0, 0, 1);
		defaultSpecularPower = 0.5f;

		inputs.push_back(std::make_shared<Input>(this, "Diffuse", FlowType::FLOAT3));
		inputs.push_back(std::make_shared<Input>(this, "Specular", FlowType::FLOAT));
		inputs.push_back(std::make_shared<Input>(this, "Emissive", FlowType::FLOAT3));
		inputs.push_back(std::make_shared<Input>(this, "Normals", FlowType::FLOAT3));
		inputs.push_back(std::make_shared<Input>(this, "SpecularPower", FlowType::FLOAT));
	}

	void compile(CompilationErrorCheck& errorCheck)
	{
		std::stringstream nodeCompileResult;

		std::vector<std::string> usedParameterNames;
		defineParameter(nodeCompileResult, usedParameterNames, errorCheck);

		std::stringstream paramDiffuseStream;
		if (inputs[0]->link != nullptr)
			inputs[0]->compile(paramDiffuseStream, errorCheck);
		else
			paramDiffuseStream << "vec3(" << defaultDiffuse.x << ", " << defaultDiffuse.y << ", " << defaultDiffuse.z << ")";

		std::stringstream paramSpecularStream;
		if (inputs[1]->link != nullptr)
			inputs[1]->compile(paramSpecularStream, errorCheck);
		else
			paramSpecularStream << defaultSpecular;

		std::stringstream paramEmissiveStream;
		if (inputs[2]->link != nullptr)
			inputs[2]->compile(paramEmissiveStream, errorCheck);
		else
			paramEmissiveStream << "vec3(" << defaultEmissive.x << ", " << defaultEmissive.y << ", " << defaultEmissive.z << ")";

		std::stringstream paramNormalsStream;
		if (inputs[3]->link != nullptr)
			inputs[3]->compile(paramNormalsStream, errorCheck);
		else
			paramNormalsStream << "vec3(" << defaultNormals.x << ", " << defaultNormals.y << ", " << defaultNormals.z << ")";

		std::stringstream paramSpecularPowerStream;
		if (inputs[4]->link != nullptr)
			inputs[4]->compile(paramSpecularPowerStream, errorCheck);
		else
			paramSpecularPowerStream << defaultSpecularPower;

		nodeCompileResult << "void computeShaderParameters(inout vec3 paramDiffuse, inout vec3 paramNormals, inout float paramSpecular, inout float paramSpecularPower, inout vec3 paramEmissive, in vec2 projectedCoord)\n";
		nodeCompileResult << "{\n";
		nodeCompileResult << "paramDiffuse = " << paramDiffuseStream.str() << ";\n";
		nodeCompileResult << "paramSpecular = " << paramSpecularStream.str() << ";\n";
		nodeCompileResult << "paramEmissive = " << paramEmissiveStream.str() << ";\n";
		nodeCompileResult << "paramNormals = " << paramNormalsStream.str() << ";\n";
		nodeCompileResult << "paramSpecularPower = " << paramSpecularPowerStream.str() << ";\n";
		nodeCompileResult << "}\n";

		finalValueStr = nodeCompileResult.str();
	}

	virtual void drawUIParameters() override
	{
		ImGui::InputFloat3("defaultDiffuse", &defaultDiffuse[0]);
		ImGui::InputFloat("defaultSpecular", &defaultSpecular);
		ImGui::InputFloat3("defaultEmissive", &defaultEmissive[0]);
		ImGui::InputFloat3("defaultNormals", &defaultNormals[0]);
		ImGui::InputFloat("defaultSpecularPower", &defaultSpecularPower);
	}
};

struct FunctionNode : public Node
{
	INHERIT_FROM_NODE(FunctionNode)

	std::string functionStr;

	FunctionNode(const std::string& _nodeName, const std::string& _operatorStr)
		: Node(NodeType::OPERATOR, _nodeName)
		, functionStr(_operatorStr)
	{

	}

	virtual void compile(CompilationErrorCheck& errorCheck) override
	{
		// Print : functionStr(input02, input02, ...)
		std::stringstream nodeCompileResult;
		nodeCompileResult << functionStr;
		nodeCompileResult << "(";
		int idx = 0;
		for (auto input : inputs)
		{
			if (input->link != nullptr)
			{
				input->compile(nodeCompileResult, errorCheck);
			}
			else
			{
				printDefaultValue(nodeCompileResult, input->desiredType);
			}
			if (idx < inputs.size() - 1)
				nodeCompileResult << ',';
			idx++;
		}
		nodeCompileResult << ")";

		outputs[0]->valueStr = nodeCompileResult.str();
	}


};

struct CustomNode : public Node
{
		CustomNode(const std::string& _nodeName)
	: Node(NodeType::CUSTOM, _nodeName)
	{

	}
};

struct OperatorNode : public Node
{
	INHERIT_FROM_NODE(OperatorNode)

	std::string operatorStr;

	OperatorNode(const std::string& _nodeName, const std::string& _operatorStr)
		: Node(NodeType::OPERATOR, _nodeName)
		, operatorStr(_operatorStr)
	{

	}

	virtual void compile(CompilationErrorCheck& errorCheck) override
	{
		// Print : ([...] operator [...])
		std::stringstream nodeCompileResult;
		nodeCompileResult << "(";
		int idx = 0;
		for (auto input : inputs)
		{
			if (input->link != nullptr)
			{
				input->compile(nodeCompileResult, errorCheck);
			}
			else
			{
				printDefaultValue(nodeCompileResult, input->desiredType);
			}
			if (idx < inputs.size() - 1)
				nodeCompileResult << operatorStr;
			idx++;
		}
		nodeCompileResult << ")";

		outputs[0]->valueStr = nodeCompileResult.str();
	}
};

struct BaseParameterNode : public Node
{
	char parameterNameForUI[100];
	std::string parameterName;
	std::string formatedParameterName;
	bool isUniform;
	bool nameDefined;

	BaseParameterNode(const std::string& _nodeName)
		: Node(NodeType::PARAMETER, _nodeName)
		, parameterName("")
		, formatedParameterName("")
		, nameDefined(false)
	{
		parameterNameForUI[0] = '\0';
	}

	virtual const InternalShaderParameterBase* getInternalParameter() const = 0;
};

template <typename T>
struct ParameterNode : public BaseParameterNode
{
	INHERIT_FROM_NODE(ParameterNode<T>)

	InternalShaderParameter<T, ShaderParameter::IsNotArray> shaderParameter;

	ParameterNode();
	virtual void onBeforeCompile(CompilationErrorCheck& errorCheck) override
	{
		nameDefined = false;
	}
	virtual void compile(CompilationErrorCheck& errorCheck) override;
	virtual void defineParameter(std::stringstream& stream, std::vector<std::string>& usedParameterNames, CompilationErrorCheck& errorCheck) override
	{
		if (nameDefined)
			return;

		// Print [uniform] type name;
		if (isUniform)
			stream << "uniform ";
		else
			stream << "const ";

		formatedParameterName = (parameterName == "" || parameterName == "ReflectionTexture") ? "a" : parameterName;
		if (std::find(usedParameterNames.begin(), usedParameterNames.end(), formatedParameterName) != usedParameterNames.end())
		{
			formatedParameterName += "_";
			formatedParameterName += std::to_string(usedParameterNames.size());
		}
		usedParameterNames.push_back(formatedParameterName);

		stream << Utils::typeAsString<T>() << " " << formatedParameterName << " = " << shaderParameter.valueAsString() << ";\n";

		nameDefined = true;
	}

	virtual const InternalShaderParameterBase* getInternalParameter() const override
	{
		return &shaderParameter;
	}

	virtual void drawUIParameters() override
	{
		if (ImGui::InputText("ParameterName", parameterNameForUI, 100))
		{
			parameterName = parameterNameForUI;
			if(parameterName.find_first_of('\0') != std::string::npos)
				parameterName = parameterName.substr();
		}
		shaderParameter.drawUI();
	}
};

template<>
inline void ParameterNode<Texture>::defineParameter(std::stringstream& stream, std::vector<std::string>& usedParameterNames, CompilationErrorCheck& errorCheck)
{
	// Texture type is always uniform
	stream << "uniform ";

	formatedParameterName = (parameterName == "" || parameterName == "ReflectionTexture") ? "a" : parameterName;
	if (std::find(usedParameterNames.begin(), usedParameterNames.end(), formatedParameterName) != usedParameterNames.end())
	{
		formatedParameterName += "_";
		formatedParameterName += std::to_string(usedParameterNames.size());
	}
	usedParameterNames.push_back(formatedParameterName);

	stream << "sampler2D " << formatedParameterName << ";\n";
}

//////////////////////////////////////////////////////////////
//// BEGIN : Parameters

template<>
inline ParameterNode<float>::ParameterNode()
	: BaseParameterNode("scalar")
	, shaderParameter(name, true, 0.f, EditorGUI::FieldDisplayType::DEFAULT)
{
	outputs.push_back(std::make_shared<Output>(this, "x", FlowType::FLOAT));
}

template<>
inline void ParameterNode<float>::compile(CompilationErrorCheck& errorCheck)
{
	outputs[0]->valueStr = formatedParameterName;
}

REGISTER_TEMPLATED_NODE("scalar", ParameterNode, float, ParameterFloat)

/////////////

template<>
inline ParameterNode<glm::vec2>::ParameterNode()
	: BaseParameterNode("vector2")
	, shaderParameter(name, true, glm::vec2(0, 0), EditorGUI::FieldDisplayType::DEFAULT)
{
	outputs.push_back(std::make_shared<Output>(this, "rg", FlowType::FLOAT2));
	outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
}

template<>
inline void ParameterNode<glm::vec2>::compile(CompilationErrorCheck& errorCheck)
{
	outputs[0]->valueStr = formatedParameterName;
	outputs[1]->valueStr = formatedParameterName + ".r";
	outputs[2]->valueStr = formatedParameterName + ".g";
}

REGISTER_TEMPLATED_NODE("vector2", ParameterNode, glm::vec2, ParameterVec2)

/////////////

template<>
inline ParameterNode<glm::vec3>::ParameterNode()
	: BaseParameterNode("vector3")
	, shaderParameter(name, true, glm::vec3(0,0,0), EditorGUI::FieldDisplayType::DEFAULT)
{
	outputs.push_back(std::make_shared<Output>(this, "rgb", FlowType::FLOAT3));
	outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "b", FlowType::FLOAT));
}

template<>
inline void ParameterNode<glm::vec3>::compile(CompilationErrorCheck& errorCheck)
{
	outputs[0]->valueStr = formatedParameterName;
	outputs[1]->valueStr = formatedParameterName + ".r";
	outputs[2]->valueStr = formatedParameterName + ".g";
	outputs[3]->valueStr = formatedParameterName + ".b";
}

REGISTER_TEMPLATED_NODE("vector3", ParameterNode, glm::vec3, ParameterVec3)

/////////////

template<>
inline ParameterNode<glm::vec4>::ParameterNode()
	: BaseParameterNode("vector4")
	, shaderParameter(name, true, glm::vec4(0, 0, 0, 0), EditorGUI::FieldDisplayType::DEFAULT)
{
	outputs.push_back(std::make_shared<Output>(this, "rgba", FlowType::FLOAT4));
	outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "b", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "a", FlowType::FLOAT));
}

template<>
inline void ParameterNode<glm::vec4>::compile(CompilationErrorCheck& errorCheck)
{
	outputs[0]->valueStr = formatedParameterName;
	outputs[1]->valueStr = formatedParameterName + ".r";
	outputs[2]->valueStr = formatedParameterName + ".g";
	outputs[3]->valueStr = formatedParameterName + ".b";
	outputs[4]->valueStr = formatedParameterName + ".a";
}

REGISTER_TEMPLATED_NODE("vector4", ParameterNode, glm::vec4, ParameterVec4)

/////////////

template<>
inline ParameterNode<Texture>::ParameterNode()
	: BaseParameterNode("texture")
	, shaderParameter(name, true, ResourcePtr<Texture>(), EditorGUI::FieldDisplayType::DEFAULT)
{
	inputs.push_back(std::make_shared<Input>(this, "texCoords", FlowType::FLOAT2));

	outputs.push_back(std::make_shared<Output>(this, "rgba", FlowType::FLOAT4));
	outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "b", FlowType::FLOAT));
	outputs.push_back(std::make_shared<Output>(this, "a", FlowType::FLOAT));
}

template<>
inline void ParameterNode<Texture>::compile(CompilationErrorCheck& errorCheck)
{
	std::stringstream textureCoords;
	if (inputs[0]->link != nullptr)
		inputs[0]->compile(textureCoords, errorCheck);
	else
		textureCoords << "In.TexCoord";

	std::stringstream core;
	core << "texture(" << formatedParameterName << "," << textureCoords.str() << ')';

	outputs[0]->valueStr = core.str() + ".rgba"; //texture(parameterName, texCoords).rgba
	outputs[1]->valueStr = core.str() + ".r";    //texture(parameterName, texCoords).r
	outputs[2]->valueStr = core.str() + ".g";    //texture(parameterName, texCoords).g
	outputs[3]->valueStr = core.str() + ".b";    //texture(parameterName, texCoords).b
	outputs[4]->valueStr = core.str() + ".a";    //texture(parameterName, texCoords).a
}

REGISTER_TEMPLATED_NODE("texture", ParameterNode, Texture, ParameterTexture)

//// END : Parameters
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//// BEGIN : Customs

struct TexCoordsNode final : public CustomNode
{
	INHERIT_FROM_NODE(TexCoordsNode)

	TexCoordsNode()
		: CustomNode("TexCoords")
	{
		outputs.push_back(std::make_shared<Output>(this, "rg", FlowType::FLOAT2));
		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
	}
	void compile(CompilationErrorCheck& errorCheck) override
	{
		outputs[0]->valueStr = "In.TexCoord";
		outputs[1]->valueStr = "In.TexCoord.r";
		outputs[2]->valueStr = "In.TexCoord.g";
	}
};

REGISTER_NODE("TexCoords", TexCoordsNode)

struct FragCoordsNode final : public CustomNode
{
	INHERIT_FROM_NODE(FragCoordsNode)

		FragCoordsNode()
		: CustomNode("FragCoords")
	{
		outputs.push_back(std::make_shared<Output>(this, "rg", FlowType::FLOAT2));
		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
	}
	void compile(CompilationErrorCheck& errorCheck) override
	{
		outputs[0]->valueStr = "gl_FragCoord.rg";
		outputs[1]->valueStr = "gl_FragCoord.r";
		outputs[2]->valueStr = "gl_FragCoord.g";
	}
};

REGISTER_NODE("FragCoords", FragCoordsNode)

struct ProjectedCoordsNode final : public CustomNode
{
	INHERIT_FROM_NODE(ProjectedCoordsNode)

		ProjectedCoordsNode()
		: CustomNode("ProjectedCoords")
	{
		outputs.push_back(std::make_shared<Output>(this, "rg", FlowType::FLOAT2));
		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
	}
	void compile(CompilationErrorCheck& errorCheck) override
	{
		outputs[0]->valueStr = "projectedCoord.rg";
		outputs[1]->valueStr = "projectedCoord.r";
		outputs[2]->valueStr = "projectedCoord.g";
	}
};

REGISTER_NODE("ProjectedCoords", ProjectedCoordsNode)

struct ReflectionTextureNode final : public CustomNode
{
	INHERIT_FROM_NODE(ReflectionTextureNode)

		ReflectionTextureNode()
		: CustomNode("ReflectionTexture")
	{
		inputs.push_back(std::make_shared<Input>(this, "texCoords", FlowType::FLOAT2));

		outputs.push_back(std::make_shared<Output>(this, "rgb", FlowType::FLOAT3));
		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "b", FlowType::FLOAT));
	}

	virtual void defineParameter(std::stringstream& stream, std::vector<std::string>& usedParameterNames, CompilationErrorCheck& errorCheck) override
	{
		stream << "uniform sampler2D ReflectionTexture;";
	}

	void compile(CompilationErrorCheck& errorCheck) override
	{
		std::stringstream textureCoords;
		if (inputs[0]->link != nullptr)
			inputs[0]->compile(textureCoords, errorCheck);
		else
			textureCoords << "projectedCoord";

		std::stringstream core;
		core << "texture(ReflectionTexture, " << textureCoords.str() << ')';

		outputs[0]->valueStr = core.str() + ".rgb";
		outputs[1]->valueStr = core.str() + ".r";
		outputs[2]->valueStr = core.str() + ".g";
		outputs[3]->valueStr = core.str() + ".b";
	}
};

REGISTER_NODE("ReflectionTexture", ReflectionTextureNode)

struct BreakVector final : public CustomNode
{
	INHERIT_FROM_NODE(BreakVector)

		BreakVector()
		: CustomNode("BreakVector")
	{
		inputs.push_back(std::make_shared<Input>(this, "vec", FlowType::CUSTOM));

		outputs.push_back(std::make_shared<Output>(this, "rgba", FlowType::FLOAT4));
		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "b", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "a", FlowType::FLOAT));
	}
	void compile(CompilationErrorCheck& errorCheck) override
	{
		// link != null and we have float or vector type
		if (inputs[0]->link != nullptr && ((int)inputs[0]->link->input->outputType > 0 && (int)inputs[0]->link->input->outputType < 5))
		{
			std::stringstream nodeCompileResult;
			nodeCompileResult << "vec4(";
			inputs[0]->compile(nodeCompileResult, errorCheck);

			Output* entry = inputs[0]->link->input;
			assert(entry != nullptr);

			// float : [float], 0, 0, 0
			// float2 : [float2], 0, 0
			// ...
			for (int i = 0; i < 4 - (int)(entry->outputType); i++)
			{
				nodeCompileResult << ", 0";
			}

			nodeCompileResult << ")";

			outputs[0]->valueStr = nodeCompileResult.str();
			outputs[1]->valueStr = nodeCompileResult.str() + ".r";
			outputs[2]->valueStr = nodeCompileResult.str() + ".g";
			outputs[3]->valueStr = nodeCompileResult.str() + ".b";
			outputs[4]->valueStr = nodeCompileResult.str() + ".a";
		}
		else
		{
			outputs[0]->valueStr = "vec4(0,0,0,0)";
			outputs[1]->valueStr = "0.f";
			outputs[2]->valueStr = "0.f";
			outputs[3]->valueStr = "0.f";
			outputs[4]->valueStr = "0.f";
		}
	}
};

REGISTER_NODE("BreakVector", BreakVector)

struct MakeVec3Node final : public CustomNode
{
	INHERIT_FROM_NODE(MakeVec3Node)

		MakeVec3Node()
		: CustomNode("MakeVec3")
	{
		inputs.push_back(std::make_shared<Input>(this, "r", FlowType::FLOAT));
		inputs.push_back(std::make_shared<Input>(this, "g", FlowType::FLOAT));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::FLOAT));

		outputs.push_back(std::make_shared<Output>(this, "rgb", FlowType::FLOAT3));
		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "g", FlowType::FLOAT));
		outputs.push_back(std::make_shared<Output>(this, "b", FlowType::FLOAT));
	}
	void compile(CompilationErrorCheck& errorCheck) override
	{
		std::stringstream nodeCompileResult;
		nodeCompileResult << "vec3(";
		for (auto input : inputs)
		{
			if (input->link != nullptr)
			{
				input->compile(nodeCompileResult, errorCheck);
			}
			else
			{
				printDefaultValue(nodeCompileResult, input->desiredType);
			}
			nodeCompileResult << ", ";
		}
		nodeCompileResult << ")";

		outputs[0]->valueStr = nodeCompileResult.str();
		outputs[1]->valueStr = nodeCompileResult.str() + ".r";
		outputs[2]->valueStr = nodeCompileResult.str() + ".g";
		outputs[3]->valueStr = nodeCompileResult.str() + ".b";
	}
};

REGISTER_NODE("MakeVec3", MakeVec3Node)

//// BEGIN : Customs
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//// BEGIN : Operators

struct OperatorNodeAdd : public OperatorNode
{
	INHERIT_FROM_NODE(OperatorNodeAdd)

	OperatorNodeAdd()
		: OperatorNode("add", "+")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("add", OperatorNodeAdd)

struct OperatorNodeMinus : public OperatorNode
{
	INHERIT_FROM_NODE(OperatorNodeMinus)

	OperatorNodeMinus()
		: OperatorNode("minus", "-")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("minus", OperatorNodeMinus)

struct OperatorNodeMultiply : public OperatorNode
{
	INHERIT_FROM_NODE(OperatorNodeMultiply)

	OperatorNodeMultiply()
		: OperatorNode("multiply", "*")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("multiply", OperatorNodeMultiply)

struct OperatorNodeDivide : public OperatorNode
{
	INHERIT_FROM_NODE(OperatorNodeDivide)

	OperatorNodeDivide()
		: OperatorNode("divide", "/")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("divide", OperatorNodeDivide)

//// END : Operators
//////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////
//// BEGIN : Functions

struct FunctionNodeDot : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeDot)

	FunctionNodeDot()
		: FunctionNode("dot", "dot")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("dot", FunctionNodeDot)

struct FunctionNodeCross : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeCross)

	FunctionNodeCross()
		: FunctionNode("cross", "cross")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::FLOAT3));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::FLOAT3));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::FLOAT3));
	}
};

REGISTER_NODE("cross", FunctionNodeCross)

struct FunctionNodeNormalize : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeNormalize)

	FunctionNodeNormalize()
		: FunctionNode("normalize", "normalize")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("normalize", FunctionNodeNormalize)

struct FunctionNodeDistance : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeDistance)

	FunctionNodeDistance()
		: FunctionNode("distance", "distance")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("distance", FunctionNodeDistance)

struct FunctionNodeLength : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeLength)

	FunctionNodeLength()
		: FunctionNode("length", "length")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("length", FunctionNodeLength)

struct FunctionNodeMin : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeMin)

	FunctionNodeMin()
		: FunctionNode("minimum", "min")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("minimum", FunctionNodeMin)

struct FunctionNodeMax : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeMax)

	FunctionNodeMax()
		: FunctionNode("maximum", "max")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("maximum", FunctionNodeMax)

struct FunctionNodeMod : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeMod)

	FunctionNodeMod()
		: FunctionNode("modulo", "mod")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));
		inputs.push_back(std::make_shared<Input>(this, "b", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("modulo", FunctionNodeMod)

struct FunctionNodeCos : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeCos)

	FunctionNodeCos()
		: FunctionNode("cosine", "cos")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("cosine", FunctionNodeCos)

struct FunctionNodeSin : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeSin)

	FunctionNodeSin()
		: FunctionNode("sine", "sin")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("sine", FunctionNodeSin)

struct FunctionNodeTan : public FunctionNode
{
	INHERIT_FROM_NODE(FunctionNodeTan)

	FunctionNodeTan()
		: FunctionNode("tangent", "tan")
	{
		inputs.push_back(std::make_shared<Input>(this, "a", FlowType::UNDEFINED));

		outputs.push_back(std::make_shared<Output>(this, "r", FlowType::UNDEFINED));
	}
};

REGISTER_NODE("tangent", FunctionNodeTan)

//// END : Functions
//////////////////////////////////////////////////////////////

class NodeManager
{
private:
	FinalNode* m_finalOutput;
	std::vector < std::shared_ptr<Node> > m_allNodes;
	std::vector< std::shared_ptr<Link> > m_allLinks;
	std::vector<BaseParameterNode*> m_parameterNodes;
	//std::stringstream m_compileStream;
	std::string m_compileResult;
	bool m_lastCompilationSucceeded;

	Material* m_programPtr;

	glm::vec2 m_dragAnchorPos;
	bool m_isDraggingNode;
	Node* m_draggedNode;
	bool m_isDraggingLink;
	Link* m_draggedLink;
	bool m_canResetDragLink;
	std::vector<Node*> m_selectedNodes;
	bool m_isSelectingNodes;
	bool m_isHoverridingANode;
	glm::vec2 m_backgroundDecal;
	ImVec2 m_newNodePos;

	MeshVisualizer m_meshVisualizer;

public:
	NodeManager(Material* programPtr);
	~NodeManager();

	void compile();

	Node* getNode(const ID& id) const;
	void save(Json::Value& root) const;
	void load(const Json::Value& root);
	void drawUI(Renderer& renderer);

	bool isDraggingNode() const;
	void setIsDraggingNode(bool state);
	void setDragAnchorPos(const glm::vec2& position);
	const glm::vec2& getDragAnchorPos() const;
	Node* getDraggedNode() const;
	void setDraggedNode(Node* node);
	void setIsDraggingLink(bool state);
	bool getIsDraggingLink() const;
	void setDraggedLink(Link* link);
	Link* getDraggedLink() const;
	void setCanResetDragLink(bool state);
	void setSelectedNode(Node* node);
	bool isSelectingNode(Node* node) const;
	void setIsHoverridingANode(bool state);

	void addLink(std::shared_ptr<Link> link);
	void removeLink(Link* link);
	std::vector<std::shared_ptr<Link>>::iterator NodeManager::removeLink(std::vector<std::shared_ptr<Link>>::iterator linkIt);
	void removeNode(Node* node);
	std::vector<std::shared_ptr<Node>>::iterator NodeManager::removeNode(std::vector<std::shared_ptr<Node>>::iterator nodeIt);

private:
	void internalOnBeforeCompilation(CompilationErrorCheck& errorCheck);
	void internalResolveUndeterminedTypes();
	//void internalDefineParameters(std::stringstream& compileStream, CompilationErrorCheck& errorCheck);
	void internalCompile(CompilationErrorCheck& errorCheck);
};

}