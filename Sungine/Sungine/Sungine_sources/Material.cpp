#include "Material.h"
#include "RenderBatch.h"
#include "ShaderParameters.h"
#include "Material.h"
#include "Factories.h"
#include "ResourceTree.h"
#include "Editor.h"
#include "MaterialVisualScripting.h"
#include "EditorFrames.h"
#include "MaterialInstance.h"
#include "Project.h"

std::string Material::s_extention = ".glProg";

//////////////////////////////////////////////////////////
// Constructors / Operators
//////////////////////////////////////////////////////////
Material::Material()
	: Asset(Object::getStaticClassId<Material>())
	, m_materialType(Rendering::MaterialType::INTERNAL)
	//, m_usedWithReflections(false)
	, m_usedWithSkeleton(false)
	, m_pipeline(Rendering::PipelineType::CUSTOM_PIPELINE)
	, m_usage(Rendering::MaterialUsage::MESH)
	//:id(0)
{
}

Material::Material(const FileHandler::CompletePath& path, Rendering::MaterialType type)
	: Asset(Object::getStaticClassId<Material>())
	, m_materialType(type)
	//, m_usedWithReflections(false)
	, m_usedWithSkeleton(false)
	, m_pipeline(Rendering::PipelineType::CUSTOM_PIPELINE)
	, m_usage(Rendering::MaterialUsage::MESH)
	//, id(0)
{
	if(m_materialType != Rendering::MaterialType::INTERNAL)
		m_nodeManager = std::make_shared<MVS::NodeManager>(this);
}
//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
// Asset override
//////////////////////////////////////////////////////////
void Material::createNewAssetFile(const FileHandler::CompletePath& filePath)
{
	saveToFile();
}

void Material::loadFromFile(const FileHandler::CompletePath& filePath)
{
	Asset::loadFromFile(filePath);

	//////////////////////////////////////
	// Path to Json value
	//////////////////////////////////////
	assert(!Project::isPathPointingInsideProjectFolder(filePath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(filePath);

	std::ifstream stream;
	if (!Utils::OpenFileStreamRead(absolutePath, stream))
	{
		std::cout << "error, can't load shader program at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;
	stream >> root;
	//////////////////////////////////////

	// MaterialInstance Type
	auto foundItMaterialType = std::find(Rendering::MaterialTypeToString.begin(), Rendering::MaterialTypeToString.end(), root.get("materialType", "internal").asString());
	assert(foundItMaterialType != Rendering::MaterialTypeToString.end());
	int foundIdxMaterialType = foundItMaterialType - Rendering::MaterialTypeToString.begin();
	m_materialType = (Rendering::MaterialType)foundIdxMaterialType;

	m_usage = Utils::stringToEnum<Rendering::MaterialUsage>(root.get("materialUsage", "custom").asString(), Rendering::MaterialUsageToString);
	m_pipeline = Utils::stringToEnum<Rendering::PipelineType>(root.get("materialPipeline", "custom").asString(), Rendering::PipelineTypesToString);
	//m_usedWithReflections = root.get("usedWithReflection", false).asBool();
	m_usedWithSkeleton = root.get("usedWithSkeleton", false).asBool();


	if (m_materialType == Rendering::MaterialType::DEFAULT)
	{
		m_computeShaderParameterFunction = root.get("computeShaderParameterFunction", "").asString();

		m_programId = makeGLProgram();
	}
	else if (m_materialType == Rendering::MaterialType::INTERNAL)
	{
		const std::string vertexShaderName = root.get("vertex", "").asString();
		const std::string fragmentShaderName = root.get("fragment", "").asString();
		const std::string geometryShaderName = root.get("geometry", "").asString();

		m_programId = makeGLProgramForInternal(absolutePath, vertexShaderName, fragmentShaderName, geometryShaderName);
	}
	else
		assert(false && "Wrong material type !");

	makeMaterialAggregates();

	// Internal parameters
	Json::Value internalShaderParameters = root["internalShaderParameters"];
	int inernalShaderParameterCount = internalShaderParameters.size();

	m_internalShaderParameters.clear();
	for (int i = 0; i < inernalShaderParameterCount; i++)
	{
		assert(internalShaderParameters.isValidIndex(i));

		auto newParameter = MakeNewInternalShaderParameter(internalShaderParameters[i]);
		newParameter->init(m_programId); //don't forget to init the parameter to get the uniforms
		m_internalShaderParameters.push_back(newParameter);
	}

	///////////////////////////////////
	// Load Nodes
	///////////////////////////////////
	if (m_materialType != Rendering::MaterialType::INTERNAL)
		m_nodeManager = std::make_shared<MVS::NodeManager>(this);

	if (m_nodeManager)
		m_nodeManager->load(root["MVS"]);
	///////////////////////////////////
}

void Material::saveToFile()
{
	Asset::saveToFile();

	////////////////////////////////////////////////////////
	// Open stream, create Json value
	////////////////////////////////////////////////////////
	assert(!Project::isPathPointingInsideProjectFolder(m_assetPath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_assetPath);

	std::ofstream stream;
	if(!Utils::OpenFileStreamWrite(absolutePath, stream))
	{
		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;
	////////////////////////////////////////////////////////

	// MaterialInstance Type
	root["materialType"] = Utils::enumToString(m_materialType, Rendering::MaterialTypeToString);
	root["materialUsage"] = Utils::enumToString(m_usage, Rendering::MaterialUsageToString);
	root["materialPipeline"] = Utils::enumToString(m_pipeline, Rendering::PipelineTypesToString);
	//root["usedWithReflection"] = m_usedWithReflections;
	root["usedWithSkeleton"] = m_usedWithSkeleton;
	// Compilation result
	root["computeShaderParameterFunction"] = m_computeShaderParameterFunction;

	// Visual scripting nodes
	if (m_materialType == Rendering::MaterialType::DEFAULT)
	{
		if (m_nodeManager)
			m_nodeManager->save(root["MVS"]);
	}
	else
		assert(false && "Wrong material type !");

	// Internal parameters
	for (int i = 0; i < m_internalShaderParameters.size(); i++)
	{
		m_internalShaderParameters[i]->save(root["internalShaderParameters"][i]);
	}

	// Refs to instances
	for (int i = 0; i < m_materialRefs.size(); i++)
	{
		m_materialRefs[i]->save(root["materialRefs"][i]);
	}

	////////////////////////////////////////////////////////
	// Send datas to stream
	////////////////////////////////////////////////////////
	stream << root;
	////////////////////////////////////////////////////////
}

void Material::saveMetas()
{
	Asset::saveMetas();
}

void Material::loadMetas()
{
	Asset::loadMetas();
}

void Material::drawInInspector()
{
	assert(m_nodeManager);

	drawUIMaterialUsage();
	drawUIPipelineType();
	//drawUIUsedWithReflections();
	drawUIUsedWithSkeleton();

	if (ImGui::Button("Edit"))
	{
		if (m_nodeManagerEditorFrameRef.expired() || m_nodeManagerEditorFrameRef.use_count() == 0)
		{
			Editor::instance().getWindowManager()->addWindowAsynchrone(std::make_shared<MVSEditorFrame>(m_nodeManager.get()));
		}
	}

	if (m_needRecompilation)
	{
		if (ImGui::Button("Compile"))
		{
			compile();
		}
	}
}

bool Material::drawRightClicContextMenu(std::string& popupToOpen)
{
	if (ImGui::Button("create material from this program."))
	{
		popupToOpen = "AddMaterialPopUp";
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////

const std::unordered_map<std::string, std::shared_ptr<MaterialAggregation>>& Material::getAggregations() const
{
	return m_aggregations;
}

const std::unordered_map<std::string, std::shared_ptr<PerInstanceMaterialAggregation>>& Material::getPerInstanceAggregation() const
{
	return m_perInstanceAggregations;
}

GLuint Material::getProgramId() const
{
	return m_programId;
}

void Material::compile()
{
	m_programId = makeGLProgram();
	makeMaterialAggregates();
}

void Material::compile(const std::string& computeShaderParameters)
{
	m_computeShaderParameterFunction = computeShaderParameters;
	m_programId = makeGLProgram();
	makeMaterialAggregates();
	for (auto material : m_materialRefs)
	{
		material->setAggregates(m_aggregations);
		material->setPerInstanceAggregates(m_perInstanceAggregations);
		material->setProgramId(m_programId);
		material->setinternalParameters(m_internalShaderParameters);
	}
}

void Material::resetNodeManagerPtr()
{
	m_nodeManager.reset();
}

void Material::makeMaterialAggregates()
{
	m_perInstanceAggregations.clear();
	m_aggregations.clear();

	if (m_pipeline == Rendering::PipelineType::FORWARD_PIPELINE)
	{
		auto aggregation = std::make_shared<MaterialAggregationForward>();
		aggregation->initParameters(m_programId);
		m_aggregations["forward"] = aggregation;
	}
	else if (m_pipeline == Rendering::PipelineType::DEFERRED_PIPILINE)
	{
		// NOTHING
	}

	if (m_usedWithSkeleton)
	{
		auto aggregation = std::make_shared<MaterialAggregationWithSkeleton>();
		aggregation->initParameters(m_programId);
		m_perInstanceAggregations["skeleton"] = aggregation;
	}
	else
	{
		//TODO
	}

	//if (m_usedWithReflections)
	//{
	//	auto aggregation = std::make_shared<MaterialAggregationWithReflection>();
	//	aggregation->initParameters(m_programId);
	//	m_perInstanceAggregations["reflection"] = aggregation;
	//}
	//else
	//{
	//	//TODO
	//}

	if (m_usage == Rendering::MaterialUsage::MESH)
	{
		auto aggregation = std::make_shared<MaterialAggregationMesh>();
		aggregation->initParameters(m_programId);
		m_perInstanceAggregations["mesh"] = aggregation;
	}
	else if (m_usage == Rendering::MaterialUsage::BILLBOARD)
	{
		auto aggregation = std::make_shared<MaterialAggregationBillboard>();
		aggregation->initParameters(m_programId);
		m_perInstanceAggregations["billboard"] = aggregation;
	}
	else if (m_usage == Rendering::MaterialUsage::PARTICLES)
	{
		auto aggregation = std::make_shared<MaterialAggregationParticles>();
		aggregation->initParameters(m_programId);
		m_perInstanceAggregations["particles"] = aggregation;
	}
	else if (m_usage == Rendering::MaterialUsage::REFLECTIVE_PLANE)
	{
		auto aggregation = std::make_shared<MaterialAggregationReflectivePlane>();
		aggregation->initParameters(m_programId);
		m_perInstanceAggregations["reflective¨Plane"] = aggregation;
	}
}

GLuint Material::makeGLProgramForInternal(const FileHandler::CompletePath& shaderFolderPath, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::string& geometryShaderName)
{
	bool hasVertShader = !vertexShaderName.empty();
	bool hasFragShader = !fragmentShaderName.empty();
	bool hasGeomShader = !geometryShaderName.empty();

	GLuint vertShaderId = 0;
	GLuint fragShaderId = 0;
	GLuint geomShaderId = 0;

	if (hasVertShader)
	{
		const FileHandler::CompletePath& vertexAbsolutePath(shaderFolderPath.getPath().toString() + "/" + vertexShaderName);
		vertShaderId = compile_shader_from_file(GL_VERTEX_SHADER, vertexAbsolutePath.c_str());
	}
	if (hasFragShader)
	{
		const FileHandler::CompletePath& fragmentAbsolutePath(shaderFolderPath.getPath().toString() + "/" + fragmentShaderName);
		fragShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, fragmentAbsolutePath.c_str());
	}
	if (hasGeomShader)
	{
		const FileHandler::CompletePath& geometryAbsolutePath(shaderFolderPath.getPath().toString() + "/" + geometryShaderName);
		geomShaderId = compile_shader_from_file(GL_GEOMETRY_SHADER, geometryAbsolutePath.c_str());
	}

	GLuint programObject = glCreateProgram();
	if (hasVertShader)
		glAttachShader(programObject, vertShaderId);
	if (hasFragShader)
		glAttachShader(programObject, fragShaderId);
	if (hasGeomShader)
		glAttachShader(programObject, geomShaderId);

	glLinkProgram(programObject);
	if (check_link_error(programObject) < 0)
		assert(false && "error in shader code.");

	//check uniform errors : 
	ASSERT_GL_ERROR("error in shader program compilation.");

	return programObject;
}

GLuint Material::makeGLProgramForDefault(const FileHandler::CompletePath& shaderFolderPath, const std::string& fragmentShaderName)
{
	///////////////////

	// Get path to compute shader params functions
	bool hasFragShader = !fragmentShaderName.empty();

	std::string computeParamsVertexShaderPath;
	std::string computeParamsFragmentShaderPath;

	if (hasFragShader)
	{
		const FileHandler::CompletePath& fragmentAbsolutePath(shaderFolderPath.getPath().toString() + "/" + fragmentShaderName);
		computeParamsFragmentShaderPath = fragmentAbsolutePath.toString();
	}

	///////////////////

	// Fragment shader : 
	std::stringstream vertexShaderStream;
	std::stringstream fragmentShaderStream;

	std::string internalShaderFolderPath(FileHandler::Path(Project::getShaderFolderPath().toString() + "/internals/").toString());

	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "header.frag");

	fillShaderStream(fragmentShaderStream, computeParamsFragmentShaderPath);

	if (m_pipeline == Rendering::PipelineType::FORWARD_PIPELINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "forward.frag");
	}
	else if (m_pipeline == Rendering::PipelineType::DEFERRED_PIPILINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "deferred.frag");
	}

	// Vertex shader :
	fillShaderStream(vertexShaderStream, internalShaderFolderPath + "header.vert");

	if (m_usedWithSkeleton)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withSkeleton.vert");
	}
	else
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withoutSkeleton.vert");
	}

	//if (m_usedWithReflections)
	//{
	//	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withReflection.vert");
	//}
	//else
	//{
	//	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withoutReflection.vert");
	//}

	if (m_usage == Rendering::MaterialUsage::MESH)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "mesh.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::BILLBOARD)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "billboard.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::PARTICLES)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "particles.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::REFLECTIVE_PLANE)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "reflectivePlane.vert");
	}

	return makeGLProgram(vertexShaderStream.str(), fragmentShaderStream.str());
}

GLuint Material::makeGLProgram()
{
	std::stringstream vertexShaderStream;
	std::stringstream fragmentShaderStream;

	std::string internalShaderFolderPath( FileHandler::Path(Project::getShaderFolderPath().toString() + "/internals/").toString() );

	// Fragment shader : 
	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "header.frag");

	if (m_pipeline == Rendering::PipelineType::FORWARD_PIPELINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "header_forward.frag");
	}
	else if (m_pipeline == Rendering::PipelineType::DEFERRED_PIPILINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "header_deferred.frag");
	}

	if (m_computeShaderParameterFunction == "")
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "defaultComputeShaderParams.frag");
	else
		fragmentShaderStream << m_computeShaderParameterFunction;

	//if (m_usedWithReflections)
	//{
	//	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withReflection.vert");
	//}
	//else
	//{
	//	fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "withoutReflection.vert");
	//}

	if (m_pipeline == Rendering::PipelineType::FORWARD_PIPELINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "forward.frag");
	}
	else if (m_pipeline == Rendering::PipelineType::DEFERRED_PIPILINE)
	{
		fillShaderStream(fragmentShaderStream, internalShaderFolderPath + "deferred.frag");
	}

	// Vertex shader : 
	fillShaderStream(vertexShaderStream, internalShaderFolderPath + "header.vert");
	if (m_usedWithSkeleton)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "withSkeleton.vert");
	}
	else
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "withoutSkeleton.vert");
	}

	if (m_usage == Rendering::MaterialUsage::MESH)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "mesh.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::BILLBOARD)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "billboard.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::PARTICLES)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "particles.vert");
	}
	else if (m_usage == Rendering::MaterialUsage::REFLECTIVE_PLANE)
	{
		fillShaderStream(vertexShaderStream, internalShaderFolderPath + "reflectivePlane.vert");
	}

	return makeGLProgram(vertexShaderStream.str(), fragmentShaderStream.str());
}

GLuint Material::makeGLProgram(const std::string & vertexShaderContent, const std::string & fragmentShaderContent)
{
	GLuint vertexShaderId = compile_shader_from_string(GL_VERTEX_SHADER, vertexShaderContent);
	GLuint fragmentShaderId = compile_shader_from_string(GL_FRAGMENT_SHADER, fragmentShaderContent);


	GLuint programObject = glCreateProgram();
	glAttachShader(programObject, vertexShaderId);
	glAttachShader(programObject, fragmentShaderId);

	glLinkProgram(programObject);
	if (check_link_error(programObject) < 0)
		assert(false && "error in shader code.");

	//check uniform errors : 
	ASSERT_GL_ERROR("error in shader program compilation.");

	// Avoid memory leaks
	glDetachShader(programObject, vertexShaderId);
	glDetachShader(programObject, fragmentShaderId);
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	return programObject;
}

//bool Material::implementPipeline(Rendering::PipelineType pipelineType) const
//{
//	return ids.find(pipelineType) != ids.end();
//}
//
//int Material::getImplementedPipelineCount() const
//{
//	return m_pipelineHandlingTypes.size();
//}
//
//Rendering::PipelineType Material::getImplementedPipeline(int idx) const
//{
//	assert(idx >= 0 && idx < m_pipelineHandlingTypes.size());
//	return m_pipelineHandlingTypes[idx];
//}
//
//GLuint Material::getProgramId(Rendering::PipelineType pipelineType) const
//{
//	assert(ids.find(pipelineType) != ids.end());
//	return ids.find(pipelineType)->second;
//}

void Material::LoadMaterialInstance(MaterialInstance* material) const
{
	//Rendering::PipelineType materialPipelineType = material->getPipelineType();
	//auto& foundProgramId = ids.find(materialPipelineType);
	//assert(foundProgramId != ids.end());

	material->loadFromShaderProgramDatas(m_programId/*foundProgramId->second*/, m_internalShaderParameters/*, m_externalShaderParameters*/);
}

void Material::addMaterialRef(const AssetHandle<MaterialInstance>& ref)
{
	if (std::find(m_materialRefs.begin(), m_materialRefs.end(), ref) == m_materialRefs.end())
		m_materialRefs.push_back(ref);
}

void Material::removeMaterialRef(const AssetHandle<MaterialInstance>& ref)
{
	m_materialRefs.erase(std::remove(m_materialRefs.begin(), m_materialRefs.end(), ref), m_materialRefs.end());
}

const std::vector<std::shared_ptr<InternalShaderParameterBase>>& Material::getInternalParameters() const
{
	return m_internalShaderParameters;
}

void Material::setInternalParameters(std::vector<std::shared_ptr<InternalShaderParameterBase>>& internals)
{
	m_internalShaderParameters.clear();
	for (auto internalParam : internals)
	{
		m_internalShaderParameters.push_back(internalParam->cloneShared());
	}
}

//const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& Material::getExternalParameters() const
//{
//	return m_externalShaderParameters;
//}

Rendering::MaterialType Material::getMaterialType() const
{
	return m_materialType;
}

Rendering::MaterialUsage Material::getMaterialUsage() const
{
	return m_usage;
}

Rendering::PipelineType Material::getPipelineType() const
{
	return m_pipeline;
}

//bool Material::getUsedWithReflections() const
//{
//	return m_usedWithReflections;
//}

bool Material::getUsedWithSkeletons() const
{
	return m_usedWithSkeleton;
}

void Material::setMaterialUsage(Rendering::MaterialUsage usage)
{
	m_usage = usage;
	m_needRecompilation = true;
}

void Material::setPipelineType(Rendering::PipelineType pipeline)
{
	m_pipeline = pipeline;
	m_needRecompilation = true;
}

//void Material::setUsedWithReflections(bool state)
//{
//	m_usedWithReflections = state;
//	m_needRecompilation = true;
//}

void Material::setUsedWithSkeletons(bool state)
{
	m_usedWithSkeleton = state;
	m_needRecompilation = true;
}

bool Material::drawUIMaterialUsage()
{
	int currentItemUsage = (int)m_usage;
	if (ImGui::Combo("material usage", &currentItemUsage, [](void* data, int idx, const char** out_text) {
		if (Rendering::MaterialUsageToString.size() > idx + 1) {
			*out_text = Rendering::MaterialUsageToString[idx].c_str();
			return true;
		}
		else return false;
	}, nullptr, (int)Rendering::MaterialUsageToString.size() - 1))
	{
		m_usage = (Rendering::MaterialUsage)currentItemUsage;
		m_needRecompilation = true;
		return true;
	}

	return false;
}

bool Material::drawUIPipelineType()
{
	int currentItemPipeline = (int)m_pipeline;
	if (ImGui::Combo("material pipeline", &currentItemPipeline, [](void* data, int idx, const char** out_text) {
		if (Rendering::PipelineTypesToString.size() > idx + 1) {
			*out_text = Rendering::PipelineTypesToString[idx].c_str(); return true;
		}
		else return false;
	}, nullptr, (int)Rendering::PipelineTypesToString.size() - 1))
	{
		m_pipeline = (Rendering::PipelineType)currentItemPipeline;
		m_needRecompilation = true;
		return true;
	}

	return false;
}

//bool Material::drawUIUsedWithReflections()
//{
//	if (ImGui::Checkbox("used with reflections", &m_usedWithReflections))
//	{
//		m_needRecompilation = true;
//		return true;
//	}
//	return false;
//}

bool Material::drawUIUsedWithSkeleton()
{
	if (ImGui::Checkbox("used with skeleton", &m_usedWithSkeleton))
	{
		m_needRecompilation = true;
		return true;
	}
	return false;
}

//Material::Material(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath)
//	: id(0)
//{
//	load(vertexShaderPath, fragmentShaderPath);
//}
//
//
//Material::Material(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath, const FileHandler::CompletePath& geometryShaderPath)
//	: id(0)
//{
//	load(vertexShaderPath, fragmentShaderPath, geometryShaderPath);
//}

//void Material::init(const FileHandler::CompletePath& path, const ID& id)
//{
//	Resource::init(path, id);
//
//	assert(!Project::isPathPointingInsideProjectFolder(path)); //path should be relative
//	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(path);
//
//	std::ifstream stream;
//	stream.open(absolutePath.toString());
//	if (!stream.is_open())
//	{
//		std::cout << "error, can't load shader program at path : " << absolutePath.toString() << std::endl;
//		return;
//	}
//	Json::Value root;
//	stream >> root;
//
//	load(root);
//}
//
//void Material::save()
//{
//	assert(!Project::isPathPointingInsideProjectFolder(m_completePath)); //path should be relative
//	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_completePath);
//
//	std::ofstream stream;
//	stream.open(absolutePath.toString());
//	if (!stream.is_open())
//	{
//		std::cout << "error, can't load shader program at path : " << absolutePath.toString() << std::endl;
//		return;
//	}
//	Json::Value root;
//	save(root);
//	stream << root;
//}
//
//void Material::resolvePointersLoading()
//{
//	std::ifstream stream;
//	stream.open(m_completePath.toString());
//	if (!stream.is_open())
//	{
//		std::cout << "error, can't load shader program at path : " << m_completePath.toString() << std::endl;
//		return;
//	}
//	Json::Value root;
//	stream >> root;
//
//	Json::Value materialRefs = root["materialRefs"];
//	for (int i = 0; i < materialRefs.size(); i++)
//	{
//		ResourcePtr<MaterialInstance> ref;
//		ref.load(materialRefs[i]);
//		m_materialRefs.push_back(ref);
//	}
//	
//}
//
//
//void Material::load(const Json::Value& root)
//{
//	// Pipeline handling type
//	//m_pipelineHandlingTypes.clear();
//	//Json::Value pipelineHandlingTypeJson = root["pipelineHandling"];
//	//if (pipelineHandlingTypeJson.isArray())
//	//{
//	//	for (int i = 0; i < pipelineHandlingTypeJson.size(); i++)
//	//	{
//	//		auto foundItPipelineType = std::find(Rendering::PipelineTypesToString.begin(), Rendering::PipelineTypesToString.end(), pipelineHandlingTypeJson[i].asString());
//	//		assert(foundItPipelineType != Rendering::PipelineTypesToString.end());
//	//		int foundIdxPipelineType = foundItPipelineType - Rendering::PipelineTypesToString.begin();
//	//		m_pipelineHandlingTypes.push_back((Rendering::PipelineType)foundIdxPipelineType);
//	//	}
//	//}
//	//else
//	//{
//	//	auto foundItPipelineType = std::find(Rendering::PipelineTypesToString.begin(), Rendering::PipelineTypesToString.end(), root.get("pipelineHandling", "").asString()/*pipelineHandlingTypeJson.asString()*/);
//	//	assert(foundItPipelineType != Rendering::PipelineTypesToString.end());
//	//	int foundIdxPipelineType = foundItPipelineType - Rendering::PipelineTypesToString.begin();
//	//	m_pipelineHandlingTypes.push_back((Rendering::PipelineType)foundIdxPipelineType);
//	//}
//
//	// Base MaterialInstance
//	//auto foundItBaseMaterial = std::find(Rendering::BaseMaterialTypeToString.begin(), Rendering::BaseMaterialTypeToString.end(), root.get("baseMaterial", "").asString());
//	//assert(foundItBaseMaterial != Rendering::BaseMaterialTypeToString.end());
//	//int foundIdxBaseMaterial = foundItBaseMaterial - Rendering::BaseMaterialTypeToString.begin();
//	//m_baseMaterialType = (Rendering::BaseMaterialType)foundIdxBaseMaterial;
//
//	// MaterialInstance Type
//	auto foundItMaterialType = std::find(Rendering::MaterialTypeToString.begin(), Rendering::MaterialTypeToString.end(), root.get("materialType", "internal").asString());
//	assert(foundItMaterialType != Rendering::MaterialTypeToString.end());
//	int foundIdxMaterialType = foundItMaterialType - Rendering::MaterialTypeToString.begin();
//	m_materialType = (Rendering::MaterialType)foundIdxMaterialType;
//
//	m_usage = Utils::stringToEnum<Rendering::MaterialUsage>(root.get("materialUsage", "custom").asString(), Rendering::MaterialUsageToString);
//	m_pipeline = Utils::stringToEnum<Rendering::PipelineType>(root.get("materialPipeline", "custom").asString(), Rendering::PipelineTypesToString);
//	//m_usedWithReflections = root.get("usedWithReflection", false).asBool();
//	m_usedWithSkeleton = root.get("usedWithSkeleton", false).asBool();
//
//	//if (std::find(m_pipelineHandlingTypes.begin(), m_pipelineHandlingTypes.end(), Rendering::PipelineType::DEFERRED_PIPILINE) != m_pipelineHandlingTypes.end()
//	//	&& std::find(m_pipelineHandlingTypes.begin(), m_pipelineHandlingTypes.end(), Rendering::PipelineType::FORWARD_PIPELINE) != m_pipelineHandlingTypes.end())
//	//{
//	//	Json::Value deferredJson = root.get("deferred", "");
//	//	assert(!deferredJson.empty());
//	//	Json::Value forwardJson = root.get("forward", "");
//	//	assert(!forwardJson.empty());
//
//	//	// Deferred : 
//	//	std::string vertexShaderName = deferredJson.get("vertex", "").asString();
//	//	std::string fragmentShaderName = deferredJson.get("fragment", "").asString();
//	//	std::string geometryShaderName = deferredJson.get("geometry", "").asString();
//
//	//	load(Rendering::PipelineType::DEFERRED_PIPILINE, path, vertexShaderName, fragmentShaderName, geometryShaderName);
//
//	//	// Forward : 
//	//	vertexShaderName = forwardJson.get("vertex", "").asString();
//	//	fragmentShaderName = forwardJson.get("fragment", "").asString();
//	//	geometryShaderName = forwardJson.get("geometry", "").asString();
//
//	//	load(Rendering::PipelineType::FORWARD_PIPELINE, path, vertexShaderName, fragmentShaderName, geometryShaderName);
//	//}
//
//	if (m_materialType == Rendering::MaterialType::DEFAULT)
//	{
//		m_computeShaderParameterFunction = root.get("computeShaderParameterFunction", "").asString();
//
//		m_programId = makeGLProgram();
//	}
//	else if (m_materialType == Rendering::MaterialType::INTERNAL)
//	{
//		const std::string vertexShaderName = root.get("vertex", "").asString();
//		const std::string fragmentShaderName = root.get("fragment", "").asString();
//		const std::string geometryShaderName = root.get("geometry", "").asString();
//
//		assert(!Project::isPathPointingInsideProjectFolder(m_completePath));
//		FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_completePath);
//
//		m_programId = makeGLProgramForInternal(absolutePath, vertexShaderName, fragmentShaderName, geometryShaderName);
//	}
//	else
//		assert(false && "Wrong material type !");
//
//	makeMaterialAggregates();
//
//	// Internal parameters
//	Json::Value internalShaderParameters = root["internalShaderParameters"];
//	int inernalShaderParameterCount = internalShaderParameters.size();
//	
//	m_internalShaderParameters.clear();
//	for (int i = 0; i < inernalShaderParameterCount; i++)
//	{
//		assert(internalShaderParameters.isValidIndex(i));
//
//		auto newParameter = MakeNewInternalShaderParameter(internalShaderParameters[i]);
//		//newParameter->init(id); //don't forget to init the parameter to get the uniforms
//		m_internalShaderParameters.push_back(newParameter);
//	}
//
//	if (m_materialType != Rendering::MaterialType::INTERNAL)
//		m_nodeManager = std::make_shared<MVS::NodeManager>(this);
//
//	if(m_nodeManager)
//		m_nodeManager->load(root["MVS"]);
//
//	// External parameters
//	/*Json::Value externalShaderParameters = root["externalShaderParameters"];
//	int externalShaderParameterCount = externalShaderParameters.size();
//
//	m_externalShaderParameters.clear();
//	for (int i = 0; i < externalShaderParameterCount; i++)
//	{
//		assert(externalShaderParameters.isValidIndex(i));
//
//		std::string parameterType = externalShaderParameters[i].get("type", "").asString();
//		std::string parameterName = externalShaderParameters[i].get("name", "").asString();
//
//		m_externalShaderParameters.push_back(MakeNewExternalShaderParameter(parameterType, parameterName));
//	}*/
//
//	PRINT_ERROR("error in shader program initialization.");
//}
//
//void Material::save(Json::Value& root) const
//{
//	// MaterialInstance Type
//	root["materialType"] = Utils::enumToString(m_materialType, Rendering::MaterialTypeToString);
//	root["materialUsage"] = Utils::enumToString(m_usage, Rendering::MaterialUsageToString);
//	root["materialPipeline"] = Utils::enumToString(m_pipeline, Rendering::PipelineTypesToString);
//	//root["usedWithReflection"] = m_usedWithReflections;
//	root["usedWithSkeleton"] = m_usedWithSkeleton;
//	// Compilation result
//	root["computeShaderParameterFunction"] = m_computeShaderParameterFunction;
//
//	if (m_materialType == Rendering::MaterialType::DEFAULT)
//	{
//		// TODO : save visual scripting infos
//	}
//	/*else if (m_materialType == Rendering::MaterialType::INTERNAL)
//	{
//		root["vertex"] = m_vertexShaderName;
//		root.get["fragment"] = m_fragmentShaderName;
//		root.get["geometry"] = m_geometryShaderName;
//	}*/
//	else
//		assert(false && "Wrong material type !");
//
//	assert(m_nodeManager);
//	m_nodeManager->save(root["MVS"]);
//
//	for (int i = 0; i < m_materialRefs.size(); i++)
//	{
//		m_materialRefs[i]->save(root["materialRefs"][i]);
//	}
//
//	// Internal parameters
//	//root["internalShaderParameters"] = Json::Value(Json::arrayValue);
//	//for (int i = 0; i < m_internalShaderParameters.size(); i++)
//	//{
//	//	root["internalShaderParameters"][i] = Json::Value(Json::objectValue);
//	//	SaveInternalShaderParameter(internalShaderParameters[i], root["internalShaderParameters"][i]);
//	//	//newParameter->init(id); //don't forget to init the parameter to get the uniforms
//	//}
//}
//
//void Material::load(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath)
//{
//	bool hasVertShader = !vertexShaderPath.empty();
//	bool hasFragShader = !fragmentShaderPath.empty();
//
//	GLuint vertShaderId = 0;
//	GLuint fragShaderId = 0;
//
//	if (hasVertShader)
//		vertShaderId = compile_shader_from_file(GL_VERTEX_SHADER, vertexShaderPath.c_str());
//	if (hasFragShader)
//		fragShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, fragmentShaderPath.c_str());
//
//	GLuint programObject = glCreateProgram();
//	if (hasVertShader)
//		glAttachShader(programObject, vertShaderId);
//	if (hasFragShader)
//		glAttachShader(programObject, fragShaderId);
//
//	glLinkProgram(programObject);
//	if (check_link_error(programObject) < 0)
//		exit(1);
//
//	//check uniform errors : 
//	if (!checkError("Uniforms"))
//		exit(1);
//
//	id = programObject;
//}
//


//
//void Material::drawInInspector(Scene & scene)
//{
//	assert(m_nodeManager);
//
//	drawUIMaterialUsage();
//	drawUIPipelineType();
//	//drawUIUsedWithReflections();
//	drawUIUsedWithSkeleton();
//
//	if (ImGui::Button("Edit"))
//	{
//		// If the edit window isn't opened
//		//if (m_nodeManagerRef.expired() || m_nodeManagerRef.use_count() == 0)
//		//{
//		//	// Node manager creation and loading
//		//	assert(!Project::isPathPointingInsideProjectFolder(m_completePath)); //path should be relative
//		//	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_completePath);
//		//	std::ifstream stream;
//		//	stream.open(absolutePath.toString());
//		//	if (!stream.is_open())
//		//	{
//		//		std::cout << "error, can't load shader program at path : " << m_completePath.toString() << std::endl;
//		//		return;
//		//	}
//		//	Json::Value root;
//		//	stream >> root;
//
//		//	auto newNodeManager = std::make_shared<MVS::NodeManager>(this);
//		//	m_nodeManagerRef = newNodeManager;
//		//	m_nodeManagerRef.lock()->load(root["MVS"]);
//
//		//	// We open the window which will display the node manager
//		//	Editor::instance().getWindowManager()->addWindowAsynchrone(newNodeManager);
//		//}
//
//		if (m_nodeManagerEditorFrameRef.expired() || m_nodeManagerEditorFrameRef.use_count() == 0)
//		{
//			Editor::instance().getWindowManager()->addWindowAsynchrone(std::make_shared<MVSEditorFrame>(m_nodeManager.get()));
//		}
//	}
//
//	if (m_needRecompilation)
//	{
//		if (ImGui::Button("Compile"))
//		{
//			compile();
//		}
//	}
//}

//
//MaterialInstance* Material::makeNewMaterialInstance(const FileHandler::CompletePath& completePath)
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return new Material3DObject(*this, completePath);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return new MaterialBillboard(*this, completePath);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return new MaterialParticlesCPU(*this, completePath);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return new MaterialReflection(*this, completePath);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}
//
//std::shared_ptr<MaterialInstance> Material::makeSharedMaterialInstance(const FileHandler::CompletePath& completePath)
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return std::make_shared<Material3DObject>(*this, completePath);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return std::make_shared<MaterialBillboard>(*this, completePath);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return std::make_shared<MaterialParticlesCPU>(*this, completePath);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return std::make_shared<MaterialReflection>(*this, completePath);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}
//
//MaterialInstance* Material::makeNewMaterialInstance()
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return new Material3DObject(*this);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return new MaterialBillboard(*this);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return new MaterialParticlesCPU(*this);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return new MaterialReflection(*this);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}
//
//std::shared_ptr<MaterialInstance> Material::makeSharedMaterialInstance()
//{
//	switch (m_baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//		return std::make_shared<Material3DObject>(*this);
//	case Rendering::BaseMaterialType::BILLBOARD:
//		return std::make_shared<MaterialBillboard>(*this);
//	case Rendering::BaseMaterialType::PARTICLE:
//		return std::make_shared<MaterialParticlesCPU>(*this);
//	case Rendering::BaseMaterialType::REFLECTION:
//		return std::make_shared<MaterialReflection>(*this);
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}

//MaterialInstance* makeNewMaterialInstance(const FileHandler::CompletePath& path)
//{
//	assert(!Project::isPathPointingInsideProjectFolder(path)); //path should be relative
//	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(path);
//
//	std::ifstream stream;
//	stream.open(absolutePath.toString());
//	if (!stream.is_open())
//	{
//		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
//		return nullptr;
//	}
//	Json::Value root;
//	stream >> root;
//
//	ResourcePtr<Material> programPtr;
//	programPtr.load(root["shaderProgramName"]);
//	assert(programPtr.isValid());
//	Rendering::BaseMaterialType baseMaterialType = programPtr->getBaseMaterialType();
//
//	//std::string shaderProgramName = root.get("shaderProgramName", "").asString();
//	//assert(shaderProgramName != "");
//	//Material* program = getProgramFactory().get(shaderProgramName);
//	//assert(program != nullptr);
//	//Rendering::BaseMaterialType baseMaterialType = program->getBaseMaterialType();
//
//	switch (baseMaterialType)
//	{
//	case Rendering::BaseMaterialType::OBJECT_3D:
//	{
//		MaterialInstance* newMaterial = new Material3DObject();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	case Rendering::BaseMaterialType::BILLBOARD:
//	{
//		MaterialInstance* newMaterial = new MaterialBillboard();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	case Rendering::BaseMaterialType::PARTICLE:
//	{
//		MaterialInstance* newMaterial = new MaterialParticlesCPU();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	case Rendering::BaseMaterialType::REFLECTION:
//	{
//		MaterialInstance* newMaterial = new MaterialReflection();
//		newMaterial->load(root);
//		return newMaterial;
//	}
//	default:
//		std::cout << "warning : we are trying to build a custom material from its program !";
//		return nullptr;
//	}
//}