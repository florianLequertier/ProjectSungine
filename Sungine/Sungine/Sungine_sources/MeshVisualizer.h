#pragma once

#include <memory>
#include "FrameBuffer.h"
#include "Camera.h"

class MaterialInstance;
class SubMesh;
class Renderer;
class RenderTarget;

class MeshVisualizer
{

private:

	RenderTarget m_renderTarget;
	//glm::vec2 m_viewportSize;
	SimpleCamera m_camera;
	std::shared_ptr<SubMesh> m_visualizedMesh;
	//Resource_ptr<Mesh> m_visualizedMesh;
	float m_cameraDistance;
	glm::vec3 m_cameraTarget;
	glm::vec3 m_cameraEye;
	glm::vec3 m_cameraUp;
	float m_cameraPhi;
	float m_cameraTheta;
	std::shared_ptr<MaterialInstance> m_materialInstance;
	std::vector<PointLight*> m_pointLights;
	std::vector<DirectionalLight*> m_directionalLights;
	std::vector<SpotLight*> m_spotLights;
	Rendering::PipelineType m_pipelineType;
	bool m_isSelected;

public:

	MeshVisualizer();
	~MeshVisualizer();

	// For Mesh Visualizer
	void setMesh(Mesh * mesh);
	void setMaterial(std::shared_ptr<MaterialInstance> material);
	void render(Renderer& renderer);
	void rotateCamera(float x, float y);
	void panCamera(float x, float y);
	void drawUI();

private:
	void updateCameraView();
};