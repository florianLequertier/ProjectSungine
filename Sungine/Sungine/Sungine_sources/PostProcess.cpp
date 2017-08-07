#include "glew/glew.h"
#include <random>

#include "PostProcess.h"
#include "Renderer.h"
#include "EngineMaterials.h"
#include "PostProcessMaterials.h"
#include "Factories.h"
#include "EditorTools.h"
#include "imgui/imgui.h"
#include "Camera.h"
#include "Lights.h"

////////////////////////////////////////////////////////////////
//// BEGIN : PostProcessManager

PostProcessManager::PostProcessManager()
{
	////////////////////// INIT POST PROCESS OPERATIONS ////////////////////////
	//auto& it = PostProcessFactory::instance().begin();
	//while (it != PostProcessFactory::instance().end())
	//{
	//	m_operationList[it->first] = it->second->cloneShared(it->first);
	//	it++;
	//}
	m_ssaoOperation = PostProcessFactory::instance().getInstanceShared("ssao");
	m_operationList.push_back(PostProcessFactory::instance().getInstanceShared("bloom"));
	m_operationList.push_back(PostProcessFactory::instance().getInstanceShared("flares"));

	////////////////////// INIT FINAL TEXTURE ////////////////////////
	GlHelper::makeFloatColorTexture(m_finalTexture, 400, 400);
	m_finalTexture.initGL();

	////////////////////// INIT FINAL FRAMEBUFFER ////////////////////////
	m_finalFB.bind();
	m_finalFB.attachTexture(&m_finalTexture, GL_COLOR_ATTACHMENT0);
	m_finalFB.checkIntegrity();
	m_finalFB.unbind();

	////////////////////// INIT BLIT MATERIAL ////////////////////////
	m_materialBlit = EngineMaterialsFactory::instance().getRef("resizedBlit");
}

void PostProcessManager::onViewportResized(float width, float height)
{
	m_finalFB.bind();
	m_finalFB.detachTexture(GL_COLOR_ATTACHMENT0);

	m_finalTexture.freeGL();
	GlHelper::makeFloatColorTexture(m_finalTexture, width, height);
	m_finalTexture.initGL();

	m_finalFB.attachTexture(&m_finalTexture, GL_COLOR_ATTACHMENT0);
	m_finalFB.checkIntegrity();
	m_finalFB.unbind();

	for (auto& operation : m_operationList)
	{
		operation->onViewportResized(width, height);
	}

	// Special operations : 
	if(m_ssaoOperation)
		m_ssaoOperation->onViewportResized(width, height);
}

void PostProcessManager::render(const BaseCamera& camera, const glm::vec2& texClipSize, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer)
{
	//auto& it = camera.getPostProcessProxy().begin();
	//while (it != camera.getPostProcessProxy().end())
	//{
	//	m_operationList[(*it)->getOperationName()]->render(**it, camera, m_renderQuad, beautyColor, beautyHighValues, beautyDepth, gPassHightValues, pointLights, debugDrawer);
	//	it++;
	//}

	// Copy the beauty color texture to the final texture. The rest of the effects will be added to this texture.
	m_finalFB.bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	m_materialBlit.use();
	m_materialBlit.setUniformResize(texClipSize);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderDatas.lightPassHDRColor.glId);
	m_materialBlit.setUniformBlitTexture(0);
	renderDatas.quadMesh.draw();
	m_finalFB.unbind();

	if (debugDrawer != nullptr)
		debugDrawer->drawOutputIfNeeded("tmp", m_finalTexture.glId);

	// Add effects to final texture one by one.
	for (auto operation : m_operationList)
	{
		auto postProcessProxy = camera.getPostProcessProxy();
		auto operationData = postProcessProxy.getOperationData(operation->getName());
		if (operationData != nullptr)
		{
			operation->render(*operationData, camera, texClipSize, m_finalFB, m_finalTexture, renderDatas, debugDrawer);
		}
	}
}

void PostProcessManager::renderSSAO(const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& ssaoFB, Texture& ssaoTexture, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer)
{
	auto postProcessProxy = camera.getPostProcessProxy();
	auto operationData = postProcessProxy.getOperationData("ssao");
	if (operationData != nullptr)
	{
		m_ssaoOperation->render(*operationData, camera, texClipSize, ssaoFB, ssaoTexture, renderDatas, debugDrawer);
	}
}

//void PostProcessManager::renderResultOnCamera(BaseCamera& camera)
//{
//	//assert(camera.getPostProcessProxy().getOperationCount() > 0);
//	//auto it = camera.getPostProcessProxy().end();
//	//it--;
//	//const Texture* resultTexture = m_operationList[(*it)->getOperationName()]->getResult();
//	camera.renderFrame(m_finalTexture /*resultTexture*/);
//}

const Texture& PostProcessManager::getFinalTexture() const
{
	return m_finalTexture;
}

int PostProcessManager::getOperationCount() const
{
	return m_operationList.size();
}

void PostProcessProxy::drawUI()
{
	ImGui::PushID(this);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	ImVec2 dropDownPos (ImGui::GetCursorPos().x + ImGui::GetWindowPos().x, ImGui::GetCursorPos().y + ImGui::GetWindowPos().y);
	if (ImGui::Button("add post process", ImVec2(ImGui::GetContentRegionAvailWidth(), 25)))
	{
		ImGui::OpenPopup("popUpAddOperation");
	}
	dropDownPos.y += 25;
	ImGui::SetNextWindowPos(ImVec2(dropDownPos.x + ImGui::GetStyle().WindowPadding.x, dropDownPos.y));
	ImGui::SetNextWindowContentWidth(ImGui::GetItemRectSize().x - ImGui::GetStyle().WindowPadding.x * 3.f );

	if (ImGui::BeginPopup("popUpAddOperation"))
	{
		auto& it = PostProcessDataFactory::instance().begin();
		while (it != PostProcessDataFactory::instance().end())
		{
			if (ImGui::Button(it->first.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 25)))
			{
				m_operationDataList.push_back(it->second->cloneShared(it->first));
				ImGui::CloseCurrentPopup();
			}
			it++;
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(2);

	int index = 0;
	int removeIdx = -1;
	for (auto& operationData : m_operationDataList)
	{
		bool shouldRemove = false;
		ImGui::PushID(index);
		
			/*if (ImGui::Ext::removableTreeNode(operationData->getOperationName().c_str(), shouldRemove))
			{
				operationData->drawUI();
				ImGui::TreePop();
			}*/
			if(ImGui::Ext::collapsingLabelWithRemoveButton(operationData->getOperationName().c_str(), shouldRemove))
				operationData->drawUI();

			if (shouldRemove)
				removeIdx = index;

		ImGui::PopID();

		index++;
	}

	if (removeIdx >= 0)
	{
		m_operationDataList.erase(m_operationDataList.begin() + removeIdx);
	}

	ImGui::PopID();
}

int PostProcessProxy::getOperationCount() const
{
	return m_operationDataList.size();
}

PostProcessOperationData * PostProcessProxy::getOperationData(const std::string& operationName) const
{
	auto& found = std::find_if(m_operationDataList.begin(), m_operationDataList.end(), [&operationName](const std::shared_ptr<PostProcessOperationData>& item) { return item->getOperationName() == operationName; });
	if (found != m_operationDataList.end())
		return found->get();
	else
		return nullptr;
}

//// END : PostProcessManager
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
//// BEGIN : BloomPostProcessOperation

REGISTER_POST_PROCESS(BloomPostProcessOperation, BloomPostProcessOperationData, "bloom")

BloomPostProcessOperationData::BloomPostProcessOperationData(const std::string& operationName) 
	: PostProcessOperationData(operationName)
	, m_blurStepCount(5)
	, m_gamma(2.0)
	, m_exposure(1.0)
{

}

void BloomPostProcessOperationData::drawUI()
{
	ImGui::InputInt("blur step count", &m_blurStepCount);
	ImGui::InputFloat("Exposure", &m_exposure);
	ImGui::InputFloat("Gamma", &m_gamma);
}

int BloomPostProcessOperationData::getBlurStepCount() const
{
	return m_blurStepCount;
}

float BloomPostProcessOperationData::getExposure() const
{
	return m_exposure;
}

float BloomPostProcessOperationData::getGamma() const
{
	return m_gamma;
}

///////////////////////

BloomPostProcessOperation::BloomPostProcessOperation(const std::string& operationName)
	: PostProcessOperation(operationName)
{
	m_materialBlur = std::make_shared<MaterialBlur>(PostProcessMaterialsFactory::instance().getRef("blur"));
	m_materialBloom = std::make_shared<MaterialBloom>(PostProcessMaterialsFactory::instance().getRef("bloom"));
	m_materialAdd = std::make_shared<MaterialAdd>(PostProcessMaterialsFactory::instance().getRef("add"));

	for (int i = 0; i < 2; i++)
	{
		GlHelper::makeFloatColorTexture(m_colorTextures[i], 400, 400);
		m_colorTextures[i].initGL();

		m_pingPongFB[i].bind();
		m_pingPongFB[i].attachTexture(&m_colorTextures[i], GL_COLOR_ATTACHMENT0);
		m_pingPongFB[i].checkIntegrity();
		m_pingPongFB[i].unbind();
	}

	GlHelper::makeFloatColorTexture(m_highValuesTexture, 400, 400);
	m_highValuesTexture.initGL();

	m_highValuesFB.bind();
	m_highValuesFB.attachTexture(&m_highValuesTexture, GL_COLOR_ATTACHMENT0);
	m_highValuesFB.checkIntegrity();
	m_highValuesFB.unbind();
}

void BloomPostProcessOperation::render(const PostProcessOperationData& operationData, const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& finalFB, Texture& finalTexture, RenderDatas& renderDatas, DebugDrawRenderer* debugDrawer)
{
	const BloomPostProcessOperationData& castedDatas = *static_cast<const BloomPostProcessOperationData*>(&operationData);
	const int blurStepCount = castedDatas.getBlurStepCount();
	const float gamma = castedDatas.getGamma();
	const float exposure = castedDatas.getExposure();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);


	m_highValuesFB.bind();
	glClear(GL_COLOR_BUFFER_BIT);

	m_materialAdd->use();
	m_materialAdd->glUniform_Resize(texClipSize);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderDatas.gPassHightValuesTexture.glId);
	m_materialAdd->glUniform_Texture01(0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderDatas.lightPassHighValues.glId);
	m_materialAdd->glUniform_Texture02(1);

	renderDatas.quadMesh.draw();

	m_highValuesFB.unbind();

	//// Begin blur pass
	bool horizontal = true;
	bool firstPass = true;
	int FBIndex = 1;
	int TexIndex = 0;
	m_materialBlur->use();
	m_materialBlur->glUniform_Resize(texClipSize);

	for (int i = 0; i < 2; i++)
	{
		m_pingPongFB[i].bind();
		glClear(GL_COLOR_BUFFER_BIT);
		m_pingPongFB[i].unbind();
	}

	for (int i = 0; i < blurStepCount; i++)
	{
		m_pingPongFB[FBIndex].bind();
		//m_materialBlur->use();
		glClear(GL_COLOR_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, firstPass ? m_highValuesTexture.glId : m_colorTextures[TexIndex].glId);
			m_materialBlur->glUniform_passId(TexIndex);
			m_materialBlur->glUniform_Texture(0);

			renderDatas.quadMesh.draw();

		m_pingPongFB[FBIndex].unbind();

		horizontal = !horizontal;
		FBIndex = horizontal ? 1 : 0;
		TexIndex = horizontal ? 0 : 1;

		if(firstPass)
			firstPass = false;
	}

	//// End blur pass


	//// Begin bloom pass
	finalFB.bind();

		m_materialBloom->use();
		m_materialBloom->glUniform_Resize(texClipSize);
		m_materialBloom->glUniform_Gamma(gamma);
		m_materialBloom->glUniform_Exposure(exposure);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_colorTextures[TexIndex].glId);
		m_materialBloom->glUniform_TextureBlur(0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, finalTexture.glId);
		m_materialBloom->glUniform_Texture(1);

		renderDatas.quadMesh.draw();

	finalFB.unbind();


	if (debugDrawer != nullptr)
	{
		debugDrawer->addSeparator();
		debugDrawer->drawOutputIfNeeded("bloom_highValues", m_highValuesTexture.glId);
		debugDrawer->drawOutputIfNeeded("bloom_blur", m_colorTextures[TexIndex].glId);
		debugDrawer->drawOutputIfNeeded("bloom_result", finalTexture.glId);
	}

	CHECK_GL_ERROR("error in Bloom Post Process.");

	glEnable(GL_DEPTH_TEST);

	//// End bloom pass
}

void BloomPostProcessOperation::onViewportResized(float width, float height)
{
	for (int i = 0; i < 2; i++)
	{
		m_pingPongFB[i].bind();
		m_pingPongFB[i].detachTexture(GL_COLOR_ATTACHMENT0);

		m_colorTextures[i].freeGL();
		GlHelper::makeFloatColorTexture(m_colorTextures[i], width, height);
		m_colorTextures[i].initGL();

		m_pingPongFB[i].attachTexture(&m_colorTextures[i], GL_COLOR_ATTACHMENT0);
		m_pingPongFB[i].checkIntegrity();
		m_pingPongFB[i].unbind();
	}

	m_highValuesFB.bind();
	m_highValuesFB.detachTexture(GL_COLOR_ATTACHMENT0);

	m_highValuesTexture.freeGL();
	GlHelper::makeFloatColorTexture(m_highValuesTexture, width, height);
	m_highValuesTexture.initGL();

	m_highValuesFB.attachTexture(&m_highValuesTexture, GL_COLOR_ATTACHMENT0);
	m_highValuesFB.checkIntegrity();
	m_highValuesFB.unbind();
}

//const Texture* BloomPostProcessOperation::getResult() const
//{
//	return &m_finalTexture;
//}

//// END : BloomPostProcessOperation
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//// BEGIN : FlaresPostProcessOperation

REGISTER_POST_PROCESS(FlaresPostProcessOperation, FlaresPostProcessOperationData, "flares")

FlaresPostProcessOperationData::FlaresPostProcessOperationData(const std::string & operationName)
	: PostProcessOperationData(operationName)
{
	m_materialFlares = PostProcessMaterialsFactory::instance().getRef("flares");
}


void FlaresPostProcessOperationData::drawUI()
{
	m_materialFlares.drawUI();
}

const MaterialFlares & FlaresPostProcessOperationData::getMaterial() const
{
	return *m_materialFlares;
}

FlaresPostProcessOperation::FlaresPostProcessOperation(const std::string & operationName)
	: PostProcessOperation(operationName)
{
	m_materialAdd = PostProcessMaterialsFactory::instance().getRef("add");

	GlHelper::makeFloatColorTexture(m_flaresTexture, 400, 400);
	m_flaresTexture.initGL();

	m_flaresFB.bind();
	m_flaresFB.attachTexture(&m_flaresTexture, GL_COLOR_ATTACHMENT0);
	m_flaresFB.checkIntegrity();
	m_flaresFB.unbind();

	//GlHelper::makeFloatColorTexture(m_finalTexture, 400, 400);
	//m_finalTexture.initGL();

	//m_finalFB.bind();
	//m_finalFB.attachTexture(&m_finalTexture, GL_COLOR_ATTACHMENT0);
	//m_finalFB.checkIntegrity();
	//m_finalFB.unbind();

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_flareDatasBuffer);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_flareDatasBuffer);
	// Position :
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 7, (void*)0); // 3 floats 
	// Color :
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 7, (void*)(sizeof(GL_FLOAT) * 3)); // 3 floats
	// Intensity :
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 7, (void*)(sizeof(GL_FLOAT) * 6)); // 1 float
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void FlaresPostProcessOperation::render(const PostProcessOperationData & operationData, const BaseCamera& camera, const glm::vec2& texClipSize, GlHelper::Framebuffer& finalFB, Texture& finalTexture, RenderDatas& renderDatas, DebugDrawRenderer * debugDrawer)
{
	const FlaresPostProcessOperationData* operationDatas = static_cast<const FlaresPostProcessOperationData*>(&operationData);
	const MaterialFlares& materialFlares = operationDatas->getMaterial();


	std::unordered_map<const MaterialFlares*, std::vector<int>[3]> m_lightsMapping;

	int index = 0;
	for (auto& lightRenderData : renderDatas.pointLightRenderDatas)
	{
		if (lightRenderData.light->getUseFlare())
		{
			if (lightRenderData.light->getFlareMaterial())
				m_lightsMapping[lightRenderData.light->getFlareMaterial()][LightManager::POINT].push_back(index);
			else
				m_lightsMapping[&materialFlares][LightManager::POINT].push_back(index);
		}
		index++;
	}
	index = 0;
	for (auto& lightRenderData : renderDatas.spotLightRenderDatas)
	{
		if (lightRenderData.light->getUseFlare())
		{
			if (lightRenderData.light->getFlareMaterial())
				m_lightsMapping[lightRenderData.light->getFlareMaterial()][LightManager::SPOT].push_back(index);
			else
				m_lightsMapping[&materialFlares][LightManager::SPOT].push_back(index);
		}
		index++;
	}
	index = 0;
	for (auto& lightRenderData : renderDatas.directionalLightRenderDatas)
	{
		if (lightRenderData.light->getUseFlare())
		{
			if (lightRenderData.light->getFlareMaterial())
				m_lightsMapping[lightRenderData.light->getFlareMaterial()][LightManager::DIRECTIONAL].push_back(index);
			else
				m_lightsMapping[&materialFlares][LightManager::DIRECTIONAL].push_back(index);
		}
		index++;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	m_flaresFB.bind();
	glClear(GL_COLOR_BUFFER_BIT);

	materialFlares.use();

	for (auto& lights : m_lightsMapping)
	{
		const MaterialFlares* currentMaterial = lights.first;
		std::vector<int>& pointLightsDatas = lights.second[LightManager::POINT];
		std::vector<int>& spotLightsDatas = lights.second[LightManager::SPOT];
		std::vector<int>& directionalLightsDatas = lights.second[LightManager::DIRECTIONAL];

		int flareCount = 0;
		std::vector<float> flareDatas;
		// For point lights :
		for (int lightIdx : pointLightsDatas)
		{
			PointLightRenderDatas& lightRenderData = renderDatas.pointLightRenderDatas[lightIdx];

			glm::vec3 lightPosition = lightRenderData.light->position;
			float angleToLight = glm::dot(glm::normalize(lightPosition - camera.getCameraPosition()), camera.getCameraForward());
			float maxAngleToLight = ((90.f * glm::pi<float>()) / 180.f);

			if (renderDatas.pointLightRenderDatas[lightIdx].light->getUseFlare())
			{
				flareDatas.push_back(lightRenderData.light->position.x);
				flareDatas.push_back(lightRenderData.light->position.y);
				flareDatas.push_back(lightRenderData.light->position.z);

				flareDatas.push_back(lightRenderData.light->color.x);
				flareDatas.push_back(lightRenderData.light->color.y);
				flareDatas.push_back(lightRenderData.light->color.z);

				float distanceToCamera = glm::distance(camera.getCameraPosition(), lightRenderData.light->position);
				float intensity = lightRenderData.light->intensity / (0.01 + distanceToCamera*distanceToCamera) *  (1.F - angleToLight / maxAngleToLight);
				flareDatas.push_back(intensity);
				flareCount++;
			}
		}
		// For spot lights :
		for (int lightIdx : spotLightsDatas)
		{
			SpotLightRenderDatas& lightRenderData = renderDatas.spotLightRenderDatas[lightIdx];

			glm::vec3 lightDirection = lightRenderData.light->direction;
			glm::vec3 lightPosition = lightRenderData.light->position;
			float angleToCamera = glm::dot(glm::normalize(camera.getCameraPosition() - lightPosition), lightDirection);
			float maxAngleToCamera = lightRenderData.light->angle * 0.5f;
			float angleToLight = glm::dot(glm::normalize(lightPosition - camera.getCameraPosition()), camera.getCameraForward());
			float maxAngleToLight = ((90.f * glm::pi<float>()) / 180.f);

			if (lightRenderData.light->getUseFlare() && angleToLight < maxAngleToCamera)
			{
				flareDatas.push_back(lightRenderData.light->position.x);
				flareDatas.push_back(lightRenderData.light->position.y);
				flareDatas.push_back(lightRenderData.light->position.z);

				flareDatas.push_back(lightRenderData.light->color.x);
				flareDatas.push_back(lightRenderData.light->color.y);
				flareDatas.push_back(lightRenderData.light->color.z);

				float distanceToCamera = glm::distance(camera.getCameraPosition(), lightRenderData.light->position);
				float intensity = (lightRenderData.light->intensity / (0.01 + distanceToCamera*distanceToCamera)) * (1.F - angleToLight / maxAngleToLight);
				flareDatas.push_back(intensity);
				flareCount++;
			}
		}
		// For directional lights :
		for (int lightIdx : directionalLightsDatas)
		{
			DirectionalLightRenderDatas& lightRenderData = renderDatas.directionalLightRenderDatas[lightIdx];

			glm::vec3 lightDirection = lightRenderData.light->direction;
			glm::vec3 lightPosition = lightRenderData.light->position;
			float angleToCamera = glm::dot(glm::normalize(camera.getCameraPosition() - lightPosition), lightDirection);
			float maxAngleToCamera = ((90.f * glm::pi<float>()) / 180.f);
			float angleToLight = glm::dot(glm::normalize(lightPosition - camera.getCameraPosition()), camera.getCameraForward());
			float maxAngleToLight = ((90.f * glm::pi<float>()) / 180.f);

			if (lightRenderData.light->getUseFlare() && angleToLight < maxAngleToCamera)
			{
				flareDatas.push_back(lightRenderData.light->position.x);
				flareDatas.push_back(lightRenderData.light->position.y);
				flareDatas.push_back(lightRenderData.light->position.z);

				flareDatas.push_back(lightRenderData.light->color.x);
				flareDatas.push_back(lightRenderData.light->color.y);
				flareDatas.push_back(lightRenderData.light->color.z);

				float intensity = lightRenderData.light->intensity * (1.F - angleToLight / maxAngleToLight);
				flareDatas.push_back(intensity);
				flareCount++;
			}
		}

		// Update vbo :
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_flareDatasBuffer);
		glBufferData(GL_ARRAY_BUFFER, flareDatas.size() * sizeof(float), &flareDatas[0], GL_DYNAMIC_DRAW);

		int texCount = 0;
		currentMaterial->pushInternalsToGPU(texCount);
		currentMaterial->glUniform_VP((camera.getProjectionMatrix() * camera.getViewMatrix()));

		glActiveTexture(GL_TEXTURE0 + texCount);
		glBindTexture(GL_TEXTURE_2D, renderDatas.lightPassDepth.glId);
		currentMaterial->glUniform_Depth(texCount);
		texCount++;

		glDrawArrays(GL_POINTS, 0, flareCount);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	m_flaresFB.unbind();

	glDisable(GL_BLEND);

	///////////////////////

	finalFB.bind();

		m_materialAdd->use();
		m_materialAdd->glUniform_Resize(texClipSize);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, finalTexture.glId);
		m_materialAdd->glUniform_Texture01(0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_flaresTexture.glId);
		m_materialAdd->glUniform_Texture02(1);

		renderDatas.quadMesh.draw();

	finalFB.unbind();

	///////////////////////

	if (debugDrawer != nullptr)
	{
		debugDrawer->addSeparator();
		debugDrawer->drawOutputIfNeeded("flares_flareTexture", m_flaresTexture.glId);
	}

	CHECK_GL_ERROR("error in Flares Post Process.");
}

void FlaresPostProcessOperation::onViewportResized(float width, float height)
{
	m_flaresFB.bind();
	m_flaresFB.detachTexture(GL_COLOR_ATTACHMENT0);

	m_flaresTexture.freeGL();
	GlHelper::makeFloatColorTexture(m_flaresTexture, width, height);
	m_flaresTexture.initGL();

	m_flaresFB.attachTexture(&m_flaresTexture, GL_COLOR_ATTACHMENT0);
	m_flaresFB.checkIntegrity();
	m_flaresFB.unbind();

	//////

	//m_finalFB.bind();
	//m_finalFB.detachTexture(GL_COLOR_ATTACHMENT0);

	//m_finalTexture.freeGL();
	//GlHelper::makeFloatColorTexture(m_finalTexture, width, height);
	//m_finalTexture.initGL();

	//m_finalFB.attachTexture(&m_finalTexture, GL_COLOR_ATTACHMENT0);
	//m_finalFB.checkIntegrity();
	//m_finalFB.unbind();
}

//const Texture * FlaresPostProcessOperation::getResult() const
//{
//	return &m_finalTexture;
//}

//// END : FlaresPostProcessOperation
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//// BEGIN : SSAOPostProcessOperation

REGISTER_POST_PROCESS(SSAOPostProcessOperation, SSAOPostProcessOperationData, "ssao")

SSAOPostProcessOperationData::SSAOPostProcessOperationData(const std::string & operationName)
	: PostProcessOperationData(operationName)
{
	m_materialSSAO = PostProcessMaterialsFactory::instance().getRef("ssao");
}

void SSAOPostProcessOperationData::drawUI()
{
	m_materialSSAO->drawUI();
}

const MaterialSSAO & SSAOPostProcessOperationData::getMaterial() const
{
	return *m_materialSSAO;
}

/////////////

SSAOPostProcessOperation::SSAOPostProcessOperation(const std::string & operationName)
	: PostProcessOperation(operationName)
{

	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine randomEngine;
	///////////////////////// SETUP KERNEL ////////////////////////////
	for (GLuint i = 0; i < 64; i++)
	{
		glm::vec3 sample(
			randomFloats(randomEngine) * 2.0 - 1.0,
			randomFloats(randomEngine)* 2.0 - 1.0,
			randomFloats(randomEngine)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(randomEngine);
		GLfloat scale = i / 64.f;
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		sample *= scale;
		m_kernel.push_back(sample);
	}
	///////////////////////// SETUP NOISE ////////////////////////////
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(randomEngine) * 2.0 - 1.0,
			randomFloats(randomEngine) *2.0 - 1.0,
			0.0
		);
		m_noise.push_back(noise);
	}
	m_noiseTexture.setTextureParameters(GL_RGB32F, GL_RGB, GL_FLOAT, false);
	m_noiseTexture.setTextureWrapping(GL_REPEAT, GL_REPEAT);
	m_noiseTexture.setTextureMinMaxFilters(GL_NEAREST, GL_NEAREST);
	m_noiseTexture.setPixels(m_noise, 4, 4);
	m_noiseTexture.initGL();
	///////////////////////// SSAO TEXTURE AND FRAMEBUFFER ////////////////////////////
	GlHelper::makeRedTexture(m_ssaoTexture, 400, 400);
	m_ssaoTexture.initGL();
	m_ssaoFB.bind();
	m_ssaoFB.attachTexture(&m_ssaoTexture, GL_COLOR_ATTACHMENT0);
	m_ssaoFB.setDrawBuffer(GL_COLOR_ATTACHMENT0);
	m_ssaoFB.unbind();

	///////////////////////// INIT BLUR MATERIAL ////////////////////////////
	m_materialBlur = PostProcessMaterialsFactory::instance().getRef("ssaoBlur");
}

void SSAOPostProcessOperation::render(const PostProcessOperationData & operationData, const BaseCamera & camera, const glm::vec2& texClipSize, GlHelper::Framebuffer & finalFB, Texture & finalTexture, RenderDatas& renderDatas, DebugDrawRenderer * debugDrawer)
{

	///////////////// RENDER SSAO ////////////////////
	const SSAOPostProcessOperationData* ssaoOperationData = static_cast<const SSAOPostProcessOperationData*>(&operationData);
	const MaterialSSAO& material = ssaoOperationData->getMaterial();

	m_ssaoFB.bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	material.use();
	material.glUniform_Resize(texClipSize);

	int texCount = 0;
	material.pushInternalsToGPU(texCount);
	// Send Depth : 
	glActiveTexture(GL_TEXTURE0 + texCount);
	glBindTexture(GL_TEXTURE_2D, renderDatas.gPassDepthTexture.glId);
	material.glUniform_Depth(texCount);
	texCount++;
	// Send Normals :
	glActiveTexture(GL_TEXTURE0 + texCount);
	glBindTexture(GL_TEXTURE_2D, renderDatas.gPassNormalTexture.glId);
	material.glUniform_Normals(texCount);
	texCount++;
	// Send Noise :
	glActiveTexture(GL_TEXTURE0 + texCount);
	glBindTexture(GL_TEXTURE_2D, m_noiseTexture.glId);
	material.glUniform_NoiseTexture(texCount);
	texCount++;
	// Send Kernel : 
	material.glUniform_Kernel(m_kernel);
	// Send Projection :
	material.glUniform_Projection(camera.getProjectionMatrix());
	// Send ScreenToView :
	material.glUniform_ScreenToView(renderDatas.screenToView);

	renderDatas.quadMesh.draw();

	m_ssaoFB.unbind();

	///////////////// BLUR RESULT ////////////////////

	finalFB.bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	m_materialBlur->use();
	m_materialBlur->glUniform_Resize(texClipSize);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ssaoTexture.glId);
	m_materialBlur->glUniform_Texture(0);

	renderDatas.quadMesh.draw();

	finalFB.unbind();

	///////////////// DEBUG OUTPUT ////////////////////

	if (debugDrawer != nullptr)
	{
		debugDrawer->addSeparator();
		debugDrawer->drawOutputIfNeeded("ssao_mainPass", m_ssaoTexture.glId);
		debugDrawer->drawOutputIfNeeded("ssao_blurred", finalTexture.glId);
	}

	CHECK_GL_ERROR("error in SSAO pass.");
}

void SSAOPostProcessOperation::onViewportResized(float width, float height)
{

	//////////// UPDATE SSAO TEXTURE AND FRAMEBUFFER ////////////
	m_ssaoFB.bind();
	m_ssaoFB.detachTexture(GL_COLOR_ATTACHMENT0);

	m_ssaoTexture.freeGL();
	GlHelper::makeRedTexture(m_ssaoTexture, width, height);
	m_ssaoTexture.initGL();

	m_ssaoFB.attachTexture(&m_ssaoTexture, GL_COLOR_ATTACHMENT0);
	m_ssaoFB.unbind();

}

//// END : SSAOPostProcessOperation
////////////////////////////////////////////////////////////////
