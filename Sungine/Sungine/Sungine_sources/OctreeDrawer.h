#pragma once

#include <vector>
#include "glew/glew.h"
#include "glm/glm.hpp"
#include "glm/common.hpp"

#include "ResourcePointer.h"
#include "Materials.h"

//class MaterialInstancedUnlit;

class OctreeDrawer
{
public:
	enum VboTypes { VERTICES = 0, POSITIONS, SIZES };

private:
	std::vector<int> triangleIndex;
	std::vector<float> vertices;
	std::vector<float> positions;
	std::vector<float> sizes;

	GLuint vbo_index;
	GLuint vbo_vertices;
	GLuint vbo_uvs;
	GLuint vbo_normals;
	GLuint vbo_pos;
	GLuint vbo_size;
	GLuint vao;

	int triangleCount;
	int instanceCount;

	ResourcePtr<MaterialInstance> material;

public:
	void initGl();
	void updateInstancedVBOs();

	//add elements which will be drawn when render is called
	void addDrawItems(const std::vector<glm::vec3>& centers, const std::vector<float>& halfSizes);
	//clear all drawable elements
	void clear();

	//render all drawable elements
	void render(const glm::mat4& projection, const glm::mat4& view);
	void draw();

private:
	OctreeDrawer();

public:
	inline static OctreeDrawer& get()
	{
		static OctreeDrawer instance;

		return instance;
	}

	OctreeDrawer(const OctreeDrawer& other) = delete;
	void operator=(const OctreeDrawer& other) = delete;
};


