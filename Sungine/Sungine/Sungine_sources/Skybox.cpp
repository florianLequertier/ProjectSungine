

#include "Skybox.h"
//forwards :
#include "Factories.h"


Skybox::Skybox() 
	: material( getMaterialFactory().getDefault("defaultSkybox") /*getProgramFactory().get("defaultSkybox"), getCubeTextureFactory().getDefault("default")*/)
	, mesh(GL_TRIANGLES, (Mesh::Vbo_usage::USE_INDEX | Mesh::Vbo_usage::USE_UVS | Mesh::Vbo_usage::USE_VERTICES) )
{
	mesh.vertices = { 1,1,-1,  1,1,1,  1,-1,1,  1,-1,-1,
					-1,1,-1,  -1,1,1,  -1,-1,1,  -1,-1,-1,
					-1,1,1,  1,1,1,  1,-1,1,  -1,-1,1,
				    -1,1,-1,  1,1,-1,  1,-1,-1,  -1,-1,-1,
					 1,1,1, -1,1,1, -1,1,-1, 1,1,-1, 
					-1,-1,-1,  1,-1,-1,  1,-1,1,  -1,-1,1 };

	mesh.normals = { 1,0,0,  1,0,0,  1,0,0,  1,0,0,
					-1,0,0,  -1,0,0,  -1,0,0,  -1,0,0,
					0,0,1,  0,0,1,  0,0,1,  0,0,1,
					0,0,-1,  0,0,-1,  0,0,-1,  0,0,-1,
					0,1,0,  0,1,0,  0,1,0,  0,1,0, 
					0,-1,0,  0,-1,0,  0,-1,0,  0,-1,0 };

	mesh.uvs = { 0.f,2/3.f,  0,1/3.f,  1/4.f, 1/3.f,  1/4.f, 2/3.f,
				3/4.f,2/3.f,  3/4.f,1/3.f, 2/4.f,1/3.f,  2/4.f,2/3.f,
				2/4.f,0.f,  1/4.f,0.f,  1/4.f,1/3.f,  2/4.f,1/3.f,
				1/4.f,2/3.f,   2/4.f,2/3.f,  1/4.f,1.f,   2/4.f,1.f,
				1.f,1/3.f,  3/4.f,1/3.f,  3/4.f,2/3.f,  1.f,2/3.f,
				2/4.f,2/3.f, 1/4.f,2/3.f,  1/4.f,1/3.f,  2/4.f,1/3.f };

	mesh.triangleIndex = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20 };

	mesh.initGl();
}


Skybox::~Skybox()
{
	// TODO : delete mesh from CG memory
}

void Skybox::drawUI()
{
	ResourcePtr<Material> materialQuery = material;
	if (EditorGUI::ResourceField<Material>("skybox material", materialQuery))
	{
		material = materialQuery;
	}
}

void Skybox::render(const glm::mat4& projection, const glm::mat4& view)
{
	glDepthFunc(GL_LEQUAL);

	glm::mat4 vp = projection * glm::mat4(glm::mat3(view));

	MaterialSkybox* castedMat = static_cast<MaterialSkybox*>(material.get());

	int texCount = 0;
	castedMat->use();
	castedMat->pushInternalsToGPU(texCount);

	castedMat->setUniform_VP(vp);

	mesh.draw();

	glDepthFunc(GL_LESS);
}

void Skybox::save(Json::Value & rootComponent) const
{
	material.save(rootComponent);
}

void Skybox::load(const Json::Value & rootComponent)
{
	material.load(rootComponent);
}
