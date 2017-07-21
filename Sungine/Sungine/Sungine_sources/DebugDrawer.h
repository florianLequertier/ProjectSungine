#pragma once

#include <glew/glew.h>
#include <glm/common.hpp>
#include <vector>

#include "Materials.h"

class RenderTarget;

class DebugDrawer
{
private:

	ResourcePtr<MaterialInstance> m_material;
	int m_maxPoint;
	GLuint m_vao;
	GLuint m_vboPositions;
	GLuint m_vboColors;

	std::vector<glm::vec3> m_points;
	std::vector<glm::vec3> m_colors;

public:
	static void drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color = glm::vec3(1,0,0));
	static void render(BaseCamera& camera, RenderTarget& renderTarget);
	static void clear();

// singleton implementation :
private:
	DebugDrawer();
	void drawLine_internal(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color = glm::vec3(1, 0, 0));
	void render_internal(const glm::mat4& projection, const glm::mat4& view);
	void draw_internal();
	void clear_internal();
	void updateVBOs_internal();

public:
	inline static DebugDrawer& get()
	{
		static DebugDrawer instance;

		return instance;
	}


	DebugDrawer(const DebugDrawer& other) = delete;
	void operator=(const DebugDrawer& other) = delete;
};

