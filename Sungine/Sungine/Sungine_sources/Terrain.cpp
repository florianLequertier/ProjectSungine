

#include "Terrain.h"
//forwards
#include "Application.h"
#include "Factories.h" 
#include "Ray.h"
#include "EditorGUI.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"


GrassField::GrassField() : mass(0.005f), rigidity(0.05f), viscosity(0.003f), lockYPlane(true)
{
	grassTexture = getTextureFactory().getDefault("default");

	float pi_3 = glm::pi<float>() / 3.f;

	//uvs
	for (int i = 0; i < 3; i++)
	{
		//a
		uvs.push_back(0);
		uvs.push_back(1);
		//b
		uvs.push_back(1);
		uvs.push_back(1);
		//c
		uvs.push_back(1);
		uvs.push_back(0);
		//d
		uvs.push_back(0);
		uvs.push_back(0);
	}

	//index
	for (int i = 0; i < 3; i++)
	{
		//a
		triangleIndex.push_back(0 + 4 * i);
		triangleIndex.push_back(1 + 4 * i);
		triangleIndex.push_back(2 + 4 * i);
		//b
		triangleIndex.push_back(0 + 4 * i);
		triangleIndex.push_back(2 + 4 * i);
		triangleIndex.push_back(3 + 4 * i);

	}

	//vertices
	for (int i = 0; i < 3; i++)
	{
		//a
		vertices.push_back(cos(i*pi_3)*(-0.5f));
		vertices.push_back(0.f);
		vertices.push_back(sin(i*pi_3)*(-0.5f));
		//b
		vertices.push_back(cos(i*pi_3)*(0.5f));
		vertices.push_back(0.f);
		vertices.push_back(sin(i*pi_3)*(0.5f));
		//c
		vertices.push_back(cos(i*pi_3)*(0.5f));
		vertices.push_back(1.f);
		vertices.push_back(sin(i*pi_3)*(0.5f));
		//d
		vertices.push_back(cos(i*pi_3)*-(0.5f));
		vertices.push_back(1.f);
		vertices.push_back(sin(i*pi_3)*(-0.5f));
	}

	//normals
	for (int i = 0; i < 3; i++)
	{
		//a 
		normals.push_back(0);
		normals.push_back(1);
		normals.push_back(0);
		//b
		normals.push_back(0);
		normals.push_back(1);
		normals.push_back(0);
		//c 
		normals.push_back(0);
		normals.push_back(1);
		normals.push_back(0);
		//d 
		normals.push_back(0);
		normals.push_back(1);
		normals.push_back(0);
	}

	triangleCount = triangleIndex.size() / 3;

	//for (int i = 0; i < 10; i++)
	//{
	//	positions.push_back(i);
	//	positions.push_back(0);
	//	positions.push_back(i);
	//}

	initGl();
}

GrassField::~GrassField()
{
	if (vbo_index != 0)
		glDeleteBuffers(1, &vbo_index);

	if (vbo_vertices != 0)
		glDeleteBuffers(1, &vbo_vertices);

	if (vbo_uvs != 0)
		glDeleteBuffers(1, &vbo_uvs);

	if (vbo_normals != 0)
		glDeleteBuffers(1, &vbo_normals);

	if (vbo_pos != 0)
		glDeleteBuffers(1, &vbo_pos);

	if (vbo_animPos != 0)
		glDeleteBuffers(1, &vbo_pos);

	glDeleteVertexArrays(1, &vao);
}

//initialize vbos and vao, based on the informations of the mesh.
void GrassField::initGl()
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


	glGenBuffers(1, &vbo_normals);
	glEnableVertexAttribArray(NORMALS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(float), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(NORMALS, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);


	glGenBuffers(1, &vbo_uvs);
	glEnableVertexAttribArray(UVS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
	glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(float), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(UVS, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 2, (void*)0);


	glGenBuffers(1, &vbo_pos);
	glEnableVertexAttribArray(POSITIONS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glVertexAttribPointer(POSITIONS, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	//for physic simulation : 
	glGenBuffers(1, &vbo_animPos);
	glEnableVertexAttribArray(ANIM_POS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_animPos);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);
	glVertexAttribPointer(ANIM_POS, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GrassField::freeGl()
{
	glDeleteBuffers(1, &vbo_index);
	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &vbo_uvs);
	glDeleteBuffers(1, &vbo_normals);
	glDeleteBuffers(1, &vbo_animPos);
	glDeleteBuffers(1, &vbo_pos);
	glDeleteVertexArrays(1, &vao);
}

void GrassField::clear()
{
	freeGl();

	grassTexture = getTextureFactory().getDefault("default");

	triangleIndex.clear();
	vertices.clear();
	normals.clear();
	uvs.clear();
	positions.clear();
	grassKeys.clear();
	offsets.clear();
	forces.clear();
	speeds.clear();
	links.clear();
}

void GrassField::addGrass(GrassKey grassKey, const glm::vec3 & position)
{
	int currentIdx = grassKeys.size();
	grassKeys.push_back(grassKey);
	//positions : 
	positions.push_back(position.x);
	positions.push_back(position.y);
	positions.push_back(position.z);
	//offsets : 
	offsets.push_back(0);
	offsets.push_back(0);
	offsets.push_back(0);
	//forces : 
	forces.push_back(0);
	forces.push_back(0);
	forces.push_back(0);
	//speeds : 
	speeds.push_back(0);
	speeds.push_back(0);
	speeds.push_back(0);
	//links : 
	links.push_back(GrassPhysicLink(currentIdx, glm::vec3(0,0,0), 0.5f));

	updateVBOPositions();
	updateVBOAnimPos();
}

void GrassField::remove(GrassKey grassKey)
{
	auto findIt = std::find(grassKeys.begin(), grassKeys.end(), grassKey);
	if (findIt == grassKeys.end())
		return;

	int idx = findIt - grassKeys.begin();

	//keys : 
	grassKeys.erase(findIt);
	//positions : 
	positions.erase(positions.begin() + idx*3);
	positions.erase(positions.begin() + (idx*3 + 1));
	positions.erase(positions.begin() + (idx*3 + 2));
	//offsets :
	offsets.erase(offsets.begin() + idx*3);
	offsets.erase(offsets.begin() + (idx*3 + 1));
	offsets.erase(offsets.begin() + (idx*3 + 2));
	//forces : 
	forces.erase(forces.begin() + idx*3);
	forces.erase(forces.begin() + (idx*3 + 1));
	forces.erase(forces.begin() + (idx*3 + 2));
	//speeds : 
	speeds.erase(speeds.begin() + idx*3);
	speeds.erase(speeds.begin() + (idx*3 + 1));
	speeds.erase(speeds.begin() + (idx*3 + 2));
	//links : 
	links.erase(links.begin() + idx);

	updateVBOPositions();
	updateVBOAnimPos();
}

void GrassField::draw()
{
	if (positions.size() == 0)
		return;

	int instanceCount = positions.size()/3.f;

	glBindVertexArray(vao);

	glVertexAttribDivisor(VERTICES, 0);
	glVertexAttribDivisor(NORMALS, 0);
	glVertexAttribDivisor(UVS, 0);
	glVertexAttribDivisor(POSITIONS, 1);
	glVertexAttribDivisor(ANIM_POS, 1);

	glDrawElementsInstanced(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_INT, (GLvoid*)0, instanceCount);

	glBindVertexArray(0);
}

void GrassField::render(const glm::mat4 & projection, const glm::mat4 & view)
{
	glm::mat4 VP = projection * view;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTexture->glId);

	materialGrassField.use();
	materialGrassField.setUniformTime(0); //TODO : ADD TIME
	materialGrassField.setUniformTexture(0);
	materialGrassField.setUniformVP(VP);

	draw();
}

void GrassField::updatePhysic(float deltaTime, std::vector<Physic::WindZone*>& windZones)
{
	//apply forces : 
	glm::vec3 newForce;
	for (auto& windZone : windZones)
	{
		for (int i = 0; i < forces.size()/3; i++)
		{
			newForce = windZone->getForce(Application::get().getTime(), vertexFrom3Floats(positions, i));
			forces[i * 3] += newForce.x;
			if(!lockYPlane)
				forces[i * 3 + 1] += newForce.y;
			forces[i * 3 + 2] += newForce.z;
		}
	}

	for (int linkIdx = 0; linkIdx < links.size(); linkIdx++)
	{
		computeLink(deltaTime, linkIdx);
	}
	//update position based on forces :
	for (int pointIdx = 0; pointIdx < offsets.size()/3; pointIdx++)
	{
		computePoint(deltaTime, pointIdx);
	}
	//update vbos : 
	updateVBOAnimPos();
}

void GrassField::computePoint(float deltaTime, int index)
{
	if (mass < 0.00000001f)
		return;

	//leapfrog
	speeds[index * 3] += (deltaTime / mass)*forces[index * 3];
	speeds[index * 3 + 1] += (deltaTime / mass)*forces[index * 3 + 1];
	speeds[index * 3 + 2]+= (deltaTime / mass)*forces[index * 3 + 2];

	offsets[index * 3] += deltaTime*speeds[index * 3];
	offsets[index * 3 + 1] += deltaTime*speeds[index * 3 + 1];
	offsets[index * 3 + 2] += deltaTime*speeds[index * 3 + 2];

	forces[index * 3] = 0;
	forces[index * 3 + 1] = 0;
	forces[index * 3 + 2] = 0;
}

void GrassField::computeLink(float deltaTime, int index)
{
	glm::vec3 p1 = vertexFrom3Floats(offsets, links[index].p1_idx);
	glm::vec3 p2 = links[index].p2_pos;
	glm::vec3 v1 = vertexFrom3Floats(speeds, links[index].p1_idx);

	float d = glm::distance(p1, p2);
	if (d < 0.00000001f)
		return;

	float f = rigidity * (1.f - links[index].l / (d));
	if (std::abs(f) < 0.00000001f)
		return;

	glm::vec3 M1M2 = p2 - p1;
	glm::normalize(M1M2);
	if (glm::length(M1M2) < 0.00000001f)
		return;

	//frein :
	glm::vec3 frein = viscosity*(-v1);

	glm::vec3 force = (f * M1M2 + frein);
	forces[links[index].p1_idx * 3] += force.x;
	forces[links[index].p1_idx * 3 + 1] += force.y;
	forces[links[index].p1_idx * 3 + 2] += force.z;
}

void GrassField::resetPhysic()
{
	for (int i = 0; i < offsets.size(); i++)
	{
		offsets[i] = 0;
		forces[i] = 0;
		speeds[i] = 0;
	}
}

void GrassField::setViscosity(float _viscosity)
{
	viscosity = _viscosity;
}

void GrassField::setRigidity(float _rigidity)
{
	rigidity = _rigidity;
}

void GrassField::setMass(float _mass)
{
	mass = _mass;
}

float GrassField::getViscosity() const
{
	return viscosity;
}

float GrassField::getRigidity() const
{
	return rigidity;
}

float GrassField::getMass() const
{
	return mass;
}

void GrassField::drawUI()
{
	ImGui::InputFloat("rigidity", &rigidity);
	ImGui::InputFloat("viscosity", &viscosity);
	ImGui::InputFloat("mass", &mass);

	if(ImGui::RadioButton("lock Y plane", &lockYPlane))
		lockYPlane = !lockYPlane;

	if (ImGui::Button("reset physic"))
	{
		resetPhysic();
	}
}

void GrassField::save(Json::Value & rootComponent) const
{
	//TODO
}

void GrassField::load(const Json::Value & rootComponent)
{
	//TODO
}

void GrassField::updateVBOPositions()
{
	if (positions.size() <= 0)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(float), &positions[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GrassField::updateVBOAnimPos()
{
	if (offsets.size() <= 0)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, vbo_animPos);
	glBufferData(GL_ARRAY_BUFFER, offsets.size()*sizeof(float), &offsets[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


////////////////// TERRAIN ///////////////////

Terrain::Terrain(float width, float height, float depth, int subdivision, glm::vec3 offset) 
		   : m_width(width), m_height(height), m_depth(depth), m_subdivision(subdivision), m_offset(offset), //terrain properties
			m_noiseMin(0.f), m_noiseMax(1.f), m_seed(0), m_terrainNoise(512, 64, 3, 0.5f, 0), //perlin properties
			m_currentMaterialToDrawIdx(-1), m_drawRadius(1), //draw material properties
			m_maxGrassDensity(1.f), m_grassDensity(0), m_grassLayoutDelta(0.3f), //draw grass properties
			m_terrainFbo(0), m_materialLayoutsFBO(0),//fbos
			m_material(*getProgramFactory().getDefault("terrain")), m_terrainMaterial(*getProgramFactory().getDefault("terrainEdition")), m_drawOnTextureMaterial(*getProgramFactory().getDefault("drawOnTexture")), //matertials
			m_quadMesh(GL_TRIANGLES, (Mesh::USE_INDEX | Mesh::USE_VERTICES), 2) , // mesh
			m_noiseTexture(1024, 1024, glm::vec4(0.f,0.f,0.f,255.f)), m_terrainDiffuse(1024, 1024), //textures
			m_terrainBump(1024, 1024), m_terrainSpecular(1024, 1024), m_drawMatTexture(1024, 1024),
			m_terrainCollider(nullptr), m_terrainRigidbody(nullptr), m_ptrToPhysicWorld(nullptr), m_triangleIndexVertexArray(nullptr), //physic
			m_aabbMin(-1000, -1000, -1000), m_aabbMax(1000, 1000, 1000) //aabb
{
	//filter texture initialisation : 
	m_filterTexture = new Texture(1024, 1024);

	// initialyze the texture name : 
	//m_newLayoutName[0] = '\0';
	//m_newGrassTextureName[0] = '\0';

	//grass layout initialization : 
	m_grassLayoutWidth = m_width / (float)m_grassLayoutDelta;
	m_grassLayoutDepth = m_depth / (float)m_grassLayoutDelta;
	for (int i = 0; i < m_grassLayoutWidth*m_grassLayoutDepth; i++)
		m_grassLayout.push_back(0);

	//push terrain texture to GPU
	//bump
	//m_terrainBump.name = "terrainTextureBump";
	m_terrainBump.internalFormat = GL_RGBA16;
	m_terrainBump.format = GL_RGBA;
	m_terrainBump.type = GL_FLOAT;
	m_terrainBump.textureWrapping_u = GL_CLAMP_TO_EDGE;
	m_terrainBump.textureWrapping_v = GL_CLAMP_TO_EDGE;
	m_terrainBump.minFilter = GL_LINEAR;
	m_terrainBump.magFilter = GL_LINEAR;
	m_terrainBump.generateMipMap = false;
	m_terrainBump.initGL();
	//specular
	//m_terrainSpecular.name = "terrainTextureSpecular";
	m_terrainSpecular.internalFormat = GL_RGBA8;
	m_terrainSpecular.format = GL_RGBA;
	m_terrainSpecular.type = GL_UNSIGNED_BYTE;
	m_terrainSpecular.textureWrapping_u = GL_CLAMP_TO_EDGE;
	m_terrainSpecular.textureWrapping_v = GL_CLAMP_TO_EDGE;
	m_terrainSpecular.minFilter = GL_LINEAR;
	m_terrainSpecular.magFilter = GL_LINEAR;
	m_terrainSpecular.generateMipMap = false;
	m_terrainSpecular.initGL();
	//diffuse
	//m_terrainDiffuse.name = "terrainTextureDiffuse";
	m_terrainDiffuse.internalFormat = GL_RGBA8;
	m_terrainDiffuse.format = GL_RGBA;
	m_terrainDiffuse.type = GL_UNSIGNED_BYTE;
	m_terrainDiffuse.textureWrapping_u = GL_CLAMP_TO_EDGE;
	m_terrainDiffuse.textureWrapping_v = GL_CLAMP_TO_EDGE;
	m_terrainDiffuse.minFilter = GL_LINEAR;
	m_terrainDiffuse.magFilter = GL_LINEAR;
	m_terrainDiffuse.generateMipMap = false;
	m_terrainDiffuse.initGL();

	//depth
	//m_terrainDepth.name = "terrainTextureDepth";
	//m_terrainDepth.internalFormat = GL_DEPTH_COMPONENT24;
	//m_terrainDepth.format = GL_DEPTH_COMPONENT;
	//m_terrainDepth.type = GL_FLOAT;
	//m_terrainDepth.generateMipMap = false;
	//m_terrainDepth.initGL ();
	//noise and filter
	m_noiseTexture.initGL();

	m_filterTexture->internalFormat = GL_RGBA16;
	m_filterTexture->format = GL_RGBA;
	m_filterTexture->type = GL_FLOAT;
	m_filterTexture->generateMipMap = false;
	m_filterTexture->initGL();
	//draw texture
	//m_drawMatTexture.name = "texture draw text";
	m_drawMatTexture.initGL();

	//set the terrain texture : 
	//bump
	//m_material.textureBump = &m_terrainBump;
	//m_material.setInternalData<Texture>("BumpTexture", &m_terrainBump);
	//m_material.bumpTextureName = m_terrainBump.getName();
	//specular
	//m_material.setInternalData<Texture>("SpecularTexture", &m_terrainSpecular);
	//m_material.specularTextureName = m_terrainSpecular.name;
	//diffuse
	//m_material.setInternalData<Texture>("DiffuseTexture", &m_terrainDiffuse);
	//m_material.diffuseTextureName = m_terrainDiffuse.name;

	////////////////////// INIT QUAD MESH ////////////////////////
	m_quadMesh.triangleIndex = { 0, 1, 2, 2, 1, 3 };
	m_quadMesh.vertices = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0 };
	m_quadMesh.initGl();

	m_terrainNoise.setSeed(m_seed);

	generateTerrain();
	applyNoise(m_terrainNoise, false);


	//generate material layout FBO : 
	glGenFramebuffers(1, &m_materialLayoutsFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_materialLayoutsFBO);

	GLuint matLayoutDrawBuffers[1];
	matLayoutDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, matLayoutDrawBuffers);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_filterTexture->glId, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Error on building framebuffer\n");
		exit(EXIT_FAILURE);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	//generate terrain FBO : 
	glGenFramebuffers(1, &m_terrainFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_terrainFbo);

	GLuint terrainDrawBuffers[3];
	terrainDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
	terrainDrawBuffers[1] = GL_COLOR_ATTACHMENT1;
	terrainDrawBuffers[2] = GL_COLOR_ATTACHMENT2;
	glDrawBuffers(3, terrainDrawBuffers);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_terrainDiffuse.glId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_terrainBump.glId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_terrainSpecular.glId, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_terrainDepth.glId, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Error on building framebuffer\n");
		exit(EXIT_FAILURE);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


Terrain::~Terrain()
{
	clear();
}

void Terrain::drawMaterialOnTerrain(glm::vec3 position, float radius, int textureIdx)
{
	assert(textureIdx >= 0 && textureIdx < m_terrainLayouts.size());
	float greyValue = (textureIdx + 0.25f) / (float)m_terrainLayouts.size();

	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, m_materialLayoutsFBO);
	//we don't want to clear the texture attached to the framebuffer

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_filterTexture->glId); //we read and write on the same texture

	m_drawOnTextureMaterial.use();
	m_drawOnTextureMaterial.setUniformColorToDraw(glm::vec4(greyValue, greyValue, greyValue,1));
	m_drawOnTextureMaterial.setUniformDrawPosition(glm::vec2((position.x + (m_width*0.5f/(float)m_subdivision) - m_width*0.5f) / (float)(m_width*.5f), (position.z + (m_depth*0.5f/ (float)m_subdivision) - m_depth*0.5f) / (float)(m_depth*.5f))); //position in tex coords, between 0 and 1
	m_drawOnTextureMaterial.setUniformDrawRadius(radius);
	m_drawOnTextureMaterial.setUniformTextureToDrawOn(0);

	m_quadMesh.draw();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//generateTerrainTexture();
}

void Terrain::drawMaterialOnTerrain(glm::vec3 position)
{
	if (m_currentMaterialToDrawIdx < 0 || m_currentMaterialToDrawIdx >= m_terrainLayouts.size())
		return;
	drawMaterialOnTerrain(position, m_drawRadius, m_currentMaterialToDrawIdx);
}

bool Terrain::isIntersectedByRay(const Ray & ray, CollisionInfo & collisionInfo) const
{
	bool isColliding = false;

	for (int i = 0; i < m_triangleIndex.size()-2; i++)
	{
		glm::vec3 a = vertexFrom3Floats(m_vertices, m_triangleIndex[i]);
		glm::vec3 c = vertexFrom3Floats(m_vertices, m_triangleIndex[i + 1]);
		glm::vec3 b = vertexFrom3Floats(m_vertices, m_triangleIndex[i + 2]);
		isColliding = ray.intersectTriangle(a, b, c, collisionInfo);
		if (isColliding)
			return true;
	}
	return false;
}

void Terrain::generateTerrainTexture() 
{
	//m_material.textureRepetition = glm::vec2(1.f / (float)m_subdivision, 1.f / (float)m_subdivision);

	
	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, m_terrainFbo);

	glClear(GL_COLOR_BUFFER_BIT);

	m_terrainMaterial.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_filterTexture->glId);

	for (int i = 0; i < m_terrainLayouts.size(); i++)
	{
		MaterialLit* castedterrainLayout = static_cast<MaterialLit*>(m_terrainLayouts[i].get()); //TODO : changer ça

		//diffuse
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, castedterrainLayout->getInternalData<Texture>("TextureDiffuse")->glId);//->getDiffuse()->glId);
		//bump
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, castedterrainLayout->getInternalData<Texture>("TextureBump")->glId);//->getBump()->glId);
		//specular
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, castedterrainLayout->getInternalData<Texture>("TextureSpecular")->glId);//->getSpecular()->glId);

		//filter texture : 
		m_terrainMaterial.setUniformFilterTexture(0);
		//diffuse texture :
		m_terrainMaterial.setUniformDiffuseTexture(1);
		//bump texture : 
		m_terrainMaterial.setUniformBumpTexture(2);
		//specular texture : 
		m_terrainMaterial.setUniformSpecularTexture(3);


		float offsetMin = (i / (float)m_terrainLayouts.size());
		float offsetMax = ((i + 1) / (float)m_terrainLayouts.size());
		m_terrainMaterial.setUniformLayoutOffset( glm::vec2( offsetMin, offsetMax ) );
		m_terrainMaterial.setUniformTextureRepetition(m_textureRepetitions[i]);

		m_quadMesh.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//UPDATE MIPMAP ???

	//glBindTexture(GL_TEXTURE_2D, m_terrainBump.glId);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glBindTexture(GL_TEXTURE_2D, m_terrainDiffuse.glId);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glBindTexture(GL_TEXTURE_2D, m_terrainSpecular.glId);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	
}


void Terrain::computeNoiseTexture(Perlin2D& perlin2D)
{
	//redraw the noise texture
	m_noiseTexture.freeGL();
	m_filterTexture->freeGL();

	for (int j = 0, k = 0; j < 1024; j++)
	{
		for (int i = 0; i < 1024; i++, k+=3)
		{
			float x = (i * m_subdivision) / 1024.f;
			float y = (j * m_subdivision) / 1024.f;


			float noiseValue = perlin2D.getNoiseValue(x, y);

			for (int p = 0; p < 3; p++)
			{
				m_noiseTexture.pixels[k + p] = ((noiseValue - m_noiseMin) / (m_noiseMax - m_noiseMin)) * 255;
				m_filterTexture->pixels[k + p] = ((noiseValue - m_noiseMin) / (m_noiseMax - m_noiseMin)) * 255;
			}
		}
	}

	m_noiseTexture.initGL();
	m_filterTexture->initGL();
}

void Terrain::computeNormals()
{
	glm::vec3 u(1, 0, 0);
	glm::vec3 v(0, 0, 1);
	glm::vec3 normal(0, 0, 0);
	glm::vec3 tangent(0, 0, 0);

	for (int j = 0, k = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++, k+=3)
		{
			if (i > 0 && i < m_subdivision - 1 && j > 0 && j < m_subdivision - 1)
			{
				if (j - 1 >= 0 && i - 1 >= 0)
				{
					u = vertexFrom3Floats(m_vertices, i + (j - 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_vertices, (i - 1)+ j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
					normal += glm::normalize(glm::cross(u, v));
				}

				if (i - 1 >= 0 && j + 1 < (m_subdivision ))
				{
					u = vertexFrom3Floats(m_vertices, (i - 1) + j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_vertices, i + (j + 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
					normal += glm::normalize(glm::cross(u, v));
				}

				if (j + 1 < (m_subdivision ) && i+1 < (m_subdivision ))
				{
					u = vertexFrom3Floats(m_vertices, i + (j + 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_vertices, (i + 1) + j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
					normal += glm::normalize(glm::cross(u, v));
				}

				if (i + 1 < (m_subdivision ) && j - 1 >= 0)
				{
					u = vertexFrom3Floats(m_vertices, (i + 1) + j * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);
					v = vertexFrom3Floats(m_vertices, i + (j - 1) * m_subdivision) - vertexFrom3Floats(m_vertices, i + j * m_subdivision);

					normal += glm::normalize(glm::cross(u, v));
				}

				normal = glm::normalize(normal);
			}
			else
				normal = glm::vec3(0, 1, 0);


			m_normals[k] = normal.x;
			m_normals[k+1] = normal.y;
			m_normals[k+2] = normal.z;

			tangent = glm::normalize(glm::cross(normal, u));

			m_tangents[k] = tangent.x;
			m_tangents[k+1] = tangent.y;
			m_tangents[k+2] = tangent.z;
		}
	}


	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size()*sizeof(float), &m_normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_tangents);
	glBufferData(GL_ARRAY_BUFFER, m_tangents.size()*sizeof(float), &m_tangents[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::applyNoise(Perlin2D& perlin2D, bool _computeNoiseTexture)
{
	//init aabb :
	m_aabbMin = m_vertices.size() > 0 ? glm::vec3(m_vertices[0], m_vertices[1], m_vertices[2]) : glm::vec3(0, 0, 0);
	m_aabbMax = m_vertices.size() > 0 ? glm::vec3(m_vertices[0], m_vertices[1], m_vertices[2]) : glm::vec3(0, 0, 0);

	m_noiseMin = 1.f;
	m_noiseMax = 0.f;

	float deltaWidth = m_width / (float)m_subdivision;
	float deltaDepth = m_depth / (float)m_subdivision;

	for (int j = 0, k = 1, l = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++, k += 3, l++)
		{
			float noiseValue = perlin2D.getNoiseValue(i*deltaWidth, j*deltaDepth);

			if (noiseValue < m_noiseMin)
				m_noiseMin = noiseValue;
			if (noiseValue > m_noiseMax)
				m_noiseMax = noiseValue;

			m_heightMap[l] = noiseValue * 2.f - 1.f;

			m_vertices[k] = m_heightMap[l] * m_height + m_offset.y;

			//update aabb :
			if (m_vertices[k - 1] < m_aabbMin.x) m_aabbMin.x = m_vertices[k - 1];
			else if(m_vertices[k - 1] > m_aabbMax.x) m_aabbMax.x = m_vertices[k - 1];
			if (m_vertices[k] < m_aabbMin.y) m_aabbMin.y = m_vertices[k];
			else if (m_vertices[k] > m_aabbMax.y) m_aabbMax.y = m_vertices[k];
			if (m_vertices[k + 1] < m_aabbMin.z) m_aabbMin.z = m_vertices[k + 1];
			else if (m_vertices[k + 1] > m_aabbMax.z) m_aabbMax.z = m_vertices[k + 1];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size()*sizeof(float), &m_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	computeNormals();

	//refresh the terrain texture : 
	if (_computeNoiseTexture)
	{
		computeNoiseTexture(perlin2D);
		generateTerrainTexture();
	}

	//init physics : 
	updateCollider();
	
}

void Terrain::generateTerrain()
{
	if (m_subdivision == 0)
		return;

	float paddingZ = m_depth / (float)m_subdivision;
	float paddingX = m_width / (float)m_subdivision;

	int lineCount = (m_subdivision - 1);
	int rowCount = (m_subdivision - 1);
	m_triangleCount = (m_subdivision - 1) * (m_subdivision - 1) * 2 + 1;

	m_vertices.clear();
	m_normals.clear();
	m_uvs.clear();
	m_triangleIndex.clear();
	m_heightMap.clear();
	m_tangents.clear();

	for (int j = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++)
		{
			m_heightMap.push_back(0);

			m_vertices.push_back(i*paddingX + m_offset.x);
			m_vertices.push_back(m_offset.y);
			m_vertices.push_back(j*paddingZ + m_offset.z);

			m_normals.push_back(0);
			m_normals.push_back(1);
			m_normals.push_back(0);

			m_tangents.push_back(1);
			m_tangents.push_back(0);
			m_tangents.push_back(0);

			m_uvs.push_back(i / (float)(m_subdivision - 1));
			m_uvs.push_back(j / (float)(m_subdivision - 1));
		}
	}

	for (int i = 0, k = 0; i < m_triangleCount; i++)
	{
		if (i % 2 == 0)
		{
			m_triangleIndex.push_back(k + 0);
			m_triangleIndex.push_back(k + 1);
			m_triangleIndex.push_back(k + m_subdivision);
		}
		else
		{
			m_triangleIndex.push_back(k + 1);
			m_triangleIndex.push_back(k + m_subdivision + 1);
			m_triangleIndex.push_back(k + m_subdivision);
		}

		if (i % 2 == 0 && i != 0)
			k++;

		if ((k + 1) % (m_subdivision) == 0 && i != 0)
		{
			k++;
		}
	}

	//update flat aabb :
	m_aabbMin = m_offset - glm::vec3(0,-0.1,0);
	m_aabbMax = m_offset + glm::vec3((m_subdivision - 1)*paddingX, 0.1, (m_subdivision - 1)*paddingZ);

	//init graphics :
	initGl();
	//init physics : 
	updateCollider();
}

void Terrain::generateCollider()
{
	if (m_terrainCollider != nullptr)
		delete m_terrainCollider;

	if (m_triangleIndexVertexArray != nullptr)
		delete m_triangleIndexVertexArray;

	//generate the new triangleIndexVertexArray :
	m_triangleIndexVertexArray = new btTriangleIndexVertexArray(m_triangleIndex.size() / 3, &m_triangleIndex[0], 3 * sizeof(int), m_vertices.size() / 3.f, (btScalar*)&m_vertices[0], 3 * sizeof(float));
	//generate the new terrainCollider :
	float aabbOffset = 5;
	m_terrainCollider = new btBvhTriangleMeshShape(m_triangleIndexVertexArray, true, btVector3(m_aabbMin.x - aabbOffset, m_aabbMin.y - aabbOffset, m_aabbMin.z - aabbOffset),
																								btVector3(m_aabbMax.x + aabbOffset, m_aabbMax.y + aabbOffset, m_aabbMax.z + aabbOffset));
}

void Terrain::updateCollider()
{
	if (m_ptrToPhysicWorld == nullptr || m_terrainRigidbody == nullptr) 
		return;

	//pop from simulation :
	if (m_terrainRigidbody->isInWorld())
	{
		m_ptrToPhysicWorld->removeRigidBody(m_terrainRigidbody);
	}

	m_terrainRigidbody->setCollisionShape(nullptr);
	
	//make a new collider for this terrain, removing the older one :
	generateCollider();

	m_terrainRigidbody->setCollisionShape(m_terrainCollider);

	//push to simulation :
	m_ptrToPhysicWorld->addRigidBody(m_terrainRigidbody);
}

btBvhTriangleMeshShape * Terrain::getColliderShape() const
{
	return m_terrainCollider;
}

void Terrain::updateTerrain()
{
	//init aabb :
	m_aabbMin = m_vertices.size() > 0 ? glm::vec3(m_vertices[0], m_vertices[1], m_vertices[2]) : glm::vec3(0, 0, 0);
	m_aabbMax = m_vertices.size() > 0 ? glm::vec3(m_vertices[0], m_vertices[1], m_vertices[2]) : glm::vec3(0, 0, 0);

	float paddingZ = m_depth / (float)m_subdivision;
	float paddingX = m_width / (float)m_subdivision;

	for (int j = 0, k = 0, l = 0; j < m_subdivision; j++)
	{
		for (int i = 0; i < m_subdivision; i++, k += 3, l++)
		{
			m_vertices[k] = i*paddingX + m_offset.x;
			m_vertices[k+1] = m_heightMap[l] * m_height + m_offset.y;
			m_vertices[k+2] = j*paddingZ + m_offset.z;

			//update aabb :
			if (m_vertices[k] < m_aabbMin.x) m_aabbMin.x = m_vertices[k];
			else if (m_vertices[k] > m_aabbMax.x) m_aabbMax.x = m_vertices[k];
			if (m_vertices[k + 1] < m_aabbMin.y) m_aabbMin.y = m_vertices[k + 1];
			else if (m_vertices[k + 1] > m_aabbMax.y) m_aabbMax.y = m_vertices[k + 1];
			if (m_vertices[k + 2] < m_aabbMin.z) m_aabbMin.z = m_vertices[k + 2];
			else if (m_vertices[k + 2] > m_aabbMax.z) m_aabbMax.z = m_vertices[k + 2];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size()*sizeof(float), &m_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//update grass layout size : 
	int grassLayoutOldWidth = m_grassLayoutWidth;
	int grassLayoutOldDepth = m_grassLayoutDepth;
	m_grassLayoutWidth = m_width / (float)m_grassLayoutDelta;
	m_grassLayoutDepth = m_depth / (float)m_grassLayoutDelta;
	resize2DArray<int>(m_grassLayout, grassLayoutOldWidth, grassLayoutOldDepth, m_grassLayoutWidth, m_grassLayoutDepth);
	//TODO resize grass field

	//init physics : 
	updateCollider();
}

//initialize vbos and vao, based on the informations of the mesh.
void Terrain::initGl()
{

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glGenBuffers(1, &vbo_index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangleIndex.size()*sizeof(int), &m_triangleIndex[0], GL_STATIC_DRAW);


	glGenBuffers(1, &vbo_vertices);
	glEnableVertexAttribArray(VERTICES);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size()*sizeof(float), &m_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(VERTICES, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);



	glGenBuffers(1, &vbo_normals);
	glEnableVertexAttribArray(NORMALS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size()*sizeof(float), &m_normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(NORMALS, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);

	glGenBuffers(1, &vbo_tangents);
	glEnableVertexAttribArray(TANGENTS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tangents);
	glBufferData(GL_ARRAY_BUFFER, m_tangents.size()*sizeof(float), &m_tangents[0], GL_STATIC_DRAW);
	glVertexAttribPointer(TANGENTS, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 3, (void*)0);


	glGenBuffers(1, &vbo_uvs);
	glEnableVertexAttribArray(UVS);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
	glBufferData(GL_ARRAY_BUFFER, m_uvs.size()*sizeof(float), &m_uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(UVS, 2, GL_FLOAT , GL_FALSE, sizeof(GL_FLOAT) * 2, (void*)0);


	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::freeGl()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo_index);
	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &vbo_uvs);
	glDeleteBuffers(1, &vbo_normals);
	glDeleteBuffers(1, &vbo_tangents);

	//TODO 10
	//m_material.textureDiffuse = nullptr; // detach texture as the texture is inside the terrain and will be destroyed

	glDeleteFramebuffers(1, &m_terrainFbo);

	m_terrainDiffuse.freeGL();
	m_terrainBump.freeGL();
	m_terrainSpecular.freeGL();
	m_noiseTexture.freeGL();
	m_filterTexture->freeGL();
}

void Terrain::clear()
{
	//graphics :
	freeGl();

	m_triangleIndex.clear();
	m_uvs.clear();
	m_vertices.clear();
	m_normals.clear();
	m_tangents.clear();

	m_heightMap.clear();
	m_terrainLayouts.clear();
	m_textureRepetitions.clear();
	m_grassLayout.clear();
	
	//grassfield :
	m_grassField.clear();

	//physic :
	if (m_ptrToPhysicWorld != nullptr && m_terrainRigidbody != nullptr) {
		m_ptrToPhysicWorld->removeRigidBody(m_terrainRigidbody);
	}
	if (m_terrainRigidbody != nullptr) {
		m_terrainRigidbody->setCollisionShape(nullptr);
	}
	if (m_terrainCollider != nullptr) {
		delete m_terrainCollider;
		m_terrainCollider = nullptr;
	}
	if (m_terrainRigidbody != nullptr) {
		delete m_terrainRigidbody;
		m_terrainRigidbody = nullptr;
	}
	m_ptrToPhysicWorld = nullptr;
	if (m_triangleIndexVertexArray != nullptr) {
		delete m_triangleIndexVertexArray;
		m_triangleIndexVertexArray = nullptr;
	}
}

void Terrain::initPhysics(btDiscreteDynamicsWorld* physicWorld)
{
	//set ptr to bullet physic simulation :
	m_ptrToPhysicWorld = physicWorld;
	//generate collider : 
	generateCollider();
	//generate terrain rigidbody :
	m_terrainRigidbody = new btRigidBody(0, nullptr, m_terrainCollider);
	//add the terrain rigidbody to the simulation : 
	m_ptrToPhysicWorld->addRigidBody(m_terrainRigidbody);
}

void Terrain::drawGrassOnTerrain(const glm::vec3 position)
{
	drawGrassOnTerrain(position, m_drawRadius * m_width*0.5f, m_grassDensity, m_maxGrassDensity);
}

void Terrain::drawGrassOnTerrain(const glm::vec3 position, float radius, float density, float maxDensity)
{

	int px = position.x / m_grassLayoutDelta;
	int pz = position.z / m_grassLayoutDelta;
	glm::vec2 p(px, pz);

	int maxDensityMinusDensity = maxDensity - density;
	int grassLayoutWidth = m_width / m_grassLayoutDelta;
	int grassLayoutDepth = m_depth / m_grassLayoutDelta;

	std::vector<glm::vec2> potentialPositionIndex;
	int currentGrassCount = 0;

	for (int j = std::max(0, (int)(-radius + pz)); j <= std::min((int)(radius + pz), grassLayoutDepth - 1); j++)
	{
		for (int i = std::max(0, (int)(-radius + px)); i <= std::min( (int)(radius + px), grassLayoutWidth - 1); i++)
		{
			if (glm::length(glm::vec2(i, j) - p) <= radius)
			{
				if (m_grassLayout[j*grassLayoutWidth + i] == 0)
					potentialPositionIndex.push_back(glm::vec2(i, j));
				else
					currentGrassCount++;
			}
		}
	}
	

	if(potentialPositionIndex.size() > 0)
	for (int i = 0; i < (density - currentGrassCount/(float)(4.f*radius*radius))*10; i++)
	{
		std::cout << "grass count" << (density - currentGrassCount / (float)(4.f*radius*radius)) << std::endl;

		//srand(time(nullptr));
		int randNumber = rand();
		std::cout << "random number" << randNumber << std::endl;
		int randomIndex = randNumber % potentialPositionIndex.size();
		std::cout << "random index" << randomIndex << std::endl;
		glm::vec2 pointIndex = potentialPositionIndex[randomIndex];

		float posX = pointIndex.x * m_grassLayoutDelta;
		float posZ = pointIndex.y * m_grassLayoutDelta;
		float posY = getHeight(posX, posZ);

		if (m_grassLayout[grassLayoutWidth*pointIndex.y + pointIndex.x] == 0)
		{
			m_grassField.addGrass(GrassKey(pointIndex.x, pointIndex.y), glm::vec3(posX, posY, posZ));
			m_grassLayout[grassLayoutWidth*pointIndex.y + pointIndex.x] = 1; //this layout controls the density of the grassField.

			potentialPositionIndex.erase(potentialPositionIndex.begin() + randomIndex);
		}
	}
}

void Terrain::updateGrassPositions()
{
	for (int i = 1; i < m_grassField.positions.size(); i+=3)
	{
		//update height : 
		float posY = getHeight(m_grassField.positions[i-1], m_grassField.positions[i+1]);
		m_grassField.positions[i] = posY;
	}
	m_grassField.updateVBOPositions();
}

Terrain::TerrainTools Terrain::getCurrentTerrainTool() const
{
	return m_currentTerrainTool;
}

void Terrain::updatePhysic(float deltaTime, std::vector<Physic::WindZone*>& windZones)
{
	//apply physic on grassField : 
	m_grassField.updatePhysic(deltaTime, windZones);
	//TODO : trees,...
}

void Terrain::save(Json::Value & rootComponent) const
{
	//parameters : 
	rootComponent["subdivision"] = m_subdivision;
	rootComponent["width"] = m_width;
	rootComponent["depth"] = m_depth;
	rootComponent["height"] = m_height;
	rootComponent["offset"] = toJsonValue<glm::vec3>(m_offset);
	
	//noise :
	rootComponent["seed"] = m_seed;
	m_terrainNoise.save(rootComponent["terrainNoise"]);

	//materials :
	rootComponent["materialLayoutCount"] = m_terrainLayouts.size();
	for (int i = 0; i < m_terrainLayouts.size(); i++) {
		rootComponent["materialLayouts"][i]["textureRepetition"] = toJsonValue(m_textureRepetitions[i]);

		rootComponent["materialLayouts"][i]["materialName"] = m_terrainLayouts[i]->getName(); //TODO : remove ?

		m_terrainLayouts[i].save(rootComponent["materialLayouts"][i]["material"]);
	}
	/*
	std::stringstream ss;
	for (int i = 0; i < m_filterTexture->w*m_filterTexture->h; i++) {
		ss << i;
	}
	std::string stringifiedTexture;
	ss>>stringifiedTexture;
	rootComponent["terrainFilterTexture"]["width"] = m_filterTexture->w;
	rootComponent["terrainFilterTexture"]["height"] = m_filterTexture->h;
	rootComponent["terrainFilterTexture"]["data"] = stringifiedTexture;
	*/
	//create file : 
	std::ofstream file;
	file.open("test_terrain.bmp");
	file.close();
	//write to file : 
	unsigned char* pixels = new unsigned char[m_filterTexture->w*m_filterTexture->h*3];
	glBindTexture(GL_TEXTURE_2D, m_filterTexture->glId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_write_bmp("test_terrain.bmp", m_filterTexture->w, m_filterTexture->h, 3, pixels); //TODO : améliorer ça
}

void Terrain::load(const Json::Value & rootComponent)
{
	//parameters : 
	m_subdivision = rootComponent.get("subdivision", 10).asInt();
	m_width = rootComponent.get("width", 10).asFloat();
	m_depth = rootComponent.get("depth", 10).asFloat();
	m_height = rootComponent.get("height", 10).asFloat();
	m_offset = fromJsonValue<glm::vec3>(rootComponent["offset"], glm::vec3(0,0,0));

	//noise : 
	m_seed = rootComponent.get("seed", 10).asInt();
	m_terrainNoise.load(rootComponent["terrainNoise"]);

	//materials : 
	int materialLayoutCount = rootComponent.get("materialLayoutCount", 0).asInt();
	for (int i = 0; i < materialLayoutCount; i++) {
		glm::vec2 textureRepetition = fromJsonValue<glm::vec2>(rootComponent["materialLayouts"][i]["textureRepetition"], glm::vec2(1, 1));
		m_textureRepetitions.push_back(textureRepetition);

		std::string materialLayoutName = rootComponent["materialLayouts"][i]["materialName"].asString(); //TODO : supprimer ?

		m_terrainLayouts.push_back(ResourcePtr<MaterialInstance>(rootComponent["materialLayouts"][i]["material"]));
	}
	//recreate m_filterTexture : 
	m_filterTexture->freeGL();
	/*
	std::string textureData = rootComponent["terrainFilterTexture"].get("data", "").asString();
	int texWidth = rootComponent["terrainFilterTexture"].get("width", "").asInt();
	int texHeight = rootComponent["terrainFilterTexture"].get("height", "").asInt();
	std::stringstream ss;
	ss << textureData;
	if (m_filterTexture != nullptr)
		delete m_filterTexture;
	unsigned char* pixels = new unsigned char[texWidth*texHeight];
	for (int i = 0; i < texWidth*texHeight; i++) {
		ss >> pixels[i];
	}
	m_filterTexture = new Texture(pixels, texWidth, texHeight, 3);
	*/
	m_filterTexture = new Texture(FileHandler::CompletePath("test_terrain.bmp")); //TODO : améliorer ça, verifier si ça marche bien

	generateTerrain();
	updateTerrain();
	applyNoise(m_terrainNoise, false);
	//redraw the terrain texture : 
	generateTerrainTexture();
}

//get terrain height at a given point
float Terrain::getHeight(float x, float y)
{
	float noiseValue = m_terrainNoise.getNoiseValue(x, y);

	return (noiseValue * 2.f - 1.f) * m_height + m_offset.y;
}

// simply draw the vertices, using vao.
void Terrain::render(const glm::mat4& projection, const glm::mat4& view)
{
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1), m_offset);
	glm::mat4 mvp = projection * view * modelMatrix;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

	int texCount = 0;
	m_material.use();
	m_material.pushInternalsToGPU(texCount);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_filterTexture->glId);

	for (int i = 0; i < m_terrainLayouts.size(); i++)
	{
		MaterialLit* castedterrainLayout = static_cast<MaterialLit*>(m_terrainLayouts[i].get()); //TODO : changer ça

		//diffuse
		glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, castedterrainLayout->getDiffuse()->glId);
		glBindTexture(GL_TEXTURE_2D, castedterrainLayout->getInternalData<Texture>("TextureDiffuse")->glId);
		//bump
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, castedterrainLayout->getInternalData<Texture>("TextureBump")->glId);//->getBump()->glId);
		//specular
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, castedterrainLayout->getInternalData<Texture>("TextureSpecular")->glId);//->getSpecular()->glId);

		//filter texture : 
		m_material.setUniformFilterTexture(0);
		//diffuse texture :
		m_material.setUniformDiffuseTexture(1);
		//bump texture : 
		m_material.setUniformBumpTexture(2);
		//specular texture : 
		m_material.setUniformSpecularTexture(3);

		m_material.setUniformSpecularPower(*castedterrainLayout->getInternalData<float>("SpecularPower")/*castedterrainLayout->getSpecularPower()*/);

		float offsetMin = (i / (float)m_terrainLayouts.size());
		float offsetMax = ((i + 1) / (float)m_terrainLayouts.size());
		m_material.setUniformLayoutOffset(glm::vec2(offsetMin, offsetMax));
		m_material.setUniformTextureRepetition(m_textureRepetitions[i]);

		//m_material.setUniform_MVP(mvp);
		//m_material.setUniform_normalMatrix(normalMatrix);
		m_material.setUniformModelMatrix(modelMatrix);
		m_material.setUniformViewMatrix(view);
		m_material.setUniformProjectionMatrix(projection);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, m_triangleCount * 3, GL_UNSIGNED_INT, (GLvoid*)0);
		glBindVertexArray(0);
	}
}

void Terrain::renderGrassField(const glm::mat4 & projection, const glm::mat4& view)
{
	m_grassField.render(projection, view);
}

void Terrain::drawUI()
{
	if (ImGui::Button("Parameter"))
		m_currentTerrainTool = TerrainTools::PARAMETER;
	ImGui::SameLine();
	if (ImGui::Button("Perlin"))
		m_currentTerrainTool = TerrainTools::PERLIN;
	ImGui::SameLine();
	if (ImGui::Button("Draw Materials"))
		m_currentTerrainTool = TerrainTools::DRAW_MATERIAL;
	ImGui::SameLine();
	if (ImGui::Button("Draw grass"))
		m_currentTerrainTool = TerrainTools::DRAW_GRASS;


	//if (ImGui::CollapsingHeader("perlin height tool"))
	if(m_currentTerrainTool == TerrainTools::PERLIN)
	{
		if (ImGui::InputInt("terrain seed", &m_seed))
		{
			// change the seed of the generator
			m_terrainNoise.setSeed(m_seed);
			//apply new noise to terrain
			applyNoise(m_terrainNoise, false);
		}

		if (ImGui::InputInt("terrain subdivision", &m_subdivision))
		{
			if (m_subdivision < 5)
				m_subdivision = 5;

			generateTerrain();
			applyNoise(m_terrainNoise, false);
		}

		float tmpFloat = m_terrainNoise.getPersistence();
		if (ImGui::SliderFloat("noise persistence", &tmpFloat, 0.f, 1.f))
		{
			m_terrainNoise.setPersistence(tmpFloat);
			applyNoise(m_terrainNoise, false);
		}
		int tmpInt = m_terrainNoise.getOctaveCount();
		if (ImGui::SliderInt("noise octave count", &tmpInt, 1, 10))
		{
			m_terrainNoise.setOctaveCount(tmpInt);
			applyNoise(m_terrainNoise, false);
		}
		tmpInt = m_terrainNoise.getHeight();
		if (ImGui::SliderInt("noise height", &tmpInt, 1, 512))
		{
			m_terrainNoise.setHeight(tmpInt);
			applyNoise(m_terrainNoise, false);
		}
		tmpInt = m_terrainNoise.getSamplingOffset();
		if (ImGui::SliderInt("noise sampling offset", &tmpInt, 1, 128))
		{
			m_terrainNoise.setSamplingOffset(tmpInt);
			applyNoise(m_terrainNoise, false);
		}


		//if (ImGui::Button("refresh noise texture"))
		//{
		//	computeNoiseTexture(m_terrainNoise.generatePerlin2D());
		//	generateTerrainTexture();
		//}
	}
	//if (ImGui::CollapsingHeader("terrain material"))
	else if(m_currentTerrainTool == TerrainTools::PARAMETER)
	{
		if (ImGui::InputFloat3("terrain offset", &m_offset[0]))
		{
			updateTerrain();
		}

		glm::vec3 terrainDim(m_width, m_height, m_depth);
		if (ImGui::InputFloat3("terrain dimensions", &terrainDim[0]))
		{
			m_width = terrainDim.x;
			m_height = terrainDim.y;
			m_depth = terrainDim.z;

			updateTerrain();
		}
		
		ImGui::PushID("terrainMaterial");
		m_material.drawUI();
		ImGui::PopID();
	}
	//if (ImGui::CollapsingHeader("draw material tool"))
	else if (m_currentTerrainTool == TerrainTools::DRAW_MATERIAL)
	{
		std::string currentMatName("no selected material");
		if(m_currentMaterialToDrawIdx >= 0 && m_currentMaterialToDrawIdx < m_terrainLayouts.size())
			currentMatName = std::string("selected material" + m_terrainLayouts[m_currentMaterialToDrawIdx]->getName());
		ImGui::Text(currentMatName.c_str());

		ImGui::SliderFloat("draw radius", &m_drawRadius, 0.f, 1.f);

		ResourcePtr<MaterialInstance> materialPtrQuery;
		//EditorGUI::ResourceField<MaterialInstance>(materialPtrQuery, "new texture layout", m_newLayoutName, 30);
		EditorGUI::ResourceField<MaterialInstance>("new texture layout", materialPtrQuery);

		ImGui::SameLine();
		if (ImGui::SmallButton("add"))
		{
			if (materialPtrQuery.isValid())
			{
				m_terrainLayouts.push_back(materialPtrQuery);
				//m_terrainLayouts.back()->initGL();
				m_textureRepetitions.push_back(glm::vec2(1.f, 1.f));

				//redraw the terrain texture : 
				generateTerrainTexture();
			}
		}

		for (int i = 0; i < m_terrainLayouts.size(); i++)
		{
			ImGui::PushID(i);

			/*
			if (ImGui::SmallButton("up") && i > 0)
			{
			auto tmp = m_terrainLayouts[i];
			m_terrainLayouts[i] = m_terrainLayouts[i - 1];
			m_terrainLayouts[i - 1] = tmp;

			generateTerrainTexture();
			}
			ImGui::SameLine();

			if (ImGui::SmallButton("down") && i < m_terrainLayouts.size() - 1)
			{
			auto tmp = m_terrainLayouts[i];
			m_terrainLayouts[i] = m_terrainLayouts[i + 1];
			m_terrainLayouts[i + 1] = tmp;

			generateTerrainTexture();
			}
			ImGui::SameLine();
			*/

			if (ImGui::Button("select"))
			{
				m_currentMaterialToDrawIdx = i;
			}
			ImGui::SameLine();

			ImGui::Text(m_terrainLayouts[i]->getName().c_str());
			ImGui::SameLine();

			glm::vec2 tmpVec2 = m_textureRepetitions[i];

			if (ImGui::SmallButton("remove"))
			{
				//m_terrainLayouts[i]->freeGL();
				m_terrainLayouts.erase(m_terrainLayouts.begin() + i);
				m_textureRepetitions.erase(m_textureRepetitions.begin() + i);

				//redraw the terrain texture : 
				generateTerrainTexture();
			}
			if (ImGui::InputFloat2("texture repetition", &(tmpVec2)[0]))
			{
				m_textureRepetitions[i] = tmpVec2;

				//glBindTexture(GL_TEXTURE_2D, m_terrainLayouts[i]->getDiffuse()->glId);
				//glGenerateMipmap(GL_TEXTURE_2D);
				//glBindTexture(GL_TEXTURE_2D, 0);
				//glBindTexture(GL_TEXTURE_2D, m_terrainLayouts[i]->getBump()->glId);
				//glGenerateMipmap(GL_TEXTURE_2D);
				//glBindTexture(GL_TEXTURE_2D, 0);
				//glBindTexture(GL_TEXTURE_2D, m_terrainLayouts[i]->getSpecular()->glId);
				//glGenerateMipmap(GL_TEXTURE_2D);
				//glBindTexture(GL_TEXTURE_2D, 0);


				generateTerrainTexture();

			}
			ImGui::PopID();
		}
	}
	else if (m_currentTerrainTool == TerrainTools::DRAW_GRASS)
	{
		if (ImGui::Button("update heights"))
		{
			updateGrassPositions();
		}

		ImGui::SliderFloat("draw radius", &m_drawRadius, 0.f, 1.f);
		ImGui::SliderFloat("grass density", &m_grassDensity, 0, m_maxGrassDensity);


		ResourcePtr<Texture> texturePtrQuery;
		//if (EditorGUI::ResourceField<Texture>(texturePtrQuery, "grass texture name", m_newGrassTextureName, 30))
		if (EditorGUI::ResourceField<Texture>("grass texture name", texturePtrQuery))
		{
			if (texturePtrQuery.isValid())
				m_grassField.grassTexture = texturePtrQuery;
		}

		m_grassField.drawUI();
	}

}