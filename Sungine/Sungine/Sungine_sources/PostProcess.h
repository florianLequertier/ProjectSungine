#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "PostProcessMaterials.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Mesh.h"
#include "ISingleton.h"
#include "ICloner.h"

class DebugDrawRenderer;
struct PointLight;
struct RenderDatas;
class SSAOPostProcessOperation;

class PostProcessOperationData
{
protected:
	std::string m_operationName;

public:
	PostProcessOperationData(const std::string& operationName) : m_operationName(operationName) {}
	virtual const std::string& getOperationName() const { return m_operationName; }
	virtual void drawUI() = 0;
};

class PostProcessOperation
{
protected:
	std::string m_name;
public:
	PostProcessOperation(const std::string& name) : m_name(name) {}
	virtual void render(const PostProcessOperationData& operationData, const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& finalFB, Texture& finalTexture, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer) = 0;
	virtual void onViewportResized(float width, float height) = 0;
	virtual const std::string& getName() const { return m_name; }
	//virtual const Texture* getResult() const = 0;
};

// Store operations : 
class PostProcessFactory : public ISingleton<PostProcessFactory>
{
	SINGLETON_IMPL(PostProcessFactory)

private:
	std::unordered_map<std::string, std::unique_ptr<IClonerWithName<PostProcessOperation>>> m_operations;

public:
	PostProcessFactory()
	{}

	template<typename T>
	bool registerNew(const std::string& name)
	{
		m_operations[name] = std::make_unique<ClonerWithName<PostProcessOperation, T>>();
		return true;
	}

	std::shared_ptr<PostProcessOperation> getInstanceShared(const std::string& name)
	{
		assert(m_operations.find(name) != m_operations.end());
		return m_operations[name]->cloneShared(name);
	}

	std::unordered_map<std::string, std::unique_ptr<IClonerWithName<PostProcessOperation>>>::const_iterator begin() const
	{
		return m_operations.begin();
	}

	std::unordered_map<std::string, std::unique_ptr<IClonerWithName<PostProcessOperation>>>::const_iterator end() const
	{
		return m_operations.end();
	}
};


// Store operation datas : 
class PostProcessDataFactory : public ISingleton<PostProcessDataFactory>
{
	SINGLETON_IMPL(PostProcessDataFactory)

private:
	std::unordered_map<std::string, std::unique_ptr<IClonerWithName<PostProcessOperationData>>> m_operationDatas;

public:
	PostProcessDataFactory()
	{}

	template<typename T>
	bool registerNew(const std::string& name)
	{
		m_operationDatas[name] = std::make_unique<ClonerWithName<PostProcessOperationData, T>>();
		return true;
	}

	std::shared_ptr<PostProcessOperationData> getInstanceShared(const std::string& name)
	{
		assert(m_operationDatas.find(name) != m_operationDatas.end());
		return m_operationDatas[name]->cloneShared(name);
	}

	std::unordered_map<std::string, std::unique_ptr<IClonerWithName<PostProcessOperationData>>>::const_iterator begin() const
	{
		return m_operationDatas.begin();
	}

	std::unordered_map<std::string, std::unique_ptr<IClonerWithName<PostProcessOperationData>>>::const_iterator end() const
	{
		return m_operationDatas.end();
	}
};

// Register both operation and operation data into there factories : 
#define REGISTER_POST_PROCESS(OperationType, DataType, OperationName)\
	static bool isRegistered##OperationType = PostProcessFactory::instance().registerNew<OperationType>(OperationName);\
	static bool isRegistered##DataType = PostProcessDataFactory::instance().registerNew<DataType>(OperationName);


// Deals with operations
class PostProcessManager
{
private:
	GlHelper::Framebuffer m_finalFB;
	Texture m_finalTexture;
	std::shared_ptr<MaterialResizedBlit> m_materialBlit;

	std::shared_ptr<PostProcessOperation> m_ssaoOperation;
	std::vector<std::shared_ptr<PostProcessOperation>> m_operationList;

public:
	PostProcessManager();
	void onViewportResized(float width, float height);
	void render(const BaseCamera& camera, const glm::vec2& texClipSize, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer);
	void renderSSAO(const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& ssaoFB, Texture& ssaoTexture, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer);
	//void renderResultOnCamera(BaseCamera& camera);
	const Texture& getFinalTexture() const;
	int getOperationCount() const;
};

// Deals with datas
class PostProcessProxy
{
private:
	std::vector<std::shared_ptr<PostProcessOperationData>> m_operationDataList;

public:
	void drawUI();
	std::vector<std::shared_ptr<PostProcessOperationData>>::const_iterator begin() const { return m_operationDataList.begin(); }
	std::vector<std::shared_ptr<PostProcessOperationData>>::const_iterator end() const { return m_operationDataList.end(); }
	int getOperationCount() const;
	PostProcessOperationData* getOperationData(const std::string& operationName) const;
};

///////////////////////////////

class BloomPostProcessOperationData : public PostProcessOperationData
{
protected:
	int m_blurStepCount;
	float m_exposure;
	float m_gamma;


public:
	BloomPostProcessOperationData(const std::string& operationName);
	virtual void drawUI() override;

	int getBlurStepCount() const;
	float getExposure() const;
	float getGamma() const;
};

class BloomPostProcessOperation : public PostProcessOperation
{
private:
	GlHelper::Framebuffer m_pingPongFB[2];
	Texture m_colorTextures[2];

	GlHelper::Framebuffer m_highValuesFB;
	Texture m_highValuesTexture;

	std::shared_ptr<MaterialBlur> m_materialBlur;
	std::shared_ptr<MaterialBloom> m_materialBloom;
	std::shared_ptr<MaterialAdd> m_materialAdd;

public:
	BloomPostProcessOperation(const std::string& operationName);
	virtual void render(const PostProcessOperationData& operationData, const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& finalFB, Texture& finalTexture, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer) override;
	virtual void onViewportResized(float width, float height) override;
	//virtual const Texture* getResult() const override;
};

//////////////////////////////


class FlaresPostProcessOperationData : public PostProcessOperationData
{
protected:
	std::shared_ptr<MaterialFlares> m_materialFlares;

public:
	FlaresPostProcessOperationData(const std::string& operationName);
	virtual void drawUI() override;
	const MaterialFlares& getMaterial() const;
};

class FlaresPostProcessOperation : public PostProcessOperation
{
private:
	//GlHelper::Framebuffer m_finalFB;
	//Texture m_finalTexture;

	GlHelper::Framebuffer m_flaresFB;
	Texture m_flaresTexture;

	GLuint m_flareDatasBuffer;
	GLuint m_vao;

	std::shared_ptr<MaterialAdd> m_materialAdd;

public:
	FlaresPostProcessOperation(const std::string& operationName);
	virtual void render(const PostProcessOperationData& operationData, const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& finalFB, Texture& finalTexture, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer) override;
	virtual void onViewportResized(float width, float height) override;
	//virtual const Texture* getResult() const override;
};

//////////////////////////////


class SSAOPostProcessOperationData : public PostProcessOperationData
{
protected:
	std::shared_ptr<MaterialSSAO> m_materialSSAO;

public:
	SSAOPostProcessOperationData(const std::string& operationName);
	virtual void drawUI() override;
	const MaterialSSAO& getMaterial() const;
};

class SSAOPostProcessOperation : public PostProcessOperation
{
private:
	std::vector<glm::vec3> m_kernel;
	std::vector<glm::vec3> m_noise;
	Texture m_noiseTexture;

	Texture m_ssaoTexture;
	GlHelper::Framebuffer m_ssaoFB;

	std::shared_ptr<MaterialSSAOBlur> m_materialBlur;

public:
	SSAOPostProcessOperation(const std::string& operationName);
	virtual void render(const PostProcessOperationData& operationData, const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& finalFB, Texture& finalTexture, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer) override;
	virtual void onViewportResized(float width, float height) override;
};

//struct RenderDatas
//{
//	// Camera : 
//	glm::mat4& screenToView;
//	glm::mat4& VP;
//	// Mesh : 
//	Mesh& renderQuad;
//	// Post G pass textures :
//	Texture gPassPositionTexture;
//	Texture gPassHighValues;
//	Texture gPassNormalTexture;
//	Texture gPassDepthTexture;
//	// Post light pass textures : 
//	Texture beautyColor;
//	Texture beautyHighValues;
//	Texture beautyDepth;
//	// Lights : 
//	const std::vector<PointLight*>& pointLights;
//	const std::vector<DirectionalLight*>& directionalLights;
//	const std::vector<SpotLight*>& spotLights;
//};