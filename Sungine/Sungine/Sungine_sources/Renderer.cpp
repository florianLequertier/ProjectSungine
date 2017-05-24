

#include "Renderer.h"

//forwards :
#include "Factories.h"
#include "EditorTools.h" 
#include "RenderBatch.h"
#include "BatchableWith.h"
#include "ReflectivePlane.h"
#include "Materials.h"

Renderer::Renderer(LightManager* _lightManager)  
{

	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();

	////////////////////// INIT SIMPLE_3D_DRAW MATERIAL ////////////////////////
	m_materialSimple3Ddraw.init(*getProgramFactory().getDefault("simple3DDraw"));

	////////////////////// INIT QUAD MESH ////////////////////////
	m_renderDatas.quadMesh.setMeshDatas(GL_TRIANGLES, (Mesh::USE_INDEX | Mesh::USE_VERTICES), 2, GL_STATIC_DRAW);
	m_renderDatas.quadMesh.triangleIndex = { 0, 1, 2, 2, 1, 3 };
	m_renderDatas.quadMesh.vertices = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0 };
	m_renderDatas.quadMesh.initGl();

	//////////////////// MAKE NEW LIGHTING MATERIALS ///////////////////

	m_pointLightMaterial = std::make_shared<MaterialPointLight>(*getProgramFactory().getDefault("pointLight"));
	CHECK_GL_ERROR("uniforms");
	m_directionalLightMaterial = std::make_shared<MaterialDirectionalLight>(*getProgramFactory().getDefault("directionalLight"));
	CHECK_GL_ERROR("uniforms");
	m_spotLightMaterial = std::make_shared<MaterialSpotLight>(*getProgramFactory().getDefault("spotLight"));
	CHECK_GL_ERROR("uniforms");

	//////////////////// INITIALIZE G BUFFER ///////////////////

	GlHelper::makeColorTexture(m_renderDatas.gPassColorTexture, width, height);
	m_renderDatas.gPassColorTexture.initGL();

	GlHelper::makeNormalTexture(m_renderDatas.gPassNormalTexture, width, height);
	m_renderDatas.gPassNormalTexture.initGL();

	GlHelper::makeDepthTexture(m_renderDatas.gPassDepthTexture, width, height);
	m_renderDatas.gPassDepthTexture.initGL();

	GlHelper::makeFloatColorTexture(m_renderDatas.gPassHightValuesTexture, width, height);
	m_renderDatas.gPassHightValuesTexture.initGL();

	gBufferFBO.bind(GL_FRAMEBUFFER);
	GLenum drawBufferForGPass[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	gBufferFBO.setDrawBuffers(4, drawBufferForGPass);
	gBufferFBO.attachTexture(&m_renderDatas.gPassColorTexture, GL_COLOR_ATTACHMENT0, 0);
	gBufferFBO.attachTexture(&m_renderDatas.gPassNormalTexture, GL_COLOR_ATTACHMENT1, 0);
	gBufferFBO.attachTexture(&m_renderDatas.gPassHightValuesTexture, GL_COLOR_ATTACHMENT2, 0);
	gBufferFBO.attachTexture(&m_renderDatas.gPassDepthTexture, GL_DEPTH_ATTACHMENT, 0);

	gBufferFBO.checkIntegrity();
	gBufferFBO.unbind();

	////////////////////// LIGHT MANAGER /////////////////////////
	lightManager = _lightManager;
	lightManager->setLightingMaterials(m_pointLightMaterial, m_directionalLightMaterial, m_spotLightMaterial);
	lightManager->setShadowMapCount(LightManager::SPOT, 10);
	lightManager->setShadowMapCount(LightManager::DIRECTIONAL, 5);
	lightManager->setShadowMapCount(LightManager::POINT, 10);

	////////////////////// SETUP LIGHT PASS FRAMEBUFFER AND TEXTURES /////////////////////////

	GlHelper::makeFloatColorTexture(m_renderDatas.lightPassHDRColor, width, height);
	m_renderDatas.lightPassHDRColor.initGL();

	GlHelper::makeDepthTexture(m_renderDatas.lightPassDepth, width, height);
	m_renderDatas.lightPassDepth.initGL();

	GlHelper::makeFloatColorTexture(m_renderDatas.lightPassHighValues, width, height);
	m_renderDatas.lightPassHighValues.initGL();

	m_lightPassBuffer.bind();
	m_lightPassBuffer.attachTexture(&m_renderDatas.lightPassHDRColor, GL_COLOR_ATTACHMENT0);
	m_lightPassBuffer.attachTexture(&m_renderDatas.lightPassHighValues, GL_COLOR_ATTACHMENT1);
	m_lightPassBuffer.attachTexture(&m_renderDatas.lightPassDepth, GlHelper::Framebuffer::AttachmentTypes::DEPTH);
	GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 };
	m_lightPassBuffer.setDrawBuffers(2, drawBuffers);
	m_lightPassBuffer.checkIntegrity();
	m_lightPassBuffer.unbind();

	////////////////////// SETUP SSAO PASS FRAMEBUFFER AND TEXTURES /////////////////////////
	GlHelper::makeRedTexture(m_ssaoTexture, width, height);
	m_ssaoTexture.initGL();

	m_SSAOPassBuffer.bind();
	m_SSAOPassBuffer.attachTexture(&m_ssaoTexture, GL_COLOR_ATTACHMENT0);
	m_SSAOPassBuffer.setDrawBuffer(GL_COLOR_ATTACHMENT0);
	m_SSAOPassBuffer.checkIntegrity();
	m_SSAOPassBuffer.unbind();

	////////////////////// FINALLY STORE THE VIEWPORT SIZE /////////////////////////
	m_intermediateViewportSize.x = width;
	m_intermediateViewportSize.y = height;

	CHECK_GL_ERROR("uniforms");

}

Renderer::~Renderer()
{
	delete lightManager;
}

const glm::vec2 & Renderer::getIntermediateViewportSize() const
{
	return m_intermediateViewportSize;
}


void Renderer::onResizeWindow(const glm::vec2& newWindowSize)
{
	const int width = newWindowSize.x, height = newWindowSize.y;

	if (width < 1 || height < 1)
		return;

	////////////////////// RESIZE G BUFFER /////////////////////////

	gBufferFBO.bind(GL_FRAMEBUFFER);

	// Detach textures
	gBufferFBO.detachTexture(GL_COLOR_ATTACHMENT0, 0);
	gBufferFBO.detachTexture(GL_COLOR_ATTACHMENT1, 0);
	gBufferFBO.detachTexture(GL_COLOR_ATTACHMENT2, 0);
	gBufferFBO.detachTexture(GL_COLOR_ATTACHMENT3, 0);
	gBufferFBO.detachTexture(GL_DEPTH_ATTACHMENT, 0);

	// Pop, resize and repush textures
	m_renderDatas.gPassColorTexture.freeGL();
	GlHelper::makeColorTexture(m_renderDatas.gPassColorTexture, width, height);
	m_renderDatas.gPassColorTexture.initGL();

	m_renderDatas.gPassNormalTexture.freeGL();
	GlHelper::makeNormalTexture(m_renderDatas.gPassNormalTexture, width, height);
	m_renderDatas.gPassNormalTexture.initGL();

	m_renderDatas.gPassHightValuesTexture.freeGL();
	GlHelper::makeFloatColorTexture(m_renderDatas.gPassHightValuesTexture, width, height);
	m_renderDatas.gPassHightValuesTexture.initGL();

	m_renderDatas.gPassDepthTexture.freeGL();
	GlHelper::makeDepthTexture(m_renderDatas.gPassDepthTexture, width, height);
	m_renderDatas.gPassDepthTexture.initGL();

	// Attach texture again
	gBufferFBO.attachTexture(&m_renderDatas.gPassColorTexture, GL_COLOR_ATTACHMENT0, 0);
	gBufferFBO.attachTexture(&m_renderDatas.gPassNormalTexture, GL_COLOR_ATTACHMENT1, 0);
	gBufferFBO.attachTexture(&m_renderDatas.gPassHightValuesTexture, GL_COLOR_ATTACHMENT2, 0);
	gBufferFBO.attachTexture(&m_renderDatas.gPassDepthTexture, GL_DEPTH_ATTACHMENT, 0);

	gBufferFBO.checkIntegrity();
	gBufferFBO.unbind();

	////////////////////// RESIZE LIGHT PASS TEXTURES /////////////////////////

	m_renderDatas.lightPassHDRColor.freeGL();
	GlHelper::makeColorTexture(m_renderDatas.lightPassHDRColor, width, height);
	m_renderDatas.lightPassHDRColor.initGL();

	m_renderDatas.lightPassDepth.freeGL();
	GlHelper::makeDepthTexture(m_renderDatas.lightPassDepth, width, height);
	m_renderDatas.lightPassDepth.initGL();

	m_renderDatas.lightPassHighValues.freeGL();
	GlHelper::makeFloatColorTexture(m_renderDatas.lightPassHighValues, width, height);
	m_renderDatas.lightPassHighValues.initGL();

	m_lightPassBuffer.bind();
	m_lightPassBuffer.attachTexture(&m_renderDatas.lightPassHDRColor, GL_COLOR_ATTACHMENT0);
	m_lightPassBuffer.attachTexture(&m_renderDatas.lightPassHighValues, GL_COLOR_ATTACHMENT1);
	m_lightPassBuffer.attachTexture(&m_renderDatas.lightPassDepth, GlHelper::Framebuffer::AttachmentTypes::DEPTH);
	m_lightPassBuffer.checkIntegrity();
	m_lightPassBuffer.unbind();

	////////////////////// RESIZE SSAO PASS TEXTURES /////////////////////////
	m_SSAOPassBuffer.bind();
	m_SSAOPassBuffer.detachTexture(GL_COLOR_ATTACHMENT0);

	m_ssaoTexture.freeGL();
	GlHelper::makeRedTexture(m_ssaoTexture, width, height);
	m_ssaoTexture.initGL();

	m_SSAOPassBuffer.attachTexture(&m_ssaoTexture, GL_COLOR_ATTACHMENT0);
	m_SSAOPassBuffer.checkIntegrity();
	m_SSAOPassBuffer.unbind();

	////////////////////// FINALLY STORE THE VIEWPORT SIZE /////////////////////////
	m_intermediateViewportSize.x = width;
	m_intermediateViewportSize.y = height;

	////////////////////// SETUP POST PROCESS MANAGER /////////////////////////
	m_postProcessManager.onViewportResized(m_intermediateViewportSize.x, m_intermediateViewportSize.y);
}


//void Renderer::initPostProcessQuad(std::string programBlit_vert_path, std::string programBlit_frag_path)
//{
//	//////////////////// BLIT shaders ////////////////////////
//	glProgram_blit = std::make_shared<MaterialBlit>(*getProgramFactory().get("blit"));
//}

//void Renderer::initialyzeShadowMapping(std::string progamShadowPass_vert_path, std::string progamShadowPass_frag_path,
//										std::string progamShadowPassOmni_vert_path, std::string progamShadowPassOmni_frag_path, std::string progamShadowPassOmni_geom_path)
//{
//	//////////////////////// SHADOWS //////////////////////////	
//
//
//	//glGenFramebuffers(1, &shadowFrameBuffer);
//	//glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
//
//	////initialyze shadowRenderBuffer : 
//	//glGenRenderbuffers(1, &shadowRenderBuffer);
//	//glBindRenderbuffer(GL_RENDERBUFFER, shadowRenderBuffer);
//	//glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, 1024, 1024);
//	//glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, shadowRenderBuffer);
//
//	////initialyze shadow texture : 
//	//glGenTextures(1, &shadowTexture);
//	//glBindTexture(GL_TEXTURE_2D, shadowTexture);
//	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//
//	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
//
//	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//	//{
//	//	fprintf(stderr, "Error on building shadow framebuffer\n");
//	//	exit(EXIT_FAILURE);
//	//}
//
//	//glBindTexture(GL_TEXTURE_2D, 0);
//	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//}


void Renderer::lightCullingPass(BaseCamera & camera, std::vector<PointLight*>& pointLights, std::vector<DirectionalLight*>& directionalLights, std::vector<SpotLight*>& spotLights, DebugDrawRenderer * debugDrawer)
{
	m_renderDatas.pointLightRenderDatas.clear();
	m_renderDatas.spotLightRenderDatas.clear();

	updateCulling(camera, pointLights, spotLights, m_renderDatas.pointLightRenderDatas, m_renderDatas.spotLightRenderDatas);

	for (auto directional : directionalLights)
		m_renderDatas.directionalLightRenderDatas.push_back(DirectionalLightRenderDatas(directional));
}

void Renderer::renderReflection(ReflectionCamera& camera, RenderTarget& renderTarget, const ReflectivePlane& reflectivePlane, std::vector<PointLight*>& pointLights, std::vector<DirectionalLight*>& directionalLights, std::vector<SpotLight*>& spotLights, DebugDrawRenderer* debugDrawer)
{
	glEnable(GL_CLIP_DISTANCE0);

	m_targetViewportSize = renderTarget.getSize();
	m_texClipSize = m_targetViewportSize / m_intermediateViewportSize;

	glClearColor(0, 0, 0, 0);

	// Make sure we clear the stencil buffer
	m_lightPassBuffer.bind();
	glStencilMask(0xFF);
	glClearStencil(1);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);
	m_lightPassBuffer.unbind();

	////////////////////////////////////////////////////////////////////////
	///////// BEGIN : Update render datas

	// Clear previous light datas
	m_renderDatas.directionalLightRenderDatas.clear();
	m_renderDatas.pointLightRenderDatas.clear();
	m_renderDatas.spotLightRenderDatas.clear();

	const glm::vec3& cameraPosition = camera.getCameraPosition();
	const glm::vec3& cameraForward = camera.getCameraForward();
	const glm::mat4& projection = camera.getProjectionMatrix();
	const glm::mat4& view = camera.getViewMatrix();

	m_renderDatas.Projection = &projection;
	m_renderDatas.View = &view;
	m_renderDatas.VP = projection * view;
	m_renderDatas.screenToView = glm::transpose(glm::inverse(projection));
	m_renderDatas.currentCameraID = camera.getCameraID();

	m_renderDatas.clipPlane = reflectivePlane.getClipPlane();
	m_renderDatas.currentReflectionLayer = renderTarget.getLayer("Reflection");

	if (m_renderDatas.currentReflectionLayer == nullptr)
	{
		PRINT_ERROR("You are trying to render a reflection into a render target which doesn't support reflections.");
		return;
	}

	///////// END : Update matrices
	////////////////////////////////////////////////////////////////////////

	// Cull lights
	lightCullingPass(camera, pointLights, directionalLights, spotLights, debugDrawer);

	// Render dynamic shadows
	shadowPass(camera, debugDrawer);

	// Render scene
	renderLightedScene(camera, debugDrawer);

	glDisable(GL_CLIP_DISTANCE0);

	// No post process for now for reflective camera
	camera.renderFrameOnTarget(m_renderDatas.lightPassHDRColor, *m_renderDatas.currentReflectionLayer, reflectivePlane);

	debugDrawer->drawOutputIfNeeded("output_reflection", m_renderDatas.lightPassHDRColor.glId);
	debugDrawer->drawOutputIfNeeded("output_reflectionInCamera", m_renderDatas.currentReflectionLayer->getFinalFrame());
}

void Renderer::render(BaseCamera& camera, RenderTarget& renderTarget, std::vector<PointLight*>& pointLights, std::vector<DirectionalLight*>& directionalLights, std::vector<SpotLight*>& spotLights, bool useGlobalPostProcess, DebugDrawRenderer* debugDrawer)
{
	m_targetViewportSize = renderTarget.getSize();
	m_texClipSize = m_targetViewportSize / m_intermediateViewportSize;

	glClearColor(0, 0, 0, 0);

	// Make sure we clear the stencil buffer
	m_lightPassBuffer.bind();
	glStencilMask(0xFF);
	glClearStencil(1);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);
	m_lightPassBuffer.unbind();

	////////////////////////////////////////////////////////////////////////
	///////// BEGIN : Update render datas

	// Clear previous light datas
	m_renderDatas.directionalLightRenderDatas.clear();
	m_renderDatas.pointLightRenderDatas.clear();
	m_renderDatas.spotLightRenderDatas.clear();

	const glm::vec3& cameraPosition = camera.getCameraPosition();
	const glm::vec3& cameraForward = camera.getCameraForward();
	const glm::mat4& projection = camera.getProjectionMatrix();
	const glm::mat4& view = camera.getViewMatrix();

	m_renderDatas.Projection = &projection;
	m_renderDatas.View = &view;
	m_renderDatas.VP = projection * view;
	m_renderDatas.screenToView = glm::transpose(glm::inverse(projection));
	m_renderDatas.currentCameraID = camera.getCameraID();

	m_renderDatas.clipPlane = glm::vec4(0,0,0,0);
	m_renderDatas.currentReflectionLayer = renderTarget.getLayer("Reflection");

	///////// END : Update matrices
	////////////////////////////////////////////////////////////////////////

	// Cull lights
	lightCullingPass(camera, pointLights, directionalLights, spotLights, debugDrawer);

	// Render dynamic shadows
	shadowPass(camera, debugDrawer);

	// Render scene
	renderLightedScene(camera, debugDrawer);

	// Render post process and final render to camera
	if (useGlobalPostProcess && camera.getPostProcessProxy().getOperationCount() > 0)
	{
		m_postProcessManager.render(camera, m_texClipSize, m_renderDatas, debugDrawer);
		//m_postProcessManager.renderResultOnCamera(camera);
		camera.renderFrameOnTarget(m_postProcessManager.getFinalTexture(), *renderTarget.getLayer(0));
	}
	else
	{
		camera.renderFrameOnTarget(m_renderDatas.lightPassHDRColor, *renderTarget.getLayer(0));
	}

	if (debugDrawer != nullptr)
	{
		debugDrawer->addSeparator();
		debugDrawer->drawOutputIfNeeded("endRender_postProcess", m_postProcessManager.getFinalTexture().glId);
		debugDrawer->drawOutputIfNeeded("endRender_renderTarget", renderTarget.getFinalFrame());
	}
}

void Renderer::renderLightedScene(const BaseCamera& camera, DebugDrawRenderer* debugDrawer)
{
	////////////////////////////////////////////////////////////////////////
	///////// BEGIN : Get batches
	const std::map<GLuint, std::shared_ptr<IRenderBatch>>& opaqueRenderBatches = camera.getRenderBatches(Rendering::PipelineType::DEFERRED_PIPILINE);
	const std::map<GLuint, std::shared_ptr<IRenderBatch>>& transparentRenderBatches = camera.getRenderBatches(Rendering::PipelineType::FORWARD_PIPELINE);
	///////// END : Get batches
	////////////////////////////////////////////////////////////////////////

	const float width = m_targetViewportSize.x; //m_intermediateViewportSize.x;
	const float height = m_targetViewportSize.y; //m_intermediateViewportSize.y;

	// Viewport 
	glViewport(0, 0, width, height);
	// Clear default buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	///////////// begin draw world

	////////////////////////////////////////////////////////////////////////
	///////// BEGIN : Deferred
	deferredPipeline(opaqueRenderBatches, camera, debugDrawer);
	///////// END : Deferred
	////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////
	///////// BEGIN : Transfert depth to main buffer
	gBufferFBO.bind(GL_READ_FRAMEBUFFER);
	m_lightPassBuffer.bind(GL_DRAW_FRAMEBUFFER);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	///////// END : Transfert depth to main buffer
	////////////////////////////////////////////////////////////////////////

	m_lightPassBuffer.bind();

	if (camera.getClearMode() == BaseCamera::ClearMode::SKYBOX)
		camera.renderSkybox();

	m_lightPassBuffer.unbind();

	////////////////////////////////////////////////////////////////////////
	///////// BEGIN :  Forward 
	forwardPipeline(transparentRenderBatches, camera);
	///////// END :  Forward
	////////////////////////////////////////////////////////////////////////
	
	///////////// end draw world

	/////////////////////////////////////////////////////////////////////////
	/////// BEGIN : Debug draw
	if (debugDrawer != nullptr)
	{
		debugDrawRenderer(*debugDrawer);
	}
	/////// END : Debug draw
	/////////////////////////////////////////////////////////////////////////

	// Prepare futur calls
	glViewport(0, 0, width, height);
	m_lightPassBuffer.bind();
}

void Renderer::shadowPass(const BaseCamera& camera, DebugDrawRenderer* debugDrawer)
{
	lightManager->generateShadowMaps(camera, m_renderDatas, debugDrawer);
}

void Renderer::gPass(const std::map<GLuint, std::shared_ptr<IRenderBatch>>& opaqueRenderBatches, const BaseCamera& camera)
{
	glEnable(GL_DEPTH_TEST);

	// Render batches (meshes and flags for now)
	for (auto& renderBatch : opaqueRenderBatches)
	{
		renderBatch.second->render(camera.getProjectionMatrix(), camera.getViewMatrix(), m_renderDatas);
	}
	
	// TODO RENDERING
	////render terrain :
	//terrain.render(projection, worldToView);
	//CHECK_GL_ERROR("error when rendering terrain");

	//terrain.renderGrassField(projection, worldToView);
	//CHECK_GL_ERROR("error when rendering grass");

	CHECK_GL_ERROR("error in G pass");
}

void Renderer::pushPointLightUniforms(PointLightRenderDatas& pointLightRenderDatas, const glm::mat4& viewToWorld, const glm::mat4& view)
{
	PointLight* currentLight = pointLightRenderDatas.light;
	//viewport = m_renderDatas.pointLightRenderDatas[i].viewport;
	GLuint shadowMapTextureId = pointLightRenderDatas.shadowMapTextureId;

	if (currentLight->getCastShadows() && shadowMapTextureId != 0)
	{
		// Send the shadow map texture
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMapTextureId);
		m_pointLightMaterial->setUniformShadowTexture(4);
		m_pointLightMaterial->setUniformShadowFactor(1.f);
	}
	else
	{
		m_pointLightMaterial->setUniformShadowFactor(0.f);
	}
	//resizeBlitQuad(viewport);

	m_pointLightMaterial->setUniformViewToWorld(viewToWorld);
	m_pointLightMaterial->setUniformFarPlane(100.f);

	lightManager->uniformPointLight(*currentLight, view);
}

void Renderer::pushSpotLightUniforms(SpotLightRenderDatas& spotLightRenderDatas, const glm::mat4& viewToWorld, const glm::mat4& view)
{
	SpotLight* currentLight = spotLightRenderDatas.light;
	//viewport = spotLightRenderDatas.viewport;
	GLuint shadowMapTextureId = spotLightRenderDatas.shadowMapTextureId;

	if (currentLight->getCastShadows() && shadowMapTextureId != 0)
	{
		//active the shadow map texture
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);
		m_spotLightMaterial->setUniformShadowTexture(4);
		m_spotLightMaterial->setUniformShadowFactor(1.f);
	}
	else
	{
		m_spotLightMaterial->setUniformShadowFactor(0.f);
	}
	//resize viewport
	//resizeBlitQuad(viewport);

	const glm::vec3 spotLightViewPosition = glm::vec3(view * glm::vec4(currentLight->position, 1.0));
	const glm::vec3 spotLightViewDirection = glm::vec3(view * glm::vec4(currentLight->direction, 0.0));
	const glm::vec3 spotLightViewUp = glm::vec3(view * glm::vec4(currentLight->up, 0.0));
	glm::mat4 projectionSpotLight = glm::perspective(currentLight->angle*2.f, 1.f, 0.1f, 100.f);
	glm::mat4 worldToLightSpotLight = glm::lookAt(spotLightViewPosition, spotLightViewPosition + spotLightViewDirection, spotLightViewUp);
	glm::mat4 ViewToLightScreen = projectionSpotLight * worldToLightSpotLight;
	//glUniformMatrix4fv(uniformWorldToLightScreen_spot, 1, false, glm::value_ptr(WorldToLightScreen));
	m_spotLightMaterial->setUniformViewToLight(ViewToLightScreen);

	lightManager->uniformSpotLight(*currentLight, view);
}

void Renderer::pushDirectionalLightUniforms(DirectionalLightRenderDatas& directionalLightRenderDatas, const glm::mat4& viewToWorld, const glm::mat4& view)
{
	DirectionalLight* currentLight = directionalLightRenderDatas.light;
	GLuint shadowMapTextureId = directionalLightRenderDatas.shadowMapTextureId;

	if (currentLight->getCastShadows() && shadowMapTextureId != 0)
	{
		//active the shadow map texture
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);
		m_directionalLightMaterial->setUniformShadowTexture(4);
		m_directionalLightMaterial->setUniformShadowFactor(1.f);
	}
	else
	{
		m_directionalLightMaterial->setUniformShadowFactor(0.f);
	}

	const glm::vec3 directionalLightViewPosition = glm::vec3(view * glm::vec4(currentLight->position, 1.0));
	const glm::vec3 directionalLightViewDirection = glm::vec3(view * glm::vec4(currentLight->direction, 0.0));
	const glm::vec3 directionalLightViewUp = glm::vec3(view * glm::vec4(currentLight->up, 0.0));
	float directionalShadowMapRadius = lightManager->getDirectionalShadowMapViewportSize()*0.5f;
	float directionalShadowMapNear = lightManager->getDirectionalShadowMapViewportNear();
	float directionalShadowMapFar = lightManager->getDirectionalShadowMapViewportFar();
	glm::vec3 orig = directionalLightViewPosition; ///*glm::vec3(0, 0, 1)*directionalShadowMapRadius + */glm::vec3(0, directionalLightViewPosition.y/*directionalShadowMapFar*0.5f*/, 0);
	glm::vec3 eye = -directionalLightViewDirection + orig;
	glm::mat4 projectionDirectionalLight = glm::ortho(-directionalShadowMapRadius, directionalShadowMapRadius, -directionalShadowMapRadius, directionalShadowMapRadius, directionalShadowMapNear, directionalShadowMapFar);
	glm::mat4 viewToLightDirectionalLight = glm::lookAt(eye, orig, directionalLightViewUp);
	glm::mat4 viewToLightScreen = projectionDirectionalLight * viewToLightDirectionalLight;
	//glUniformMatrix4fv(uniformWorldToLightScreen_directional, 1, false, glm::value_ptr(WorldToLightScreen));
	m_directionalLightMaterial->setUniformViewToLight(viewToLightScreen);

	lightManager->uniformDirectionalLight(*currentLight, view);
}

void Renderer::lightPass( const glm::vec3& cameraPosition, const glm::vec3& cameraForward, const glm::mat4& view)
{

	// The View to world matrix is the same of all the process, we compute it here :
	glm::mat4 viewToWorld = glm::inverse(view);

	///// begin light pass
	// Disable the depth test
	glDisable(GL_DEPTH_TEST);
	// Enable blending
	glEnable(GL_BLEND);
	// Setup additive blending
	glBlendFunc(GL_ONE, GL_ONE);


	// Render quad
	glm::vec4 viewport;

	/////////////////////////////////////////////////////////////////////////
	/////// BEGIN : Point light
	m_pointLightMaterial->use();
	m_pointLightMaterial->setUniformResize(m_texClipSize);

	// send screen to world matrix : 
	m_pointLightMaterial->setUniformScreenToView(m_renderDatas.screenToView);
	//m_pointLightMaterial->setUniformCameraPosition(cameraPosition);

	//geometry informations :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassColorTexture.glId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassNormalTexture.glId);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassDepthTexture.glId);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_ssaoTexture.glId);

	m_pointLightMaterial->setUniformColorTexture(0);
	m_pointLightMaterial->setUniformNormalTexture(1);
	m_pointLightMaterial->setUniformDepthTexture(2);
	m_pointLightMaterial->setUniformSSAOTexture(3);

	for (int i = 0; i < m_renderDatas.pointLightRenderDatas.size(); i++)
	{
		// Push light unfiforms
		pushPointLightUniforms(m_renderDatas.pointLightRenderDatas[i], viewToWorld, view);
		// Optimisation : Resize viewport
		viewport = m_renderDatas.pointLightRenderDatas[i].viewport;
		resizeBlitQuad(viewport);
		// Draw quad
		m_renderDatas.quadMesh.draw();
	}

	/////// END : Point light
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	/////// BEGIN : Spot light
	m_spotLightMaterial->use();
	m_spotLightMaterial->setUniformResize(m_texClipSize);

	// send screen to world matrix : 
	m_spotLightMaterial->setUniformScreenToView(m_renderDatas.screenToView);
	//m_spotLightMaterial->setUniformCameraPosition(cameraPosition);


	//geometry informations :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassColorTexture.glId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassNormalTexture.glId);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassDepthTexture.glId);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_ssaoTexture.glId);

	m_spotLightMaterial->setUniformColorTexture(0);
	m_spotLightMaterial->setUniformNormalTexture(1);
	m_spotLightMaterial->setUniformDepthTexture(2);
	m_spotLightMaterial->setUniformSSAOTexture(3);

	for (int i = 0; i < m_renderDatas.spotLightRenderDatas.size(); i++)
	{
		// Push light uniforms
		pushSpotLightUniforms(m_renderDatas.spotLightRenderDatas[i], viewToWorld, view);
		// Optimisation : Resize viewport
		viewport = m_renderDatas.spotLightRenderDatas[i].viewport;
		resizeBlitQuad(viewport);
		// Render Quad
		m_renderDatas.quadMesh.draw();
	}

	/////// END : Spot light
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	/////// BEGIN : Directional light

	//make sure that the blit quat cover all the screen : 
	resizeBlitQuad();

	//directionals : 
	m_directionalLightMaterial->use();
	m_directionalLightMaterial->setUniformResize(m_texClipSize);

	// send screen to world matrix : 
	m_directionalLightMaterial->setUniformScreenToView(m_renderDatas.screenToView);
	//m_directionalLightMaterial->setUniformCameraPosition(cameraPosition);


	//geometry informations :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassColorTexture.glId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassNormalTexture.glId);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_renderDatas.gPassDepthTexture.glId);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_ssaoTexture.glId);

	m_directionalLightMaterial->setUniformColorTexture(0);
	m_directionalLightMaterial->setUniformNormalTexture(1);
	m_directionalLightMaterial->setUniformDepthTexture(2);
	m_directionalLightMaterial->setUniformSSAOTexture(3);


	for (int i = 0; i < m_renderDatas.directionalLightRenderDatas.size(); i++)
	{
		// Push light uniforms
		pushDirectionalLightUniforms(m_renderDatas.directionalLightRenderDatas[i], viewToWorld, view);
		// Render quad
		m_renderDatas.quadMesh.draw();
	}
	/////// END : Directional light
	/////////////////////////////////////////////////////////////////////////

	// Disable blending
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	CHECK_GL_ERROR("error in light pass");
}

void Renderer::deferredPipeline(const std::map<GLuint, std::shared_ptr<IRenderBatch>>& opaqueRenderBatches, const BaseCamera& camera, DebugDrawRenderer* debugDrawer)
{
	////// begin G pass 
	gBufferFBO.bind();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gPass(opaqueRenderBatches, camera);
	gBufferFBO.unbind();
	////// end G pass

	////// begin SSAO pass
	m_SSAOPassBuffer.bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	m_SSAOPassBuffer.unbind();
	m_postProcessManager.renderSSAO(camera, m_texClipSize, m_SSAOPassBuffer, m_ssaoTexture, m_renderDatas, debugDrawer);
	////// end SSAO pass

	///// begin light pass
	m_lightPassBuffer.bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightPass(camera.getCameraPosition(), camera.getCameraForward(), camera.getViewMatrix());

	m_lightPassBuffer.unbind();
	///// end light pass
}

void Renderer::forwardPipeline(const std::map<GLuint, std::shared_ptr<IRenderBatch>>& transparentRenderBatches, const BaseCamera& camera)
{
	m_lightPassBuffer.bind();

	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	//glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto& renderBatch : transparentRenderBatches)
	{
		//renderBatch.second->renderForward(projection, view, m_renderDatas);
		renderBatch.second->render(camera.getProjectionMatrix(), camera.getViewMatrix(), m_renderDatas);
	}
	
	//glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	m_lightPassBuffer.unbind();
	CHECK_GL_ERROR("error in forward pass");
}

void Renderer::debugDrawRenderer(DebugDrawRenderer& debugDrawer) const
{
	debugDrawer.addSeparator();
	debugDrawer.drawOutputIfNeeded("gBuffer_color", m_renderDatas.gPassColorTexture.glId);
	debugDrawer.drawOutputIfNeeded("gBuffer_normal", m_renderDatas.gPassNormalTexture.glId);
	debugDrawer.drawOutputIfNeeded("gBuffer_depth", m_renderDatas.gPassDepthTexture.glId);
	debugDrawer.addSeparator();
	debugDrawer.drawOutputIfNeeded("beauty_color", m_renderDatas.lightPassHDRColor.glId);
	debugDrawer.drawOutputIfNeeded("beauty_depth", m_renderDatas.lightPassDepth.glId);
	debugDrawer.drawOutputIfNeeded("beauty_highValues", m_renderDatas.lightPassHighValues.glId);
	CHECK_GL_ERROR("error in render debug pass");
}

void Renderer::transferDepthTo(const GlHelper::Framebuffer & to, const glm::vec2 & depthTextureSize) const
{
	m_lightPassBuffer.bind(GL_READ_FRAMEBUFFER);
	to.bind(GL_DRAW_FRAMEBUFFER);
	glBlitFramebuffer(0, 0, depthTextureSize.x, depthTextureSize.y, 0, 0, depthTextureSize.x, depthTextureSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::transferDepthTo(const RenderTargetLayer & renderTargetLayer, const glm::vec2 & depthTextureSize) const
{
	m_lightPassBuffer.bind(GL_READ_FRAMEBUFFER);
	renderTargetLayer.bindFramebuffer(GL_DRAW_FRAMEBUFFER);
	glBlitFramebuffer(0, 0, depthTextureSize.x, depthTextureSize.y, 0, 0, depthTextureSize.x, depthTextureSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::debugDrawColliders(const BaseCamera& camera, const std::vector<Entity*>& entities)
{
	const int width = m_intermediateViewportSize.x;
	const int height = m_intermediateViewportSize.y;

	glm::mat4 projection = camera.getProjectionMatrix();//glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 view = camera.getViewMatrix();// glm::lookAt(camera.eye, camera.o, camera.up);
	glm::mat4 vp = projection * view;
	glm::mat4 screenToWorld = glm::transpose(glm::inverse(vp));

	//draw collider in forward rendering pass (no lightning)
	for (int i = 0; i < entities.size(); i++)
	{
		Collider* collider = static_cast<Collider*>(entities[i]->getComponent(Component::ComponentType::COLLIDER));
		if (collider == nullptr)
			continue;

		glm::vec3 colliderColor(1, 0, 0);
		if (entities[i]->getIsSelected())
			colliderColor = glm::vec3(1, 1, 0);

		collider->render(projection, view, colliderColor);
	}

	CHECK_GL_ERROR("error when rendering : debugDrawColliders");
}
//
//void Renderer::debugDrawDeferred()
//{
//	int width = Application::get().getWindowWidth(), height = Application::get().getWindowHeight();
//
//	///////////// begin draw blit quad
//	glDisable(GL_DEPTH_TEST);
//
//	//glUseProgram(glProgram_blit);
//	glProgram_blit->use();
//
//	for (int i = 0; i < 3; i++)
//	{
//		glViewport((width * i) / 4, 0, width / 4, height / 4);
//
//		glActiveTexture(GL_TEXTURE0);
//		// Bind gbuffer color texture
//		glBindTexture(GL_TEXTURE_2D, gbufferTextures[i]);
//		//glUniform1i(uniformTextureBlit, 0);
//		glProgram_blit->setUniformBlitTexture(0);
//
//		quadMesh.draw();
//	}
//
//	//shadow : 
//	if (lightManager->getShadowMapCount(LightManager::DIRECTIONAL) > 0)
//	{
//		glViewport((width * 3) / 4, 0, width / 4, height / 4);
//		glActiveTexture(GL_TEXTURE0);
//		lightManager->bindShadowMapTexture(LightManager::DIRECTIONAL, 0);
//		//glUniform1i(uniformTextureBlit, 0);
//		glProgram_blit->setUniformBlitTexture(0);
//
//		quadMesh.draw();
//	}
//
//
//	glViewport(0, 0, width, height);
//
//	glEnable(GL_DEPTH_TEST);
//	///////////// end draw blit quad
//}

void Renderer::debugDrawLights(const BaseCamera& camera, const std::vector<PointLight*>& pointLights, const std::vector<SpotLight*>& spotLights)
{
	int width = m_intermediateViewportSize.x;
	int height = m_intermediateViewportSize.y;

	glm::mat4 projection = camera.getProjectionMatrix(); //glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 view = camera.getViewMatrix(); //glm::lookAt(camera.eye, camera.o, camera.up);

	for (auto& light : pointLights)
	{
		light->renderBoundingBox(projection, view, glm::vec3(0, 0, 1));
	}

	for (auto& light : spotLights)
	{
		light->renderBoundingBox(projection, view, glm::vec3(0, 0, 1));
	}

	CHECK_GL_ERROR("error when rendering : debugDrawLights");
}


bool Renderer::passCullingTest(glm::vec4& viewport, const glm::mat4& projection, const glm::mat4& view, const glm::vec3 cameraPosition, BoxCollider& collider)
{
	glm::vec3 topRight = collider.topRight;
	glm::vec3 bottomLeft = collider.bottomLeft;

	bool insideFrustum = false;

	float maxX = 1;
	float maxY = 1;
	float minX = -1;
	float minY = -1;

	//first optimisation : test if the light is inside camera frustum 

	//is camera outside light bounding box ? 
	if (!(cameraPosition.x > bottomLeft.x && cameraPosition.x < topRight.x &&
		cameraPosition.y > bottomLeft.y && cameraPosition.y < topRight.y &&
		cameraPosition.z > bottomLeft.z && cameraPosition.z < topRight.z))
	{

		// ce code semble plus optimisé que celui retenu, mais je n'ai pas réussi à le faire marcher convenablement pour deux raisons : 
		// le carré généré est trop petit, il n'englobe pas tout le champs d'action de la lumière
		// il y a des bugs lorsqu'on place la camera au dessus ou en dessous de la lumière, la rotation s'effectue mal et le collider se deforme.
		
		/*
		// permet au collider d'être toujours face à la camera. 
		//cela permet d'éviter les erreurs du au caractère "AABB" du collider.
		//en effet, on stock le collider avec une seule diagonale (deux points), il faut qu'une fois projeté en repère ecrant cette diagonale soit aussi celle du carré projeté.
		glm::vec3 camToCollider = glm::normalize(collider.translation - cameraPosition);
		glm::mat4 facingCameraRotation = glm::lookAt(camToCollider, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		facingCameraRotation = glm::inverse(glm::mat4(facingCameraRotation));

		collider.applyRotation(glm::quat(facingCameraRotation));

		glm::vec4 tmpTopRight = projection * view  * glm::vec4(topRight, 1);
		glm::vec4 tmpBottomLeft = projection * view  * glm::vec4(bottomLeft, 1);

		topRight = glm::vec3(tmpTopRight.x / tmpTopRight.w, tmpTopRight.y / tmpTopRight.w, tmpTopRight.z / tmpTopRight.w);
		bottomLeft = glm::vec3(tmpBottomLeft.x / tmpBottomLeft.w, tmpBottomLeft.y / tmpBottomLeft.w, tmpBottomLeft.z / tmpBottomLeft.w);
		//now, topRight and bottom left are in screen space, and their coordinates are between -1 and 1 on all axis.

		//is light outside the camera frustum ? 
		if ((topRight.x < -1 && bottomLeft.x < -1) || (topRight.y < -1 && bottomLeft.y < -1)
			|| (topRight.x > 1 && bottomLeft.x > 1) || (topRight.y > 1 && bottomLeft.y > 1))
		{
			//we don't have to draw the light if its bounding box is outside the camera frustum
			insideFrustum = false;
		}
		*/

		//compute 8 points of 3D collider : 

		float colliderWidth = glm::abs(topRight.x - bottomLeft.x);
		float colliderDepth = glm::abs(topRight.z - bottomLeft.z);

		glm::vec3 colliderPoints[8] = { bottomLeft, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z + colliderDepth), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z + colliderDepth),
										topRight, glm::vec3(topRight.x, topRight.y, topRight.z - colliderDepth), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z - colliderDepth) };

		glm::mat4 vp = projection * view;
		glm::vec4 tmpColliderPoint;
		maxX = -1;
		maxY = -1;
		minX = 1;
		minY = 1;
		for (int i = 0; i < 8; i++)
		{
			tmpColliderPoint = projection * view * glm::vec4(colliderPoints[i], 1);
			colliderPoints[i] = glm::vec3(tmpColliderPoint.x / tmpColliderPoint.w, tmpColliderPoint.y / tmpColliderPoint.w, tmpColliderPoint.z / tmpColliderPoint.w);
		
			if (colliderPoints[i].x > maxX)
				maxX = colliderPoints[i].x;
			if(colliderPoints[i].x < minX)
				minX = colliderPoints[i].x;

			if (colliderPoints[i].y > maxY)
				maxY = colliderPoints[i].y;
			if (colliderPoints[i].y < minY)
				minY = colliderPoints[i].y;
		}

		//is light outside the camera frustum ? 
		if ((maxX < -1 && minX < -1) || (maxY < -1 && minY < -1)
			|| (maxX > 1 && minX > 1) || (maxY > 1 && minY > 1))
		{
			//we don't have to draw the light if its bounding box is outside the camera frustum
			insideFrustum = false;
		}


		//we have to draw the light if its boudning box is inside/intersect the camera frustum
		insideFrustum = true;
	}
	//we draw the light if we are inside its bounding box
	insideFrustum = true;




	//second optimisation : we modify the quad verticies such that it is reduce to the area of the light
	if (insideFrustum)
	{
		float width = maxX - minX;
		float height = maxY - minY;
		viewport = glm::vec4(minX, minY, width, height);

		//quadMesh.vertices = { minX, minY, minX + width, minY, minX, minY + height, minX + width , minY + height };
		//// update in CG : 
		//glBindBuffer(GL_ARRAY_BUFFER, quadMesh.vbo_vertices);
		//glBufferData(GL_ARRAY_BUFFER, quadMesh.vertices.size() * sizeof(float), &(quadMesh.vertices)[0], GL_STATIC_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return insideFrustum;
}

void Renderer::resizeBlitQuad(const glm::vec4 & viewport)
{
	m_renderDatas.quadMesh.vertices = { viewport.x, viewport.y, viewport.x + viewport.z, viewport.y, viewport.x, viewport.y + viewport.w, viewport.x + viewport.z , viewport.y + viewport.w };
	// update in CG : 
	glBindBuffer(GL_ARRAY_BUFFER, m_renderDatas.quadMesh.vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_renderDatas.quadMesh.vertices.size() * sizeof(float), &(m_renderDatas.quadMesh.vertices)[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::updateCulling(const BaseCamera& camera, std::vector<PointLight*>& pointLights, std::vector<SpotLight*>& spotLights, std::vector<PointLightRenderDatas>& pointLightRenderDatas, std::vector<SpotLightRenderDatas>& spotLightRenderDatas )
{
	glm::vec3 cameraPosition = camera.getCameraPosition();


	if (!pointLightRenderDatas.empty())
		pointLightRenderDatas.clear();
	if (!spotLightRenderDatas.empty())
		spotLightRenderDatas.clear();

	bool insideFrustum = false;

	float maxX = 1;
	float maxY = 1;
	float minX = -1;
	float minY = -1;

	//m_renderDatas.pointLightCount = 0;
	//m_renderDatas.spotLightCount = 0;

	int width = m_intermediateViewportSize.x;
	int height = m_intermediateViewportSize.y;

	glm::mat4 projection = camera.getProjectionMatrix(); //glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.f);
	glm::mat4 view = camera.getViewMatrix(); //glm::lookAt(camera.eye, camera.o, camera.up);

	int lastId = pointLights.size() - 1;;
	for (int lightIdx = 0; lightIdx < pointLights.size(); lightIdx++)
	{

		BoxCollider& collider = pointLights[lightIdx]->boundingBox;
		glm::vec3 topRight = collider.topRight;
		glm::vec3 bottomLeft = collider.bottomLeft;


		if (!(cameraPosition.x > bottomLeft.x && cameraPosition.x < topRight.x && cameraPosition.y > bottomLeft.y && cameraPosition.y < topRight.y && cameraPosition.z > bottomLeft.z && cameraPosition.z < topRight.z))
		{

			//compute 8 points of 3D collider : 

			float colliderWidth = glm::abs(topRight.x - bottomLeft.x);
			float colliderDepth = glm::abs(topRight.z - bottomLeft.z);

			glm::vec3 colliderPoints[8] = { bottomLeft, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z + colliderDepth), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z + colliderDepth),
				topRight, glm::vec3(topRight.x, topRight.y, topRight.z - colliderDepth), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z - colliderDepth) };

			glm::mat4 vp = projection * view;
			glm::vec4 tmpColliderPoint;
			maxX = -1;
			maxY = -1;
			minX = 1;
			minY = 1;
			for (int i = 0; i < 8; i++)
			{
				tmpColliderPoint = projection * view * glm::vec4(colliderPoints[i], 1);
				colliderPoints[i] = glm::vec3(tmpColliderPoint.x / tmpColliderPoint.w, tmpColliderPoint.y / tmpColliderPoint.w, tmpColliderPoint.z / tmpColliderPoint.w);

				if (colliderPoints[i].x > maxX)
					maxX = colliderPoints[i].x;
				if (colliderPoints[i].x < minX)
					minX = colliderPoints[i].x;

				if (colliderPoints[i].y > maxY)
					maxY = colliderPoints[i].y;
				if (colliderPoints[i].y < minY)
					minY = colliderPoints[i].y;
			}


			//is light outside the camera frustum ? 
			if ((maxX < -1 && minX < -1) || (maxY < -1 && minY < -1)
				|| (maxX > 1 && minX > 1) || (maxY > 1 && minY > 1))
			{
				//PointLight* tmpLight = pointLights[i];
				//pointLights[i] = pointLights[lastId];
				//pointLights[lastId] = tmpLight;

				//lastId--;
				//i--;

				//we don't have to draw the light if its bounding box is outside the camera frustum
				insideFrustum = false;
			}

			//we have to draw the light if its boudning box is inside/intersect the camera frustum
			insideFrustum = true;
		}
		//we draw the light if we are inside its bounding box
		insideFrustum = true;

		//second optimisation : we modify the quad verticies such that it is reduce to the area of the light
		if (insideFrustum)
		{
			float viewportWidth = maxX - minX;
			float viewportHeight = maxY - minY;
			
			pointLightRenderDatas.push_back(PointLightRenderDatas(glm::vec4(minX, minY, viewportWidth, viewportHeight), pointLights[lightIdx]));
		}

	}

	//lastId = spotLights.size() - 1;
	for (int lightIdx = 0; lightIdx < spotLights.size(); lightIdx++)
	{

		BoxCollider& collider = spotLights[lightIdx]->boundingBox;
		glm::vec3 topRight = collider.topRight;
		glm::vec3 bottomLeft = collider.bottomLeft;


		if (!(cameraPosition.x > bottomLeft.x && cameraPosition.x < topRight.x && cameraPosition.y > bottomLeft.y && cameraPosition.y < topRight.y && cameraPosition.z > bottomLeft.z && cameraPosition.z < topRight.z))
		{
			//compute 8 points of 3D collider : 

			float colliderWidth = glm::abs(topRight.x - bottomLeft.x);
			float colliderDepth = glm::abs(topRight.z - bottomLeft.z);

			glm::vec3 colliderPoints[8] = { bottomLeft, glm::vec3(bottomLeft.x, bottomLeft.y, bottomLeft.z + colliderDepth), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z), glm::vec3(bottomLeft.x + colliderWidth, bottomLeft.y, bottomLeft.z + colliderDepth),
				topRight, glm::vec3(topRight.x, topRight.y, topRight.z - colliderDepth), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z), glm::vec3(topRight.x - colliderWidth, topRight.y, topRight.z - colliderDepth) };

			glm::mat4 vp = projection * view;
			glm::vec4 tmpColliderPoint;
			maxX = -1;
			maxY = -1;
			minX = 1;
			minY = 1;
			for (int i = 0; i < 8; i++)
			{
				tmpColliderPoint = projection * view * glm::vec4(colliderPoints[i], 1);
				colliderPoints[i] = glm::vec3(tmpColliderPoint.x / tmpColliderPoint.w, tmpColliderPoint.y / tmpColliderPoint.w, tmpColliderPoint.z / tmpColliderPoint.w);

				if (colliderPoints[i].x > maxX)
					maxX = colliderPoints[i].x;
				if (colliderPoints[i].x < minX)
					minX = colliderPoints[i].x;

				if (colliderPoints[i].y > maxY)
					maxY = colliderPoints[i].y;
				if (colliderPoints[i].y < minY)
					minY = colliderPoints[i].y;
			}


			//is light outside the camera frustum ? 
			if ((maxX < -1 && minX < -1) || (maxY < -1 && minY < -1)
				|| (maxX > 1 && minX > 1) || (maxY > 1 && minY > 1))
			{
				//SpotLight* tmpLight = spotLights[i];
				//spotLights[i] = spotLights[lastId];
				//spotLights[lastId] = tmpLight;

				//lastId--;
				//i--;

				//we don't have to draw the light if its bounding box is outside the camera frustum
				insideFrustum = false;
			}

			//we have to draw the light if its boudning box is inside/intersect the camera frustum
			insideFrustum = true;
		}

		//we draw the light if we are inside its bounding box
		insideFrustum = true;

		//second optimisation : we modify the quad verticies such that it is reduce to the area of the light
		if (insideFrustum)
		{
			float width = maxX - minX;
			float height = maxY - minY;

			spotLightRenderDatas.push_back(SpotLightRenderDatas(glm::vec4(minX, minY, width, height), spotLights[lightIdx]));
		}
	}

	//m_renderDatas.spotLightCount = spotLightCullingInfos.size();
	//m_renderDatas.pointLightCount = pointLightCullingInfos.size();

}