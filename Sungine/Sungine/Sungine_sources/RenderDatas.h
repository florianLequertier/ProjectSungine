#pragma once

#include <glm/common.hpp>

#include "Mesh.h"
#include "Texture.h"
#include "Lights.h"
#include "FrameBuffer.h"

class RenderTargetLayer;

struct RenderDatas
{
	Mesh quadMesh;

	Texture lightPassHDRColor;
	Texture lightPassDepth;
	Texture lightPassHighValues;
	//GlHelper::Renderbuffer lightPassStencilBuffer;

	Texture gPassColorTexture;
	Texture gPassNormalTexture;
	Texture gPassDepthTexture;
	Texture gPassHightValuesTexture;

	glm::mat4 screenToView;
	glm::mat4 VP;
	const glm::mat4* Projection;
	const glm::mat4* View;
	ID currentCameraID;
	glm::vec4 clipPlane;
	RenderTargetLayer* currentReflectionLayer;

	//std::vector<PointLight*>* pointLights;
	//std::vector<DirectionalLight*>* directionalLights;
	//std::vector<SpotLight*>* spotLights;

	//light count after culling :
	//int pointLightCount;
	//int spotLightCount;

	//for light culling :
	//std::vector<LightCullingInfo> pointLightCullingInfos;
	//std::vector<LightCullingInfo> spotLightCullingInfos;
	std::vector<PointLightRenderDatas> pointLightRenderDatas;
	std::vector<SpotLightRenderDatas> spotLightRenderDatas;
	std::vector<DirectionalLightRenderDatas> directionalLightRenderDatas;
};