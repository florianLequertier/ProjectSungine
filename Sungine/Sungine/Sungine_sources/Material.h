#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <istream>
#include <glew/glew.h>
#include <glm/glm.hpp>
#include "FileHandler.h"
#include "MaterialAggregation.h"
#include "ICloneable.h"

#include "Asset.h"
#include "AssetManager.h"

//forwards :
class InternalShaderParameterBase;
class ExternalShaderParameterBase;
class MaterialInstance;
class ResourceTreeView;
class EditorFrame;

namespace MVS {
	class NodeManager;
}

namespace Rendering
{
	// WARNING : custom enum value must always be at the end of [...]ToString arrays !

	enum class MaterialType
	{
		INTERNAL,
		DEFAULT,
		COUNT
	};

	static std::vector<std::string> MaterialTypeToString = {
		"internal",
		"default",
	};

	enum class MaterialUsage
	{
		MESH,
		BILLBOARD,
		PARTICLES,
		REFLECTIVE_PLANE,
		CUSTOM,
		COUNT
	};

	static std::vector<std::string> MaterialUsageToString = {
		"mesh",
		"billboard",
		"particles",
		"reflective plane",
		"custom",
	};

	enum class BaseMaterialType
	{
		OBJECT_3D = 0,
		BILLBOARD,
		PARTICLE,
		REFLECTION,
		CUSTOM,
		COUNT
	};

	static std::vector<std::string> BaseMaterialTypeToString = {
		"object3D",
		"billboard",
		"particle",
		"reflection",
		"custom",
	};

	enum class PipelineType
	{
		DEFERRED_PIPILINE = 0,
		FORWARD_PIPELINE,
		CUSTOM_PIPELINE,
		COUNT
	};

	static std::vector<std::string> PipelineTypesToString = {
		"deferred",
		"forward",
		"custom",
	};
}


struct Material : public Asset, public ICloneable<Material>
{
	CLONABLE_IMPL(Material, Material)

public:
	static std::string s_extention;

private:
	GLuint m_programId;

	// MaterialInstance parameters and aggregates.
	std::vector<std::shared_ptr<InternalShaderParameterBase>> m_internalShaderParameters;
	std::unordered_map<std::string, std::shared_ptr<MaterialAggregation>> m_aggregations;
	std::unordered_map<std::string, std::shared_ptr<PerInstanceMaterialAggregation>> m_perInstanceAggregations;
	//std::vector<std::shared_ptr<ExternalShaderParameterBase>> m_externalShaderParameters;

	// How the material is used.
	Rendering::MaterialType m_materialType;
	Rendering::MaterialUsage m_usage;
	Rendering::PipelineType m_pipeline;
	//bool m_usedWithReflections;
	bool m_usedWithSkeleton;
	/*std::vector<Rendering::PipelineType> m_pipelineHandlingTypes;
	Rendering::BaseMaterialType m_baseMaterialType;*/

	// Pointers to materials using this program.
	std::vector<AssetHandle<MaterialInstance>> m_materialRefs;

	std::string m_vertexShaderName;
	std::string m_fragmentShaderName;
	std::string m_geometryShaderName;

	// A pointer to the NodeManager associated with this material.
	std::shared_ptr<MVS::NodeManager> m_nodeManager;
	// A pointed to the window displaying the NodeManager
	std::weak_ptr<EditorFrame> m_nodeManagerEditorFrameRef;

	std::string m_computeShaderParameterFunction;
	bool m_needRecompilation;

public:

	//////////////////////////////////////////////////////////
	// Constructors and operators
	//////////////////////////////////////////////////////////
	Material();
	Material(const FileHandler::CompletePath& path, Rendering::MaterialType type);
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// Asset override
	//////////////////////////////////////////////////////////
	void createNewAssetFile(const FileHandler::CompletePath& filePath) override;
	void loadFromFile(const FileHandler::CompletePath& filePath) override;
	void saveToFile() override;
	void saveMetas() override;
	void loadMetas() override;
	void drawInInspector() override;
	bool drawRightClicContextMenu(std::string& popupToOpen) override;
	//////////////////////////////////////////////////////////


	const std::unordered_map<std::string, std::shared_ptr<MaterialAggregation>>& getAggregations() const;
	const std::unordered_map<std::string, std::shared_ptr<PerInstanceMaterialAggregation>>& getPerInstanceAggregation() const;
	GLuint getProgramId() const;
	void compile(); // DEPRECATED
	void compile(const std::string& computeShaderParameters);

	void resetNodeManagerPtr();

	void makeMaterialAggregates();
	GLuint makeGLProgramForInternal(const FileHandler::CompletePath& shaderFolderPath, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::string& geometryShaderName);
	//GLuint makeGLProgram(const FileHandler::CompletePath& shaderFolderPath, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::string& geometryShaderName);
	GLuint makeGLProgramForDefault(const FileHandler::CompletePath& shaderFolderPath, const std::string& fragmentShaderName);
	GLuint makeGLProgram();
	GLuint makeGLProgram(const std::string& vertexShaderContent, const std::string& fragmentShaderContent);


	//fill material datas from this shaderProgram
	void LoadMaterialInstance(MaterialInstance* material) const;

	void addMaterialRef(const AssetHandle<MaterialInstance>& ref);
	void removeMaterialRef(const AssetHandle<MaterialInstance>& ref);

	const std::vector<std::shared_ptr<InternalShaderParameterBase>>& getInternalParameters() const;
	void setInternalParameters(std::vector<std::shared_ptr<InternalShaderParameterBase>>& internals);
	//const std::vector<std::shared_ptr<ExternalShaderParameterBase>>& getExternalParameters() const;

	Rendering::MaterialType getMaterialType() const;
	Rendering::MaterialUsage getMaterialUsage() const;
	Rendering::PipelineType getPipelineType() const;
	//bool getUsedWithReflections() const;
	bool getUsedWithSkeletons() const;

	void setMaterialUsage(Rendering::MaterialUsage usage);
	void setPipelineType(Rendering::PipelineType pipeline);
	//void setUsedWithReflections(bool state);
	void setUsedWithSkeletons(bool state);

	//////////////////////////////////////////////////////////
	// Draw UI helpers
	//////////////////////////////////////////////////////////
	bool drawUIMaterialUsage();
	bool drawUIPipelineType();
	//bool drawUIUsedWithReflections();
	bool drawUIUsedWithSkeleton();
	//////////////////////////////////////////////////////////

	//Not copyable
	Material(const Material& other) = delete;
	const Material& operator=(const Material& other) = delete;




	//Material(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath);
	//Material(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath, const FileHandler::CompletePath& geometryShaderPath);

	//void load(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath);
	//void load(const FileHandler::CompletePath& vertexShaderPath, const FileHandler::CompletePath& fragmentShaderPath, const FileHandler::CompletePath& geometryShaderPath);

	//bool implementPipeline(Rendering::PipelineType pipelineType) const;
	//int getImplementedPipelineCount() const;
	//Rendering::PipelineType getImplementedPipeline(int idx) const;
	//GLuint getProgramId(Rendering::PipelineType pipelineType) const;


	//MaterialInstance* makeNewMaterialInstance(const FileHandler::CompletePath& completePath);
	//std::shared_ptr<MaterialInstance> makeSharedMaterialInstance(const FileHandler::CompletePath& completePath);
	////make a new material from this shaderProgram (warning : internaly use "new", you have to deal with deletion)
	//MaterialInstance* makeNewMaterialInstance();
	////make a new material from this shaderProgram (use shared ptr for automatic deletion)
	//std::shared_ptr<MaterialInstance> makeSharedMaterialInstance();

	//void init(const FileHandler::CompletePath& path, const ID& id) override;
	//void save() override;
	//void resolvePointersLoading() override;

	//void load(const Json::Value& root);
	//void save(Json::Value& root) const;

};

//MaterialInstance* makeNewMaterialInstance(const FileHandler::CompletePath& path);