#include "MeshVisualizer.h"
#include "Renderer.h"
#include "RenderTarget.h"


MeshVisualizer::MeshVisualizer()
//: m_viewportSize(400, 400)
	: m_renderTarget(std::make_shared<ColorAndDepthRTL>(), std::make_shared<ReflectionRTL>(), 400, 400)
	, m_cameraDistance(1.f)
	, m_cameraTarget(0.f, 0.f, 0.f)
	, m_cameraPhi(glm::pi<float>()*0.5f)
	, m_cameraTheta(glm::pi<float>()*0.5f)
{
	//m_camera.onViewportResized(m_viewportSize);

	m_pointLights.push_back(new PointLight(10, glm::vec3(1, 1, 1), glm::vec3(3, 3, 3)));

	DirectionalLight* newDirLight = new DirectionalLight(0.1f, glm::vec3(1, 1, 1), glm::vec3(-1, -1, 0));
	newDirLight->setCastShadows(false);
	newDirLight->position = glm::vec3(0, 2, 0);
	m_directionalLights.push_back(newDirLight);

	newDirLight = new DirectionalLight(0.1f, glm::vec3(1, 1, 1), glm::vec3(1, 1, 0));
	newDirLight->setCastShadows(false);
	newDirLight->position = glm::vec3(0, 2, 0);
	m_directionalLights.push_back(newDirLight);
}

MeshVisualizer::~MeshVisualizer()
{
	for (int i = 0; i < m_pointLights.size(); i++)
		delete m_pointLights[i];
	for (int i = 0; i < m_directionalLights.size(); i++)
		delete m_directionalLights[i];
	for (int i = 0; i < m_spotLights.size(); i++)
		delete m_spotLights[i];
}

void MeshVisualizer::setMesh(Mesh * mesh)
{
	m_visualizedMesh = mesh->makeSharedSubMesh(0);

	//m_camera.clearRenderBatches();

	//auto renderBatch = m_materialInstance->makeSharedRenderBatch();
	//renderBatch->add(m_visualizedMesh.get(), m_materialInstance.get());

	//m_camera.addRenderBatch(pipeline, renderBatch);

	//m_pipelineType = pipeline;
}

void MeshVisualizer::setMaterial(std::shared_ptr<MaterialInstance> material)
{
	m_materialInstance = material;
	Rendering::PipelineType newPipelineType = material->getPipelineType();

	m_camera.clearRenderBatches();
	auto renderBatch = m_materialInstance->makeSharedRenderBatch();
	renderBatch->add(m_visualizedMesh.get(), m_materialInstance.get());
	m_camera.addRenderBatch(newPipelineType, renderBatch);

	m_pipelineType = newPipelineType;

	//if (m_camera.getRenderBatches(m_pipelineType).size() == 0)
	//{
	//	setMesh(newPipelineType, getMeshFactory().getDefault("cube"));
	//}
	//else
	//{
	//	std::shared_ptr<IRenderBatch> renderBatch = m_camera.getRenderBatches(m_pipelineType).begin()->second;
	//	m_camera.clearRenderBatches(m_pipelineType);
	//	m_camera.addRenderBatch(newPipelineType, renderBatch);
	//}
	//m_pipelineType = newPipelineType;
}

void MeshVisualizer::render(Renderer& renderer)
{
	//TODO : refactor

	renderer.render(m_camera, m_renderTarget, m_pointLights, m_directionalLights, m_spotLights, false, nullptr);
}

void MeshVisualizer::rotateCamera(float x, float y)
{
	m_cameraTheta += x * 0.01;
	m_cameraPhi += y * 0.01;
	updateCameraView();
}

void MeshVisualizer::panCamera(float x, float y)
{
	m_cameraTarget.x += x * 0.05;
	m_cameraTarget.y += y * 0.05;
	updateCameraView();
}

void MeshVisualizer::drawUI()
{
	ImVec2 availContent = ImGui::GetContentRegionAvail();
	availContent.x -= 8.f;
	availContent.y -= 8.f;

	const glm::vec2 viewportSize = m_renderTarget.getSize();
	if (glm::abs(availContent.x - viewportSize.x) > 0.1f || glm::abs(availContent.y - viewportSize.y) > 0.1f)
	{
		m_renderTarget.setSize(glm::vec2(availContent.x, availContent.y));
	}

	ImGui::ImageButton((void*)m_renderTarget.getFinalFrame(), ImVec2(availContent.x, availContent.y) /*ImVec2(100, 100)*/, ImVec2(0, 1), ImVec2(1, 0));
	m_isSelected = ImGui::IsItemActive();
	bool isItemHovered = ImGui::IsItemHovered();

	ImVec2 dragDelta = ImGui::GetMouseDragDelta(0);
	if (m_isSelected && (dragDelta.x * dragDelta.x + dragDelta.y * dragDelta.y > 0.01f))
	{
		if (ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
			panCamera(dragDelta.x, dragDelta.y);
		else
			rotateCamera(dragDelta.x, dragDelta.y);
		ImGui::ResetMouseDragDelta(0);
	}
	ImGuiIO& io = ImGui::GetIO();
	float mouseWheel = io.MouseWheel;
	if (isItemHovered && (mouseWheel > 0.1f || mouseWheel < -0.1f))
	{
		m_cameraDistance += (mouseWheel*0.1f);
		if (m_cameraDistance < 0.01f)
			m_cameraDistance = 0.01f;
		updateCameraView();
	}

	if (isItemHovered)
	{
		ImGui::BeginTooltip();
		ImGui::Text("Phi : %f, Theta : %f.\n CameraTarget : (%f, %f).\n CameraDistance : %f.\n CameraEye : (%f, %f, %f).\n CameraUp : (%f, %f, %f)."
			, m_cameraPhi, m_cameraTheta, m_cameraTarget.x, m_cameraTarget.y, m_cameraDistance, m_cameraEye.x, m_cameraEye.y, m_cameraEye.z, m_cameraUp.x, m_cameraUp.y, m_cameraUp.z);
		ImGui::EndTooltip();
	}
}

void MeshVisualizer::updateCameraView()
{
	//glm::vec3 forward(glm::cos(m_cameraTheta) * glm::sin(m_cameraPhi), glm::cos(m_cameraPhi), glm::sin(m_cameraTheta) * glm::sin(m_cameraPhi));
	glm::vec3 forward(glm::cos(m_cameraTheta), 0, glm::sin(m_cameraTheta));
	forward *= -1;
	forward = glm::normalize(forward);
	m_cameraEye = m_cameraTarget - forward * m_cameraDistance;

	m_cameraUp = glm::vec3(0.f, m_cameraPhi < glm::pi<float>() ? 1.f : -1.f, 0.f);
	glm::vec3 right = glm::normalize(glm::cross(forward, m_cameraUp));
	m_cameraUp = glm::normalize(glm::cross(right, forward));

	m_camera.lookAt(m_cameraEye, m_cameraTarget, m_cameraUp);

	//m_camera.lookAt(glm::vec3(-2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}
