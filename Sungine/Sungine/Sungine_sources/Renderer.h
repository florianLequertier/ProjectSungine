#pragma once

#include "glew/glew.h"

#include "Mesh.h"
#include "LightManager.h"
#include "Application.h"
#include "Entity.h"
#include "Collider.h"
#include "Terrain.h"
#include "Skybox.h"
#include "Materials.h"
#include "Flag.h"
#include "ParticleEmitter.h"
#include "FrameBuffer.h"
#include "ErrorHandler.h"

#include "PostProcess.h"
#include "RenderDatas.h"
#include "RenderTarget.h"

class DebugDrawRenderer;
class ReflectivePlane;

//struct LightCullingInfo
//{
//	glm::vec4 viewport;
//	int idx;
//
//	inline LightCullingInfo(const glm::vec4& _viewport, int _idx) : viewport(_viewport), idx(_idx) {}
//};


class Renderer
{
	enum LightType { POINT = 0, DIRECTIONAL = 1, SPOT = 2 };

private:
	// Viewport size :
	glm::vec2 m_intermediateViewportSize;
	glm::vec2 m_targetViewportSize;
	glm::vec2 m_texClipSize;

	// FrameBuffers :
	// For G pass :
	GlHelper::Framebuffer gBufferFBO;
	// For light pass :
	GlHelper::Framebuffer m_lightPassBuffer;
	// For SSAO pass :
	GlHelper::Framebuffer m_SSAOPassBuffer;
	Texture m_ssaoTexture;

	// Render datas :
	RenderDatas m_renderDatas;

	MaterialSimple3DDraw m_materialSimple3Ddraw;

	// Lighting materials :
	std::shared_ptr<MaterialPointLight> m_pointLightMaterial;
	std::shared_ptr<MaterialDirectionalLight> m_directionalLightMaterial;
	std::shared_ptr<MaterialSpotLight> m_spotLightMaterial;

	// Managers :
	//post process
	PostProcessManager m_postProcessManager;
	//light system
	LightManager* lightManager;

	////shadows :
	//GLuint shadowFrameBuffer;
	//GLuint shadowRenderBuffer;
	//GLuint shadowTexture;

public:
	Renderer(LightManager* _lightManager);
	~Renderer();

	//return the final frame, after all render process
	const glm::vec2& getIntermediateViewportSize() const;

	//update deferred textures used by the FBO when we resize the screen.
	void onResizeWindow(const glm::vec2& newWindowSize);

	//initialyze buffers for blit quad.
	//void initPostProcessQuad(std::string programBlit_vert_path, std::string programBlit_frag_path);

	//void initialyzeShadowMapping(std::string progamShadowPass_vert_path, std::string progamShadowPass_frag_path, std::string progamShadowPassOmni_vert_path, std::string progamShadowPassOmni_frag_path, std::string progamShadowPassOmni_geom_path);

	// Render the scene through a reflective camera. Practically the same as render() but doesn't render post processing.
	void renderReflection(ReflectionCamera& camera, RenderTarget& renderTarget, const ReflectivePlane& reflectivePlane, std::vector<PointLight*>& pointLights, std::vector<DirectionalLight*>& directionalLights, std::vector<SpotLight*>& spotLights, DebugDrawRenderer* debugDrawer);
	// Main render function. Will render the sene into a camera texture.
	void render(BaseCamera& camera, RenderTarget& renderTarget, std::vector<PointLight*>& pointLights, std::vector<DirectionalLight*>& directionalLights, std::vector<SpotLight*>& spotLights, bool useGlobalPostProcess = true, DebugDrawRenderer* debugDrawer = nullptr);
	
	void debugDrawRenderer(DebugDrawRenderer& debugDrawer) const;

	// Transfert the depth of the final m_lightPassBuffer buffer to the given framebuffer.
	void transferDepthTo(const GlHelper::Framebuffer& to, const glm::vec2& depthTextureSize) const;
	void transferDepthTo(const RenderTargetLayer & renderTargetLayer, const glm::vec2 & depthTextureSize) const;

	//draw colliders on scene.
	void debugDrawColliders(const BaseCamera& camera, const std::vector<Entity*>& entities);

	//draw textures of gPass.
	//void debugDrawDeferred();

	//draw lights bounding box.
	void debugDrawLights(const BaseCamera& camera, const std::vector<PointLight*>& pointLights, const std::vector<SpotLight*>& spotLights);

	//check if a light bounding box has to be drawn, in that case it scales the blit quad to render only what is influenced by the light.
	bool passCullingTest(glm::vec4& viewport, const glm::mat4& projection, const glm::mat4& view, const glm::vec3 cameraPosition, BoxCollider& collider);

	//resize the blit quad, changing its vertices coordinates
	void resizeBlitQuad(const glm::vec4& viewport = glm::vec4(-1,-1,2,2));

private:
	// Camera culling for point lights and spot lights
	void updateCulling(const BaseCamera& camera, std::vector<PointLight*>& pointLights, std::vector<SpotLight*>& spotLights, std::vector<PointLightRenderDatas>& pointLightRenderDatas, std::vector<SpotLightRenderDatas>& spotLightRenderDatas);
	// Apply light culling, and populate lightRenderDatas
	void lightCullingPass(BaseCamera& camera, std::vector<PointLight*>& pointLights, std::vector<DirectionalLight*>& directionalLights, std::vector<SpotLight*>& spotLights, DebugDrawRenderer* debugDrawer = nullptr);
	// Compute shadow maps
	void shadowPass(const BaseCamera& camera, DebugDrawRenderer* debugDrawer = nullptr);
	// Render the scene with lights.
	void renderLightedScene(const BaseCamera& camera, DebugDrawRenderer* debugDrawer = nullptr);

	void gPass(const std::map<GLuint, std::shared_ptr<IRenderBatch>>& opaqueRenderBatches, const BaseCamera& camera);
	void lightPass(const glm::vec3& cameraPosition, const glm::vec3& cameraForward, const glm::mat4& view);
	void deferredPipeline(const std::map<GLuint, std::shared_ptr<IRenderBatch>>& opaqueRenderBatches, const BaseCamera& camera, DebugDrawRenderer* debugDrawer);
	void forwardPipeline(const std::map<GLuint, std::shared_ptr<IRenderBatch>>& transparentRenderBatches, const BaseCamera& camera);

	// Utility function to push all light/shadow uniforms to GPU
	void Renderer::pushPointLightUniforms(PointLightRenderDatas& pointLightRenderDatas, const glm::mat4& viewToWorld, const glm::mat4& view);
	// Utility function to push all light/shadow uniforms to GPU
	void Renderer::pushSpotLightUniforms(SpotLightRenderDatas& spotLightRenderDatas, const glm::mat4& viewToWorld, const glm::mat4& view);
	// Utility function to push all light/shadow uniforms to GPU
	void Renderer::pushDirectionalLightUniforms(DirectionalLightRenderDatas& directionalLightRenderDatas, const glm::mat4& viewToWorld, const glm::mat4& view);
};

