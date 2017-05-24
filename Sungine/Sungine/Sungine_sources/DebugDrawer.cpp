

#include "DebugDrawer.h"
//forwards : 
#include "Factories.h"
#include "Camera.h"
#include "RenderTarget.h"

DebugDrawer::DebugDrawer() : m_maxPoint(10000), m_material(nullptr)
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vboPositions);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
	glBufferData(GL_ARRAY_BUFFER, m_maxPoint * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	glGenBuffers(1, &m_vboColors);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
	glBufferData(GL_ARRAY_BUFFER, m_maxPoint * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_material = getMaterialFactory().getDefault("debugDrawer");
	assert(m_material.isValid());
}

void DebugDrawer::drawLine_internal(const glm::vec3 & from, const glm::vec3 & to, const glm::vec3 & color)
{
	m_points.push_back(from);
	m_colors.push_back(color);
	m_points.push_back(to);
	m_colors.push_back(color); 
}

void DebugDrawer::render_internal(const glm::mat4 & projection, const glm::mat4 & view)
{
	if (m_points.size() == 0)
		return;

	updateVBOs_internal();

	glm::mat4 MVP = projection * view;

	const MaterialDebugDrawer* castedMaterial = static_cast<MaterialDebugDrawer*>(m_material.get()); //TODO : changer ça apres la refonte du pipeline de rendu

	castedMaterial->use();
	castedMaterial->setUniform_MVP(MVP);

	draw_internal();
}

void DebugDrawer::draw_internal()
{
	glBindVertexArray(m_vao);

	glDrawArrays(GL_LINES, 0, m_points.size());

	glBindVertexArray(0);
}

void DebugDrawer::clear_internal()
{
	m_points.clear();
	m_colors.clear();

	updateVBOs_internal();
}

void DebugDrawer::updateVBOs_internal()
{
	if (m_points.size() == 0)
		return;

	if (m_points.size() > m_maxPoint) {
		m_maxPoint = m_points.size();
		glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
		glBufferData(GL_ARRAY_BUFFER, m_maxPoint * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
		glBufferData(GL_ARRAY_BUFFER, m_maxPoint * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_points.size() * sizeof(glm::vec3), &m_points[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_colors.size() * sizeof(glm::vec3), &m_colors[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void DebugDrawer::drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color)
{
	DebugDrawer::get().drawLine_internal(from, to, color);
}

void DebugDrawer::render(BaseCamera& camera, RenderTarget& renderTarget)
{
	renderTarget.bindFramebuffer();
	DebugDrawer::get().render_internal(camera.getProjectionMatrix(), camera.getViewMatrix());
	renderTarget.unbindFramebuffer();
	CHECK_GL_ERROR("Render error into debugDrawer::render().");
}

void DebugDrawer::clear()
{
	DebugDrawer::get().clear_internal();
}
