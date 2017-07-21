#include "MaterialVisualScripting.h"
#include "Material.h"
#include "Factories.h"

namespace MVS {


void formatAndOutputResult(std::stringstream& stream, const Output& output, FlowType desiredType, CompilationErrorCheck& errorCheck)
{
	if (desiredType == output.outputType)
	{
		stream << output.valueStr;
	}
	else if (desiredType == FlowType::FLOAT)
	{
		if (output.outputType == FlowType::FLOAT2)
		{
			stream << ".rr";
		}
		else if (output.outputType == FlowType::FLOAT3)
		{
			stream << ".rrr";
		}
		else if (output.outputType == FlowType::FLOAT4)
		{
			stream << ".rrrr";
		}
	}
	else if (desiredType == FlowType::FLOAT2)
	{
		if (output.outputType == FlowType::FLOAT)
		{
			stream << "vec2(" << output.valueStr << ',' << output.valueStr << ')';
		}
		else if (output.outputType == FlowType::FLOAT3 || output.outputType == FlowType::FLOAT4)
		{
			stream << output.valueStr << ".rg";
		}
		else
		{
			errorCheck.errorMsg = "incompatible types.";
			errorCheck.compilationFailed = true;
			assert(false && "incompatible types.");
		}
	}
	else if (desiredType == FlowType::FLOAT3)
	{
		if (output.outputType == FlowType::FLOAT)
		{
			stream << "vec3(" << output.valueStr << ',' << output.valueStr << ',' << output.valueStr << ')';
		}
		else if (output.outputType == FlowType::FLOAT4)
		{
			stream << output.valueStr << ".rgb";
		}
		else
		{
			errorCheck.errorMsg = "incompatible types.";
			errorCheck.compilationFailed = true;
			assert(false && "incompatible types.");
		}
	}
	else if (desiredType == FlowType::FLOAT4)
	{
		if (output.outputType == FlowType::FLOAT)
		{
			stream << "vec3(" << output.valueStr << ',' << output.valueStr << ',' << output.valueStr << ',' << output.valueStr << ')';
		}
		else
		{
			errorCheck.errorMsg = "incompatible types.";
			errorCheck.compilationFailed = true;
			assert(false && "incompatible types.");
		}
	}
	// No formating for cusrom desired type
	else if (desiredType == FlowType::CUSTOM)
	{
		stream << output.valueStr;
	}
}

void printDefaultValue(std::stringstream & nodeCompileResult, FlowType desiredType)
{
	switch (desiredType)
	{
	case MVS::FlowType::UNDEFINED:
		break;
	case MVS::FlowType::FLOAT:
		nodeCompileResult << "0.f";
		break;
	case MVS::FlowType::FLOAT2:
		nodeCompileResult << "vec2(0.f, 0.f)";
		break;
	case MVS::FlowType::FLOAT3:
		nodeCompileResult << "vec3(0.f, 0.f, 0.f)";
		break;
	case MVS::FlowType::FLOAT4:
		nodeCompileResult << "vec4(0.f, 0.f, 0.f, 0.f)";
		break;
	default:
		break;
	}
}


///////////////////////////////////////////
//// BEGIN : Input

Input::Input(Node* _parentNode, const std::string& _name, FlowType _desiredType)
	: parentNode(_parentNode)
	, name(_name)
	, desiredType(_desiredType)
	, link(nullptr)
{}

void Input::resolveUndeterminedTypes(FlowType _desiredType)
{
	assert(_desiredType != FlowType::UNDEFINED);
	if (desiredType == FlowType::UNDEFINED)
		desiredType = _desiredType;

	if (link != nullptr)
	{
		Output* linkedOutput = link->input;
		assert(linkedOutput != nullptr);
		if (linkedOutput->outputType == FlowType::UNDEFINED)
			linkedOutput->outputType = _desiredType;

		linkedOutput->parentNode->resolveUndeterminedTypes();
	}
}

void Input::compile(std::stringstream& stream, CompilationErrorCheck& errorCheck)
{
	if (link != nullptr)
	{
		Output* linkedOutput = link->input;
		assert(linkedOutput != nullptr);
		linkedOutput->parentNode->compile(errorCheck);
		formatAndOutputResult(stream, *linkedOutput, desiredType, errorCheck);
	}
}

//// END : Input
///////////////////////////////////////////

///////////////////////////////////////////
//// BEGIN : Output

Output::Output(Node* _parentNode, const std::string& _name, FlowType _outputType)
	: parentNode(_parentNode)
	, name(_name)
	, outputType(_outputType)
{}

//// END : Output
///////////////////////////////////////////

///////////////////////////////////////////
//// BEGIN : Link

Link::Link(Output* _input, Input* _output)
	: input(_input)
	, output(_output)
{
}

void Link::save(Json::Value& root) const
{
	inputNodeID.save(root["inputNodeID"]);
	root["inputIdx"] = inputIdx;
	outputNodeID.save(root["outputNodeID"]);
	root["outputIdx"] = outputIdx;
}
void Link::load(const Json::Value& root)
{
	inputNodeID.load(root["inputNodeID"]);
	inputIdx = root["inputIdx"].asInt();
	outputNodeID.load(root["outputNodeID"]);
	outputIdx = root["outputIdx"].asInt();
}

void Link::drawUI(NodeManager& manager)
{
	if (input == nullptr || output == nullptr)
		return;

	ImVec2 A(input->position.x, input->position.y);
	ImVec2 A_(A.x + 100.f, A.y);
	ImVec2 B(output->position.x, output->position.y);
	ImVec2 B_(B.x - 100.f, B.y);

	ImGui::GetWindowDrawList()->AddBezierCurve(A, A_, B_, B, ImColor(0.5f, 0.5f, 0.5f, 1.0f), 2, 0);
}

//// END : Link
///////////////////////////////////////////

///////////////////////////////////////////
//// BEGIN : Node

void Node::defineParameter(std::stringstream & stream, std::vector<std::string>& usedParameterNames, CompilationErrorCheck & errorCheck)
{
	for (auto input : inputs)
	{
		Link* link = input->link;
		if (link != nullptr)
		{
			Output* output = link->input;
			assert(output != nullptr);
			output->parentNode->defineParameter(stream, usedParameterNames, errorCheck);
		}
	}
}

Node::Node(NodeType _type, const std::string& _name)
	: type(_type)
	, name(_name)
	, size(200, 120)
{
	nodeID = IDGenerator<MVS::Node>::instance().lockID();
}

Node::Node()
	: Node(NodeType::FUNCTION, "default")
{
}

Node::Node(const Node & other)
	: type(other.type)
	, name(other.name)
	, nodeID(other.nodeID)
	, position(other.position)
	, size(other.size)
{
	//Deep copy
	for (auto otherInput : other.inputs)
	{
		inputs.push_back(std::make_shared<Input>(*otherInput));
		inputs.back()->parentNode = this; // /!\ Don't norget to change the parent node !
	}

	for (auto otherOutput : other.outputs)
	{
		outputs.push_back(std::make_shared<Output>(*otherOutput));
		outputs.back()->parentNode = this; // /!\ Don't norget to change the parent node !
	}
}

Node & Node::operator=(const Node & other)
{
	type = other.type;
	name = other.name;
	nodeID = other.nodeID;
	position = other.position;
	size = other.size;

	//Deep copy
	for (auto otherInput : other.inputs)
	{
		inputs.push_back(std::make_shared<Input>(*otherInput));
	}

	for (auto otherOutput : other.outputs)
	{
		outputs.push_back(std::make_shared<Output>(*otherOutput));
	}

	return *this;
}

void Node::resolveUndeterminedTypes()
{
	for (auto input : inputs)
	{
		if (input->desiredType == FlowType::UNDEFINED && outputs.size() > 0)
			input->resolveUndeterminedTypes(outputs[0]->outputType);
		else if (input->desiredType != FlowType::UNDEFINED)
			input->resolveUndeterminedTypes(input->desiredType);
	}
}

void Node::save(Json::Value& root) const
{
	root["name"] = name;
	root["type"] = (int)type;
	nodeID.save(root["id"]);
	root["position"][0] = position.x;
	root["position"][1] = position.y;
}
void Node::load(const Json::Value& root)
{
	//name = root["name"].asString();
	nodeID.load(root["id"]);
	position = glm::vec2(root["position"][0].asFloat(), root["position"][1].asFloat());
}

void Node::drawUI(NodeManager& manager)
{
	ImGui::PushID(this);
	//ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(20, 5));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0.4, 0.4, 0.4, 1));

	ImVec2 cursorPos = ImGui::GetCursorScreenPos();
	ImVec2 recMin(cursorPos.x + position.x, cursorPos.y + position.y);
	ImVec2 recMax(cursorPos.x + position.x + size.x, cursorPos.y + position.y + size.y);

	ImGui::SetCursorScreenPos(recMin);
	const bool showBorders = (manager.isSelectingNode(this));

	ImGui::BeginChild(name.c_str(), ImVec2(size.x, size.y), showBorders);

	bool needToCheckIsDraginfNode = false;
	//ImGui::GetWindowDrawList()->AddRectFilled(recMin, recMax, ImColor(0.4f, 0.4f, 0.4f, 1.f), 0.4f);
	if (ImGui::IsMouseHoveringWindow() && ImGui::IsMouseHoveringRect(recMin, recMax) && ImGui::IsMouseClicked(0) && !DragAndDropManager::isDragAndDropping())
	{
		needToCheckIsDraginfNode = true;
	}

	ImVec2 inputsPosition(recMin.x + 12.f, recMin.y + 20.f);
	for (auto input : inputs)
	{
		ImGui::PushID(input->name.c_str());

		ImVec2 textSize = ImGui::CalcTextSize(input->name.c_str());
		ImGui::SetCursorScreenPos(ImVec2(inputsPosition.x, inputsPosition.y));
		if (ImGui::RadioButton("##input", true))
		{
			if (!manager.isDraggingNode())
			{
				if (!manager.getIsDraggingLink())
				{
					manager.setIsDraggingLink(true);

					Link* link = input->link;
					if (link != nullptr)
					{
						//manager.removeLink(link);
						//auto newLink = std::make_shared<Link>(nullptr, input.get());
						link->output->link = nullptr;
						link->output = nullptr;
						manager.setDraggedLink(link);
					}
					else
					{
						auto newLink = std::make_shared<Link>(nullptr, input.get());
						input->link = newLink.get();
						manager.addLink(newLink);
						manager.setDraggedLink(newLink.get());
					}
				}
				else
				{
					if (input->link != nullptr)
						manager.removeLink(input->link);

					Link* link = manager.getDraggedLink();
					link->output = input.get();
					input->link = link;
					manager.setIsDraggingLink(false);
					manager.setDraggedLink(nullptr);
				}
			}
		}
		if (ImGui::IsItemHovered())
		{
			needToCheckIsDraginfNode = false;
			manager.setCanResetDragLink(false);
		}
		ImGui::SameLine();
		ImGui::Text(input->name.c_str());

		//ImGui::GetWindowDrawList()->AddCircleFilled(outputsPosition, 10.f, ImColor(0.3f, 0.3f, 0.3f, 1.0f), 8);
		//ImGui::GetWindowDrawList()->AddText(ImVec2(outputsPosition.x - 12.f, outputsPosition.y), ImColor(0.8f, 0.8f, 0.8f, 1.0f), output->name.c_str());
		input->position.x = inputsPosition.x + 5; // ImGui::GetCursorScreenPos().x; // outputsPosition.x + 10;
		input->position.y = inputsPosition.y + 5; // ImGui::GetCursorScreenPos().y; // outputsPosition.y + 10;

		inputsPosition.y += 20.f;

		ImGui::PopID();
	}

	ImVec2 outputsPosition(recMax.x - 30.f, recMin.y + 20.f);
	for (auto output : outputs)
	{
		ImGui::PushID(output->name.c_str());

		ImVec2 textSize = ImGui::CalcTextSize(output->name.c_str());
		outputsPosition.x = recMax.x - textSize.x - 30.f;
		ImGui::SetCursorScreenPos(ImVec2(outputsPosition.x, outputsPosition.y));
		ImGui::Text(output->name.c_str());
		ImGui::SameLine();
		if (ImGui::RadioButton("##output", true))
		{
			if (!manager.isDraggingNode())
			{
				if (!manager.getIsDraggingLink())
				{
					manager.setIsDraggingLink(true);

					auto newLink = std::make_shared<Link>(output.get(), nullptr);
					output->links.push_back(newLink.get());
					manager.addLink(newLink);
					manager.setDraggedLink(newLink.get());
				}
				else
				{
					Link* link = manager.getDraggedLink();
					link->input = output.get();
					output->links.push_back(link);
					manager.setIsDraggingLink(false);
					manager.setDraggedLink(nullptr);
				}
			}
		}
		if (ImGui::IsItemHovered())
		{
			needToCheckIsDraginfNode = false;
			manager.setCanResetDragLink(false);
		}
		//ImGui::GetWindowDrawList()->AddCircleFilled(outputsPosition, 10.f, ImColor(0.3f, 0.3f, 0.3f, 1.0f), 8);
		//ImGui::GetWindowDrawList()->AddText(ImVec2(outputsPosition.x - 12.f, outputsPosition.y), ImColor(0.8f, 0.8f, 0.8f, 1.0f), output->name.c_str());
		output->position.x = outputsPosition.x + 5; //ImGui::GetCursorScreenPos().x; // outputsPosition.x + 10;
		output->position.y = outputsPosition.y + 5; //ImGui::GetCursorScreenPos().y; // outputsPosition.y + 10;

		outputsPosition.y += 20.f;

		ImGui::PopID();
	}

	if (needToCheckIsDraginfNode)
	{
		if (!manager.isDraggingNode() && !manager.getIsDraggingLink())
		{
			manager.setIsDraggingNode(true);
			manager.setDragAnchorPos(glm::vec2(ImGui::GetMousePos().x - position.x, ImGui::GetMousePos().y - position.y));
			manager.setDraggedNode(this);
		}
	}

	ImGui::SetCursorScreenPos(recMin);
	ImGui::Text(name.c_str());
	ImGui::Separator();

	if (ImGui::IsMouseHoveringWindow())
	{
		manager.setIsHoverridingANode(true);

		if(ImGui::IsMouseReleased(0) && !manager.getIsDraggingLink())
			manager.setSelectedNode(this);
	}

	ImGui::EndChild();
	ImGui::SetCursorScreenPos(cursorPos);

	ImGui::PopStyleColor();
	ImGui::PopID();
}

void Node::setPosition(const glm::vec2& pos)
{
	position = pos;
}

const glm::vec2& Node::getPosition() const
{
	return position;
}

//// END : Node
///////////////////////////////////////////

///////////////////////////////////////////
//// BEGIN : NodeManager

NodeManager::NodeManager(Material* programPtr)
	: m_programPtr(programPtr)
	, m_draggedNode(nullptr)
	, m_isDraggingNode(false)
	, m_dragAnchorPos(0, 0)
	, m_draggedLink(nullptr)
	, m_isDraggingLink(false)
	, m_backgroundDecal(0, 0)
{
	auto newFinalNode = std::make_shared<FinalNode>();
	m_allNodes.push_back(newFinalNode);
	m_finalOutput = newFinalNode.get();

	// Init Mesh Visualizer
	m_meshVisualizer.setMesh(getMeshFactory().getDefault("cube"));
}

NodeManager::~NodeManager()
{
	if (m_programPtr != nullptr)
	{
		m_programPtr->resetNodeManagerPtr();
	}
}

void NodeManager::compile()
{
	//m_compileStream.clear();
	//m_compileStream.str(std::string());

	CompilationErrorCheck errorCheck;
	internalOnBeforeCompilation(errorCheck);
	internalResolveUndeterminedTypes();
	//internalDefineParameters(m_compileStream, errorCheck);
	internalCompile(errorCheck);
	if (errorCheck.compilationFailed)
		m_lastCompilationSucceeded = false;
	else
	{
		//m_compileStream << m_finalOutput->finalValueStr;
		//m_compileResult = m_compileStream.str();
		m_compileResult = m_finalOutput->finalValueStr;

		// Send new uniforms to the shader program
		std::vector<std::shared_ptr<InternalShaderParameterBase>> m_internalParameters;
		for (auto parameter : m_parameterNodes)
		{
			m_internalParameters.push_back(parameter->getInternalParameter()->cloneShared());
		}
		m_programPtr->setInternalParameters(m_internalParameters);

		// Compile the shader program
		m_programPtr->compile(m_compileResult);
	}

	// For Mesh Visualizer
	m_meshVisualizer.setMaterial(std::make_shared<MaterialInstance>(*m_programPtr));
}

void NodeManager::internalOnBeforeCompilation(CompilationErrorCheck & errorCheck)
{
	for (auto node : m_allNodes)
	{
		node->onBeforeCompile(errorCheck);
	}
}

void NodeManager::internalResolveUndeterminedTypes()
{
	m_finalOutput->resolveUndeterminedTypes();
}

//void NodeManager::internalDefineParameters(std::stringstream& compileStream, CompilationErrorCheck& errorCheck)
//{
//	m_finalOutput->defineParameter(compileStream, errorCheck);
//}

void NodeManager::internalCompile(CompilationErrorCheck& errorCheck)
{
	m_finalOutput->compile(errorCheck);
}

Node* NodeManager::getNode(const ID& id) const
{
	auto foundIt = std::find_if(m_allNodes.begin(), m_allNodes.end(), [&id](const std::shared_ptr<Node>& item) { return item->nodeID == id; });
	if (foundIt != m_allNodes.end())
		return foundIt->get();
	else
		return nullptr;
}

void NodeManager::save(Json::Value& root) const
{
	root.clear();

	root["nodesCount"] = m_allNodes.size();
	for (auto node : m_allNodes)
	{
		root["nodes"] = Json::Value(Json::objectValue);
		node->save(root["nodes"]);
	}

	root["linksCount"] = m_allLinks.size();
	for (auto link : m_allLinks)
	{
		root["links"] = Json::Value(Json::objectValue);
		link->save(root["links"]);
	}
}

void NodeManager::load(const Json::Value& root)
{
	// Clear
	m_allNodes.clear();
	m_allLinks.clear();
	m_finalOutput = nullptr;
	m_parameterNodes.clear();

	// Load nodes
	int nodeCount = root["nodesCount"].asInt();
	for (int i = 0; i < nodeCount; i++)
	{
		// Create
		std::string nodeName = root["nodes"][i]["name"].asString();
		NodeType nodeType = (NodeType)root["nodes"][i]["type"].asInt();
		// Load
		std::shared_ptr<Node> newNode = NodeFactory::instance().getSharedNodeInstance(nodeName);
		newNode->load(root["nodes"][i]);
		// Add
		m_allNodes.push_back(newNode);

		// Store refs to special nodes
		if (nodeType == NodeType::PARAMETER)
		{
			assert(dynamic_cast<BaseParameterNode*>(newNode.get()) != nullptr);
			m_parameterNodes.push_back(static_cast<BaseParameterNode*>(newNode.get()));
		}
		else if (nodeType == NodeType::FINAL)
		{
			m_finalOutput = static_cast<FinalNode*>(newNode.get()); //TODO : Make it safer
		}
	}
	// Make sur we have at least an output node
	if (m_finalOutput == nullptr)
	{
		auto newFinalNode = std::make_shared<FinalNode>();
		m_allNodes.push_back(newFinalNode);
		m_finalOutput = newFinalNode.get();
	}

	// Load links
	int linkCount = root["linksCount"].asInt();
	for (int i = 0; i < linkCount; i++)
	{
		// Create
		std::shared_ptr<Link> newLink = std::make_shared<Link>(nullptr, nullptr);
		// Load
		newLink->load(root["links"][i]);
		Node* leftNode = getNode(newLink->inputNodeID);
		Node* rightNode = getNode(newLink->outputNodeID);
		assert(newLink->inputIdx >= 0 && newLink->inputIdx < leftNode->outputs.size());
		newLink->input = leftNode->outputs[newLink->inputIdx].get();
		assert(newLink->outputIdx >= 0 && newLink->outputIdx  < leftNode->inputs.size());
		newLink->output = rightNode->inputs[newLink->outputIdx].get();
		// Add
		m_allLinks.push_back(newLink);
	}
}

void NodeManager::drawUI(Renderer& renderer)
{
	const float bottomHeight = 200;
	const float bottomLeftWidth = ImGui::GetWindowWidth() * 0.3;
	const float bottomMiddleWidth = ImGui::GetWindowWidth() * 0.4;
	const float bottomRightWidth = ImGui::GetWindowWidth() * 0.3;

	ImGui::BeginChild("##nodeContent", ImVec2(0, ImGui::GetWindowHeight() - bottomHeight));

	// Draw background
	const ImVec2 windowPos = ImGui::GetWindowPos();
	const float lineTickness = 1.f;
	const float spaceBetweenLines = 20;
	float decalX = glm::mod(m_backgroundDecal.x, spaceBetweenLines) - spaceBetweenLines;
	float decalY = glm::mod(m_backgroundDecal.y, spaceBetweenLines) - spaceBetweenLines;
	const float numLinesX = (ImGui::GetWindowWidth() / spaceBetweenLines) + 1;
	const float numLinesY = (ImGui::GetWindowHeight() / spaceBetweenLines) + 1;
	const ImColor lineColor(0.5f, 0.5f, 0.5f, 0.7f);
	for (int i = 0; i < numLinesX + 1; i++)
	{
		decalX += (spaceBetweenLines + lineTickness);

		ImVec2 a(windowPos.x + ImGui::GetWindowContentRegionMin().x + decalX, windowPos.y + ImGui::GetWindowContentRegionMin().y - spaceBetweenLines);
		ImVec2 b(windowPos.x + ImGui::GetWindowContentRegionMin().x + decalX, windowPos.y + ImGui::GetWindowContentRegionMax().y + spaceBetweenLines);
		ImGui::GetWindowDrawList()->AddLine(a, b, lineColor, lineTickness);
	}
	for (int i = 0; i < numLinesY + 1; i++)
	{
		decalY += (spaceBetweenLines + lineTickness);

		ImVec2 a(windowPos.x + ImGui::GetWindowContentRegionMin().x, windowPos.y + ImGui::GetWindowContentRegionMin().y + decalY);
		ImVec2 b(windowPos.x + ImGui::GetWindowContentRegionMax().x, windowPos.y + ImGui::GetWindowContentRegionMin().y + decalY);
		ImGui::GetWindowDrawList()->AddLine(a, b, lineColor, lineTickness);
	}

	// Draw nodes
	for (auto node : m_allNodes)
	{
		node->drawUI(*this);
	}

	// Draw links
	for (auto link : m_allLinks)
	{
		link->drawUI(*this);
	}

	// Move all nodes
	if (ImGui::IsMouseHoveringWindow() && ImGui::IsKeyDown(GLFW_KEY_SPACE) && ImGui::IsMouseDown(0) && !m_isHoverridingANode && !DragAndDropManager::isDragAndDropping())
	{
		ImVec2 deltaDrag = ImGui::GetMouseDragDelta(0);
		for (auto node : m_allNodes)
		{
			node->position.x += deltaDrag.x;
			node->position.y += deltaDrag.y;
		}
		m_backgroundDecal.x += deltaDrag.x;
		m_backgroundDecal.y += deltaDrag.y;
		ImGui::ResetMouseDragDelta(0);
	}
	// Select multiple nodes
	else if (ImGui::IsMouseHoveringWindow() && ImGui::IsMouseDown(0) && !m_isDraggingLink && !m_isDraggingNode && !DragAndDropManager::isDragAndDropping())
	{
		ImVec2 deltaDrag = ImGui::GetMouseDragDelta(0);
		ImVec2 mousePos = ImGui::GetMousePos();
		if (!m_isSelectingNodes)
		{
			m_dragAnchorPos = glm::vec2(mousePos.x, mousePos.y);
			m_isSelectingNodes = true;
		}
		ImVec2 a(std::min(m_dragAnchorPos.x, mousePos.x), std::min(mousePos.y, m_dragAnchorPos.y));
		ImVec2 b(std::max(m_dragAnchorPos.x, mousePos.x), std::max(mousePos.y, m_dragAnchorPos.y));
		ImGui::GetWindowDrawList()->AddRect(a, b, ImColor(1.f, 1.f, 0.f, 1.f));

		for (auto node : m_allNodes)
		{
			bool selected = false;
			if ((node->position.x >= a.x && node->position.x <= b.x)
				&& (node->position.y >= a.y && node->position.y <= b.y))
				selected = true;
			else if ((node->position.x + node->size.x >= a.x && node->position.x + node->size.x <= b.x)
				&& (node->position.y + node->size.y >= a.y && node->position.y + node->size.y <= b.y))
				selected = true;

			if (selected)
			{
				if(std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node.get()) == m_selectedNodes.end())
					m_selectedNodes.push_back(node.get());
			}
			else
			{
				m_selectedNodes.erase(std::remove_if(m_selectedNodes.begin(), m_selectedNodes.end(), [&node](const Node* item) { return item == node.get(); }), m_selectedNodes.end());
			}
		}
	}

	// Add a new node
	if (ImGui::IsMouseReleased(1) && !m_isHoverridingANode && ImGui::IsWindowHovered())
	{
		m_newNodePos = ImGui::GetMousePos();
		ImGui::OpenPopup("selectNodePopUp");
	}
	if (ImGui::BeginPopup("selectNodePopUp"))
	{
		int currentType = -1;
		for (auto it = NodeFactory::instance().begin(); it != NodeFactory::instance().end(); it++)
		{
			if (currentType != (int)(*it)->type)
			{
				ImGui::Text(NodeTypeToString[(int)(*it)->type].c_str());
				currentType = (int)(*it)->type;
			}

			bool canDisplay = (*it)->type != NodeType::CUSTOM ? true : ((*it)->name != "ReflectionTexture" ? true : (m_programPtr != nullptr && m_programPtr->getMaterialUsage() == Rendering::MaterialUsage::REFLECTIVE_PLANE));

			if (canDisplay)
			{
				if (ImGui::Button((*it)->name.c_str()))
				{
					auto newNode = (*it)->cloneShared();

					const glm::vec2 nodePosition(m_newNodePos.x - windowPos.x/* + m_backgroundDecal.x*/, m_newNodePos.y - windowPos.y/* + m_backgroundDecal.y*/);
					newNode->setPosition(nodePosition);

					m_allNodes.push_back(newNode);

					ImGui::CloseCurrentPopup();
				}
			}
		}

		ImGui::EndPopup();
	}

	// Delete nodes
	if (!m_selectedNodes.empty() && ImGui::IsKeyPressed(GLFW_KEY_DELETE) && !m_isDraggingLink)
	{
		for (int i = 0; i < m_selectedNodes.size(); i++)
		{
			if (m_selectedNodes[i]->type != NodeType::FINAL)
			{
				removeNode(m_selectedNodes[i]);

				setIsDraggingLink(false);
				setDraggedLink(nullptr);
				setIsDraggingNode(false);
				setDraggedNode(nullptr);
			}
		}
		m_selectedNodes.clear();
	}

	// Drag nodes
	if (isDraggingNode() && m_draggedNode != nullptr)
	{
		ImVec2 deltaDrag = ImGui::GetMouseDragDelta(0);
		//glm::vec2 dragDelta(ImGui::GetMousePos().x - m_dragAnchorPos.x, ImGui::GetMousePos().y - m_dragAnchorPos.y);
		m_draggedNode->position.x += deltaDrag.x;
		m_draggedNode->position.y += deltaDrag.y;

		bool canDragSelection = false;
		for (auto node : m_selectedNodes)
		{
			if (m_draggedNode == node)
				canDragSelection = true;
		}
		if (canDragSelection)
		{
			for (auto node : m_selectedNodes)
			{
				if (node == m_draggedNode)
					continue;

				node->position.x += deltaDrag.x;
				node->position.y += deltaDrag.y;
			}
		}

		ImGui::ResetMouseDragDelta(0);
	}
	// Drag links
	else if (m_isDraggingLink && m_draggedLink != nullptr)
	{
		if (m_draggedLink->input == nullptr && m_draggedLink->output != nullptr)
		{
			ImVec2 A = ImGui::GetMousePos();
			ImVec2 A_(A.x + 100.f, A.y);
			ImVec2 B(m_draggedLink->output->position.x, m_draggedLink->output->position.y);
			ImVec2 B_(B.x - 100.f, B.y);

			ImGui::GetWindowDrawList()->AddBezierCurve(A, A_, B_, B, ImColor(0.5f, 0.5f, 0.5f, 1.0f), 2);
		}
		else if (m_draggedLink->input != nullptr && m_draggedLink->output == nullptr)
		{
			ImVec2 A(m_draggedLink->input->position.x, m_draggedLink->input->position.y);
			ImVec2 A_(A.x + 100.f, A.y);
			ImVec2 B = ImGui::GetMousePos();
			ImVec2 B_(B.x - 100.f, B.y);

			ImGui::GetWindowDrawList()->AddBezierCurve(A, A_, B_, B, ImColor(0.5f, 0.5f, 0.5f, 1.0f), 2);
		}
	}

	if (ImGui::IsMouseReleased(0))
	{
		if (m_isDraggingNode)
		{
			setIsDraggingNode(false);
			setDraggedNode(nullptr);
		}

		if(m_isSelectingNodes)
			m_isSelectingNodes = false;
	}

	if (ImGui::IsMouseClickedAnyButton() && m_isDraggingLink && m_canResetDragLink)
	{
		if (m_draggedLink != nullptr)
		{
			removeLink(m_draggedLink);
		}

		setIsDraggingLink(false);
		setDraggedLink(nullptr);
	}

	if (ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClickedAnyButton() && !m_isHoverridingANode)
	{
		m_selectedNodes.clear();
	}

	m_canResetDragLink = true;
	m_isHoverridingANode = false;

	ImGui::EndChild();

	//////////////////////////////////////

	ImGui::BeginChild("##bottomWindow", ImVec2(0, bottomHeight - 8), true);

	ImGui::BeginChild("##parameters", ImVec2(bottomLeftWidth, 0), true);
	if (m_selectedNodes.size() == 1)
	{
		m_selectedNodes[0]->drawUIParameters();
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("##option", ImVec2(bottomMiddleWidth, 0), true);
	assert(m_programPtr != nullptr);
	bool needRecompilation = false;
	bool materialUsageChanged = m_programPtr->drawUIMaterialUsage();
	materialUsageChanged |= needRecompilation;
	needRecompilation |= m_programPtr->drawUIPipelineType();
	//bool drawWithReflectionChanged = m_programPtr->drawUIUsedWithReflections();
	//needRecompilation |= drawWithReflectionChanged;
	if (materialUsageChanged)
	{
		if (m_programPtr->getMaterialUsage() != Rendering::MaterialUsage::REFLECTIVE_PLANE)
		{
			for (auto it = m_allNodes.begin(); it < m_allNodes.end();)
			{
				if ((*it)->type == NodeType::CUSTOM && (*it)->name == "ReflectionTexture")
				{
					it = removeNode(it);					
				}
				else
					it++;
			}
		}
	}
	needRecompilation |= m_programPtr->drawUIUsedWithSkeleton();

	if (ImGui::Button("Compile"))
	{
		compile();
	}

	ImGui::SameLine();

	if (ImGui::Button("Show textual result"))
	{
		ImGui::OpenPopup("ShowResultPopUp");
	}

	if (ImGui::BeginPopupModal("ShowResultPopUp"))
	{
		if (ImGui::Button("close"))
			ImGui::CloseCurrentPopup();

		ImGui::Text(m_compileResult.c_str());

		ImGui::EndPopup();
	}

	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("##result", ImVec2(bottomRightWidth - 8, 0), true);
	m_meshVisualizer.render(renderer);
	m_meshVisualizer.drawUI();
	ImGui::EndChild();

	ImGui::EndChild();
}

bool NodeManager::isDraggingNode() const
{
	return m_isDraggingNode;
}

void NodeManager::setIsDraggingNode(bool state)
{
	m_isDraggingNode = state;
}

void NodeManager::setDragAnchorPos(const glm::vec2 & position)
{
	m_dragAnchorPos = position;
}

const glm::vec2 & NodeManager::getDragAnchorPos() const
{
	return m_dragAnchorPos;
}

Node * NodeManager::getDraggedNode() const
{
	return m_draggedNode;
}

void NodeManager::setDraggedNode(Node * node)
{
	m_draggedNode = node;
}

void NodeManager::setIsDraggingLink(bool state)
{
	m_isDraggingLink = state;
}

bool NodeManager::getIsDraggingLink() const
{
	return m_isDraggingLink;
}

void NodeManager::setDraggedLink(Link * link)
{
	m_draggedLink = link;
}

Link * NodeManager::getDraggedLink() const
{
	return m_draggedLink;
}

void NodeManager::setCanResetDragLink(bool state)
{
	m_canResetDragLink = state;
}

void NodeManager::setSelectedNode(Node * node)
{
	m_selectedNodes.clear();
	m_selectedNodes.push_back(node);
}

bool NodeManager::isSelectingNode(Node* node) const
{
	return std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node) != m_selectedNodes.end();
}

void NodeManager::setIsHoverridingANode(bool state)
{
	m_isHoverridingANode = state;
}

void NodeManager::addLink(std::shared_ptr<Link> link)
{
	m_allLinks.push_back(link);
}

void NodeManager::removeLink(Link * link)
{
	if (link->input != nullptr && link->input->parentNode != nullptr)
	{
		link->input->links.erase(std::remove(link->input->links.begin(), link->input->links.end(), link), link->input->links.end());
	}
	if (link->output != nullptr && link->output->parentNode != nullptr)
	{
		link->output->link = nullptr;
	}

	m_allLinks.erase(std::remove_if(m_allLinks.begin(), m_allLinks.end(), [link](const std::shared_ptr<Link>& item) { return item.get() == link; }), m_allLinks.end());

	link = nullptr;
}

std::vector<std::shared_ptr<Link>>::iterator NodeManager::removeLink(std::vector<std::shared_ptr<Link>>::iterator linkIt)
{
	if ((*linkIt)->input != nullptr && (*linkIt)->input->parentNode != nullptr)
	{
		(*linkIt)->input->links.erase(std::remove((*linkIt)->input->links.begin(), (*linkIt)->input->links.end(), linkIt->get()));

	}

	if ((*linkIt)->output != nullptr && (*linkIt)->output->parentNode != nullptr)
	{
		(*linkIt)->output->link = nullptr;
	}

	return m_allLinks.erase(linkIt);
}

void NodeManager::removeNode(Node * node)
{
	for (auto input : node->inputs)
	{
		if (input->link != nullptr)
		{
			removeLink(input->link);
		}
	}

	for (auto output : node->outputs)
	{
		for (auto it = output->links.begin(); it != output->links.end();)
		{
			removeLink(*it);
		}
	}

	m_allNodes.erase(std::remove_if(m_allNodes.begin(), m_allNodes.end(), [node](const std::shared_ptr<Node>& item) { return item.get() == node; }), m_allNodes.end());

	node = nullptr;
}

std::vector<std::shared_ptr<Node>>::iterator NodeManager::removeNode(std::vector<std::shared_ptr<Node>>::iterator nodeIt)
{
	for (auto input : (*nodeIt)->inputs)
	{
		if (input->link != nullptr)
		{
			removeLink(input->link);
		}
	}

	for (auto output : (*nodeIt)->outputs)
	{
		for (auto it = output->links.begin(); it != output->links.end();)
		{
			removeLink(*it);
		}
	}

	m_selectedNodes.clear();
	m_isSelectingNodes = false;
	m_isDraggingLink = false;
	m_isDraggingNode = false;
	m_draggedLink = nullptr;
	m_draggedNode = nullptr;

	return m_allNodes.erase(nodeIt);
}

//// END : NodeManager
///////////////////////////////////////////

}
