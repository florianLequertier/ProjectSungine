

#include "OctreeDrawer.h"
//forwards : 
#include "Factories.h"
#include "Materials.h"

OctreeDrawer::OctreeDrawer() 
	: material(getMaterialFactory().getDefault("wireframeInstanced"))
{
	//push wireframe cube : 
	//up face :
	vertices.push_back(0.5f);
	vertices.push_back(0.5f);
	vertices.push_back(0.5f);

	vertices.push_back(-0.5f);
	vertices.push_back(0.5f);
	vertices.push_back(0.5f);

	vertices.push_back(-0.5f);
	vertices.push_back(0.5f);
	vertices.push_back(-0.5f);

	vertices.push_back(0.5f);
	vertices.push_back(0.5f);
	vertices.push_back(-0.5f);

	//down size :
	vertices.push_back(0.5f);
	vertices.push_back(-0.5f);
	vertices.push_back(0.5f);

	vertices.push_back(-0.5f);
	vertices.push_back(-0.5f);
	vertices.push_back(0.5f);

	vertices.push_back(-0.5f);
	vertices.push_back(-0.5f);
	vertices.push_back(-0.5f);

	vertices.push_back(0.5f);
	vertices.push_back(-0.5f);
	vertices.push_back(-0.5f);

	std::vector<unsigned int> index = { 0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 4, 5, 5, 1, 1, 0,
		1, 5, 5, 6, 6, 2, 2, 1,
		2, 6, 6, 7, 7, 3, 3, 2,
		3, 7, 7, 4, 4, 0, 0, 3 };
	triangleIndex.insert(triangleIndex.end(), index.begin(), index.end());

	initGl();
}

void OctreeDrawer::initGl()
{
	triangleCount = triangleIndex.size() / 3;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glGenBuffers(1, &vbo_index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndex.size()*sizeof(int), &triangleIndex[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vbo_vertices);
	glEnableVertexAttribArray(VERTICES);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(VERTICES, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	glGenBuffers(1, &vbo_pos);
	glEnableVertexAttribArray(POSITIONS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(POSITIONS, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	glGenBuffers(1, &vbo_size);
	glEnableVertexAttribArray(SIZES);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_size);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(SIZES, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OctreeDrawer::addDrawItems(const std::vector<glm::vec3>& centers, const std::vector<float>& halfSizes)
{
	for (int i = 0; i < std::min(centers.size(), halfSizes.size()); i++)
	{
		positions.push_back(centers[i].x);
		positions.push_back(centers[i].y);
		positions.push_back(centers[i].z);

		sizes.push_back(halfSizes[i] * 2.f);
		sizes.push_back(halfSizes[i] * 2.f);
		sizes.push_back(halfSizes[i] * 2.f);
	}

	updateInstancedVBOs();
}

void OctreeDrawer::updateInstancedVBOs()
{
	if (positions.size() == 0 || sizes.size() == 0)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(float), &positions[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_size);
	glBufferData(GL_ARRAY_BUFFER, sizes.size()*sizeof(float), &sizes[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OctreeDrawer::clear()
{
	positions.clear();
	sizes.clear();
	instanceCount = 0;
}

void OctreeDrawer::render(const glm::mat4 & projection, const glm::mat4 & view)
{
	glm::mat4 VP = projection * view;

	MaterialInstancedUnlit* castedMaterial = static_cast<MaterialInstancedUnlit*>(material.get()); //TODO : changer ça

	castedMaterial->use();
	castedMaterial->setUniform_VP(VP);
	castedMaterial->setUniform_color(glm::vec3(1, 0, 0));

	draw();
}

void OctreeDrawer::draw()
{
	if (positions.size() == 0)
		return;

	int instanceCount = positions.size() / 3.f;

	glBindVertexArray(vao);

	glVertexAttribDivisor(VERTICES, 0);
	glVertexAttribDivisor(POSITIONS, 1);
	glVertexAttribDivisor(SIZES, 1);

	glDrawElementsInstanced(GL_LINE_STRIP, triangleCount * 3, GL_UNSIGNED_INT, (GLvoid*)0, instanceCount);

	glBindVertexArray(0);
}
