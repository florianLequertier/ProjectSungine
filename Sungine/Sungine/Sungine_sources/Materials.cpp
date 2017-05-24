

#include "Materials.h"
#include "Factories.h"//forward
#include "EditorGUI.h"
#include "RenderDatas.h"

////////////////////////////////////////////////////
/////BEGIN : Material

Material::Material()
	: m_glProgramId(0)
	//, m_pipelineType(Rendering::PipelineType::CUSTOM_PIPELINE)
{

}
//
//Material::Material(const FileHandler::CompletePath& glProgramPath, GLuint glProgramId, PipelineTypes pipelineType, std::vector<std::shared_ptr<InternalShaderParameterBase>>& internalParameters)
//	: Resource(glProgramPath)
//	, m_glProgramId(glProgramId)
//	, m_glProgramName(glProgramPath.getFilename())
//	, m_pipelineType(pipelineType)
//{
//	for (auto shaderParameter : internalParameters)
//	{
//		m_internalParameters.push_back(shaderParameter->clone());
//	}
//
//	initInternalParameters();
//	getProgramFactory().get(m_glProgramName)->addMaterialRef(this);
//}
//
//Material::Material(const FileHandler::CompletePath& glProgramPath, GLuint glProgramId, PipelineTypes pipelineType, std::vector<std::shared_ptr<InternalShaderParameterBase>>& internalParameters, std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters)
//	: Material(glProgramPath, glProgramId, pipelineType, internalParameters)
//{
//
//}

Material::Material(const ShaderProgram& shaderProgram)
	//: m_glProgramName(shaderProgram.getName())
{
	m_programPtr = &shaderProgram;
	assert(m_programPtr.isValid());

	//assert(shaderProgram.getImplementedPipelineCount() > 0);
	//m_pipelineType = shaderProgram.getImplementedPipeline(0);

	m_glProgramId = shaderProgram.getProgramId(/*m_pipelineType*/);

	for (auto shaderParameter : shaderProgram.getInternalParameters())
	{
		m_internalParameters.push_back(shaderParameter->cloneShared());
	}

	m_aggregations.clear();
	m_aggregations = shaderProgram.getAggregations();
	m_perInstanceAggregations.clear();
	m_perInstanceAggregations = shaderProgram.getPerInstanceAggregation();

	initInternalParameters();
	m_programPtr->addMaterialRef(this);
	//getProgramFactory().get(m_glProgramName)->addMaterialRef(this);
	//setExternalParameters(shaderProgram.getExternalParameters());
}

Material::Material(const ShaderProgram & shaderProgram, const FileHandler::CompletePath & completePath)
	: Resource(completePath)
	//, m_glProgramName(shaderProgram.getName())
{
	m_programPtr = &shaderProgram;
	assert(m_programPtr.isValid());

	//assert(shaderProgram.getImplementedPipelineCount() > 0);
	//m_pipelineType = shaderProgram.getImplementedPipeline(0);

	m_glProgramId = shaderProgram.getProgramId(/*m_pipelineType*/);

	for (auto shaderParameter : shaderProgram.getInternalParameters())
	{
		m_internalParameters.push_back(shaderParameter->cloneShared());
	}

	m_aggregations.clear();
	m_aggregations = shaderProgram.getAggregations();
	m_perInstanceAggregations.clear();
	m_perInstanceAggregations = shaderProgram.getPerInstanceAggregation();

	initInternalParameters();
	m_programPtr->addMaterialRef(this);
	//getProgramFactory().get(m_glProgramName)->addMaterialRef(this);
}

Material::~Material()
{
	if (m_programPtr.isValid())
	{
		m_programPtr->removeMaterialRef(this);
	}
	//if (m_glProgramName != "")
	//{
	//	auto shaderProgramRef = getProgramFactory().get(m_glProgramName);
	//	if(shaderProgramRef != nullptr)
	//		shaderProgramRef->removeMaterialRef(this);
	//}
}

//void Material::changePipeline(Rendering::PipelineType pipelineType)
//{
//	ShaderProgram* currentProgram = m_programPtr.get(); //getProgramFactory().get(m_glProgramName);
//	assert(currentProgram != nullptr);
//	if (currentProgram->implementPipeline(pipelineType))
//	{
//		m_glProgramId = currentProgram->getProgramId(pipelineType);
//
//		// Refresh parameters GLIds : 
//		// Internals :
//		for (auto& parameter : m_internalParameters)
//		{
//			parameter->init(m_glProgramId);
//		}
//		// Externals :
//		setExternalParameters();
//
//		m_pipelineType = pipelineType;
//
//		// Special case for forward pipeline
//		if (pipelineType == Rendering::PipelineType::FORWARD_PIPELINE)
//		{
//			auto newAggregate = std::make_shared<MaterialAggregationForward>();
//			newAggregate->initParameters(m_glProgramId);
//			addAggregation("forward", newAggregate);
//		}
//		else
//		{
//			removeAggregation("forward");
//		}
//	}
//}

//void Material::addAggregation(const std::string & key, std::shared_ptr<MaterialAggregation> aggregation)
//{
//	m_aggregations[key] = aggregation;
//}
//
//void Material::removeAggregation(const std::string & key)
//{
//	m_aggregations.erase(key);
//}
//
//const std::shared_ptr<MaterialAggregation> Material::getAggregation(const std::string& key) const
//{
//	return m_aggregations.at(key);
//}

void Material::initInternalParameters()
{
	for (auto& internalParam : m_internalParameters)
	{
		internalParam->init(m_glProgramId);
	}
}

void Material::init(const FileHandler::CompletePath& path, const ID& id)
{
	Resource::init(path, id);

	assert(!Project::isPathPointingInsideProjectFolder(path)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(path);

	std::ifstream stream;
	stream.open(absolutePath.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;
	stream >> root;

	load(root);
}

void Material::save()
{
	assert(!Project::isPathPointingInsideProjectFolder(m_completePath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_completePath);

	std::ofstream stream;
	stream.open(absolutePath.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;

	save(root);

	stream << root;
}


void Material::save(Json::Value & entityRoot) const
{
	// Pipeline type : 
	//entityRoot["pipelineType"] = Rendering::PipelineTypesToString[(int)m_pipelineType];

	//// Program name :
	//entityRoot["shaderProgramName"] = m_glProgramName;
	//assert(getProgramFactory().contains(m_glProgramName));
	m_programPtr.save(entityRoot["shaderProgramPtr"]);

	// Internal parameters :
	entityRoot["internalParametersCount"] = m_internalParameters.size();
	int parameterIdx = 0;
	for (auto& parameter : m_internalParameters)
	{
		parameter->save(entityRoot["internalParameters"][parameterIdx]);
		parameterIdx++;
	}
}

void Material::load(const Json::Value & entityRoot)
{
	// Pipeline type : 
	//auto foundItPipelineType = std::find(Rendering::PipelineTypesToString.begin(), Rendering::PipelineTypesToString.end(), entityRoot.get("pipelineType", "").asString());
	//assert(foundItPipelineType != Rendering::PipelineTypesToString.end());
	//int foundIdxPipelineType = foundItPipelineType - Rendering::PipelineTypesToString.begin();
	//m_pipelineType = (Rendering::PipelineType)foundIdxPipelineType;

	//m_programPtr.load(entityRoot["shaderProgramPtr"]);
	//assert(m_programPtr.isValid());
	//m_programPtr->LoadMaterialInstance(this);

	//// Program name : 
	//m_glProgramName = entityRoot.get("shaderProgramName", "").asString();
	//assert(m_glProgramName != "");

	//// Init from shader program : 
	//m_programPtr = getProgramFactory().get(m_glProgramName);
	//assert(m_programPtr != nullptr);
	//m_programPtr->LoadMaterialInstance(this);

	// Internal parameters : 
	int internalParameterSavedCount = entityRoot.get("internalParametersCount", 0).asInt();
	// We only try to load internals if there are no mismatch in parameter count.
	if (internalParameterSavedCount == m_internalParameters.size())
	{
		int parameterIdx = 0;
		for (auto& parameter : m_internalParameters)
		{
			parameter->load(entityRoot["internalParameters"][parameterIdx]);
			parameterIdx++;
		}
		initInternalParameters();
	}
	else
		PRINT_ERROR("error in material loading : internal parameter count mismatch. You may have added or remove parameters from a .glProg.");

	// Add ref to shader program :
	//m_programPtr->addMaterialRef(this);
	//getProgramFactory().get(m_glProgramName)->addMaterialRef(this);
}

//
//void Material::save(const FileHandler::CompletePath& path) const
//{
//	//We check that we have the absolute project path
//	assert(Project::isPathPointingInsideProjectFolder(path));
//
//	std::ofstream stream;
//	stream.open(path.toString());
//	if (!stream.is_open())
//	{
//		std::cout << "error, can't save material at path : " << path.toString() << std::endl;
//		return;
//	}
//	Json::Value root;
//	save(root);
//
//	stream << root;
//}

void Material::resolvePointersLoading()
{
	assert(!Project::isPathPointingInsideProjectFolder(m_completePath)); //path should be relative
	FileHandler::CompletePath absolutePath = Project::getAbsolutePathFromRelativePath(m_completePath);

	std::ifstream stream;
	stream.open(absolutePath.toString());
	if (!stream.is_open())
	{
		std::cout << "error, can't load material at path : " << absolutePath.toString() << std::endl;
		return;
	}
	Json::Value root;
	stream >> root;

	/////////////////

	m_programPtr.load(root["shaderProgramPtr"]);
	assert(m_programPtr.isValid());
	if (m_programPtr.isValid())
	{
		m_programPtr->LoadMaterialInstance(this);
		m_programPtr->addMaterialRef(this);
	}
}

void Material::init(const ShaderProgram & shaderProgram)
{
	m_programPtr = &shaderProgram;
	assert(m_programPtr.isValid());

	//assert(shaderProgram.getImplementedPipelineCount() > 0);
	//m_pipelineType = shaderProgram.getImplementedPipeline(0);

	// Set shader program id
	m_glProgramId = shaderProgram.getProgramId(/*m_pipelineType*/);

	// Init externals
	setExternalParameters();

	// Setup internal parameters
	m_internalParameters.clear();
	for (auto shaderParameter : shaderProgram.getInternalParameters())
	{
		m_internalParameters.push_back(shaderParameter->cloneShared());
	}
	initInternalParameters();
	//m_glProgramName = shaderProgram.getName();

	// Setup aggregations
	m_aggregations.clear();
	m_aggregations = shaderProgram.getAggregations();
	m_perInstanceAggregations.clear();
	m_perInstanceAggregations = shaderProgram.getPerInstanceAggregation();

	// Add pointer reference
	m_programPtr->addMaterialRef(this);
	//getProgramFactory().get(m_glProgramName)->addMaterialRef(this);
	//setExternalParameters(shaderProgram.getExternalParameters());
}

void Material::setAggregates(std::unordered_map<std::string, std::shared_ptr<MaterialAggregation>>& aggregations)
{
	m_aggregations.clear();
	m_aggregations = aggregations;
}

void Material::setPerInstanceAggregates(std::unordered_map<std::string, std::shared_ptr<PerInstanceMaterialAggregation>>& perInstanceAggregations)
{
	m_perInstanceAggregations.clear();
	m_perInstanceAggregations = perInstanceAggregations;
}

void Material::setProgramId(GLuint programId)
{
	m_glProgramId = programId;
}

void Material::setinternalParameters(std::vector<std::shared_ptr<InternalShaderParameterBase>>& internalShaderParameters)
{
	m_internalParameters.clear();
	m_internalParameters = internalShaderParameters;
}

void Material::drawUI()
{
	//if (m_programPtr->implementPipeline(Rendering::PipelineType::FORWARD_PIPELINE) && m_programPtr->implementPipeline(Rendering::PipelineType::DEFERRED_PIPILINE))
	//{
	//	int currentPipelineIdx = (m_pipelineType == Rendering::PipelineType::FORWARD_PIPELINE ? 0 : 1);
	//	if (ImGui::Combo("pipeline", &currentPipelineIdx, "forward\0deferred\0"))
	//	{
	//		Rendering::PipelineType newPipelineType = currentPipelineIdx == 0 ? Rendering::PipelineType::FORWARD_PIPELINE : Rendering::PipelineType::DEFERRED_PIPILINE;
	//		changePipeline(newPipelineType);
	//	}
	//}

	for (auto& parameter : m_internalParameters)
	{
		parameter->drawUI();
	}
}

void Material::pushGlobalsToGPU(const RenderDatas & renderDatas) const
{
	for (auto aggregation : m_aggregations)
	{
		aggregation.second->pushParametersToGPU(renderDatas);
	}
}

void Material::pushInternalsToGPU(int& boundTextureCount) const 
{
	for (auto& parameter : m_internalParameters)
	{
		parameter->pushToGPU(boundTextureCount);
	}
}

void Material::pushExternalsToGPU(const IDrawable & drawable, const RenderDatas & renderDatas, int & boundTextureCount) const
{
	for (auto perInstanceAggregation : m_perInstanceAggregations)
	{
		perInstanceAggregation.second->pushParametersToGPU(drawable, renderDatas, boundTextureCount);
	}
}

//void Material::setExternalParameters(const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters)
//{
//	for (int i = 0; i < externalParameters.size(); i++)
//	{
//		m_externalParameters[externalParameters[i]->getName()] = externalParameters[i];
//	}
//}

void Material::use() const
{
	glUseProgram(m_glProgramId);
}

void Material::use()
{
	glUseProgram(m_glProgramId);
}

GLuint Material::getGLId() const
{
	return m_glProgramId;
}

Rendering::PipelineType Material::getPipelineType() const
{
	assert(m_programPtr.get() != nullptr);
	return m_programPtr->getPipelineType();
	//return m_pipelineType;
}

void Material::loadFromShaderProgramDatas(GLuint glProgramId, const std::vector<std::shared_ptr<InternalShaderParameterBase>>& internalParameters/*, std::vector<std::shared_ptr<ExternalShaderParameterBase>>& externalParameters*/)
{
	//m_pipelineType = pipelineType;

	m_internalParameters.clear();
	//m_externalParameters.clear();

	m_glProgramId = glProgramId;
	for (auto shaderParameter : internalParameters)
	{
		m_internalParameters.push_back(shaderParameter->cloneShared());
	}

	//setExternalParameters(externalParameters);
}

void Material::drawInInspector(Scene & scene)
{
	Resource::drawInInspector(scene);

	drawUI();
}

/////END : Material
////////////////////////////////////////////////////



/////////////////////////////////////////////////////////
//// BEGIN : MaterialAggregationForward


void MaterialAggregationForward::initParameters(GLuint programID)
{

	for (int i = 0; i < MAX_POINT_LIGHT_COUNT; ++i)
	{
		uniform_PointLight_position[i] = MaterialHelper::getUniformStruct(programID, "PointLight", i, "position");
		uniform_PointLight_color[i] = MaterialHelper::getUniformStruct(programID, "PointLight", i, "color");
		uniform_PointLight_intensity[i] = MaterialHelper::getUniformStruct(programID, "PointLight", i, "intensity");
	}

	for (int i = 0; i < MAX_SPOT_LIGHT_COUNT; ++i)
	{
		uniform_SpotLight_position[i] = MaterialHelper::getUniformStruct(programID, "SpotLight", i, "position");
		uniform_SpotLight_direction[i] = MaterialHelper::getUniformStruct(programID, "SpotLight", i, "direction");
		uniform_SpotLight_color[i] = MaterialHelper::getUniformStruct(programID, "SpotLight", i, "color");
		uniform_SpotLight_intensity[i] = MaterialHelper::getUniformStruct(programID, "SpotLight", i, "intensity");
		uniform_SpotLight_angle[i] = MaterialHelper::getUniformStruct(programID, "SpotLight", i, "angle");
	}

	for (int i = 0; i < MAX_DIRECTIONAL_LIGHT_COUNT; ++i)
	{
		uniform_DirectionalLight_direction[i] = MaterialHelper::getUniformStruct(programID, "DirectionalLight", i, "direction");
		uniform_DirectionalLight_color[i] = MaterialHelper::getUniformStruct(programID, "DirectionalLight", i, "color");
		uniform_DirectionalLight_intensity[i] = MaterialHelper::getUniformStruct(programID, "DirectionalLight", i, "intensity");
	}

	uniform_PointLightCount = MaterialHelper::getUniform(programID, "PointLightCount");
	uniform_SpotLightCount = MaterialHelper::getUniform(programID, "SpotLightCount");
	uniform_DirectionalLightCount = MaterialHelper::getUniform(programID, "DirectionalLightCount");

	uniform_ScreenToView = MaterialHelper::getUniform(programID, "ScreenToView");

	CHECK_GL_ERROR("error in material initialization.");
}

void MaterialAggregationForward::pushParametersToGPU(const RenderDatas& renderDatas) const
{
	// Push light infos to GPU : 
	setUniformPointLightCount(renderDatas.pointLightRenderDatas.size());
	setUniformPointLightCount(renderDatas.spotLightRenderDatas.size());
	setUniformPointLightCount(renderDatas.directionalLightRenderDatas.size());

	for (int i = 0; i < renderDatas.pointLightRenderDatas.size(); ++i)
	{
		PointLight* currentLight = renderDatas.pointLightRenderDatas[i].light;
		setUniformPointLight(i, currentLight->position, currentLight->color, currentLight->intensity);
	}
	for (int i = 0; i < renderDatas.spotLightRenderDatas.size(); ++i)
	{
		SpotLight* currentLight = renderDatas.spotLightRenderDatas[i].light;
		setUniformSpotLight(i, currentLight->position, currentLight->direction, currentLight->angle, currentLight->color, currentLight->intensity);
	}
	for (int i = 0; i < renderDatas.directionalLightRenderDatas.size(); ++i)
	{
		DirectionalLight* currentLight = renderDatas.directionalLightRenderDatas[i].light;
		setUniformDirectionalLight(i, currentLight->direction, currentLight->color, currentLight->intensity);
	}

	setUniformScreenToView(renderDatas.screenToView);
}

void MaterialAggregationForward::setUniformPointLight(int index, const glm::vec3& position, const glm::vec3& color, float intensity) const
{
	assert(index < MAX_POINT_LIGHT_COUNT);
	GlHelper::pushParameterToGPU(uniform_PointLight_position[index], position);
	GlHelper::pushParameterToGPU(uniform_PointLight_color[index], color);
	GlHelper::pushParameterToGPU(uniform_PointLight_intensity[index], intensity);
}
void MaterialAggregationForward::setUniformSpotLight(int index, const glm::vec3& position, const glm::vec3& direction, float angle, const glm::vec3& color, float intensity) const
{
	assert(index < MAX_SPOT_LIGHT_COUNT);

	GlHelper::pushParameterToGPU(uniform_SpotLight_position[index], position);
	GlHelper::pushParameterToGPU(uniform_SpotLight_direction[index], direction);
	GlHelper::pushParameterToGPU(uniform_SpotLight_angle[index], angle);
	GlHelper::pushParameterToGPU(uniform_SpotLight_color[index], color);
	GlHelper::pushParameterToGPU(uniform_SpotLight_intensity[index], intensity);
}
void MaterialAggregationForward::setUniformDirectionalLight(int index, const glm::vec3& direction, const glm::vec3& color, float intensity) const
{
	assert(index < MAX_DIRECTIONAL_LIGHT_COUNT);

	GlHelper::pushParameterToGPU(uniform_DirectionalLight_direction[index], direction);
	GlHelper::pushParameterToGPU(uniform_DirectionalLight_color[index], color);
	GlHelper::pushParameterToGPU(uniform_DirectionalLight_intensity[index], intensity);
}

void MaterialAggregationForward::setUniformPointLightCount(int count) const
{
	GlHelper::pushParameterToGPU(uniform_PointLightCount, count);
}
void MaterialAggregationForward::setUniformSpotLightCount(int count) const
{
	GlHelper::pushParameterToGPU(uniform_SpotLightCount, count);
}
void MaterialAggregationForward::setUniformDirectionalLightCount(int count) const
{
	GlHelper::pushParameterToGPU(uniform_DirectionalLightCount, count);
}

void MaterialAggregationForward::setUniformScreenToView(const glm::mat4& screenToView) const
{
	GlHelper::pushParameterToGPU(uniform_ScreenToView, screenToView);
}

//// END : MaterialAggregationForward
/////////////////////////////////////////////////////////