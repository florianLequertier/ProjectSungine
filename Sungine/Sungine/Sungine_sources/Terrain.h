#pragma once

#include <vector>
#include <time.h>
#include <sstream>
#include <fstream>

#include "glew/glew.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Mesh.h"
#include "Materials.h"
#include "PerlinNoise.h"

#include "Point.h"
#include "Link.h"
#include "WindZone.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

//forwards : 
class Ray;

struct GrassPhysicLink {
	int p1_idx;
	glm::vec3 p2_pos;
	float l;

	GrassPhysicLink(int _p1_idx, glm::vec3 _p2_pos, float _l) : p1_idx(_p1_idx), p2_pos(_p2_pos), l(_l)
	{}
};

struct GrassKey
{
	int i;
	int j;

	inline GrassKey(int _i, int _j) : i(_i), j(_j)
	{}

	inline bool operator==(const GrassKey& other)
	{
		return i == other.i && j == other.j;
	}
};

//structure which store infos to render grass in instanced mode
struct GrassField : public ISerializable
{

	enum VboTypes {VERTICES = 0, NORMALS, UVS, POSITIONS, ANIM_POS};
	
	MaterialGrassField materialGrassField;

	ResourcePtr<Texture> grassTexture;

	GLuint vao;
	
	int triangleCount;

	std::vector<int> triangleIndex;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> uvs;
	std::vector<float> positions; //grass positions
	std::vector<GrassKey> grassKeys; //keys to identity grass
	//for physic simulation : 
	std::vector<float> offsets;
	std::vector<float> forces;
	std::vector<float> speeds;
	std::vector<GrassPhysicLink> links;
	//global parameter for grass :
	float mass;
	float viscosity;
	float rigidity;
	bool lockYPlane;

	GLuint vbo_index;
	GLuint vbo_vertices;
	GLuint vbo_uvs;
	GLuint vbo_normals;
	//for physic simulation : 
	GLuint vbo_animPos;

	//additional vbos for instantiation : 
	GLuint vbo_pos;

	GrassField();
	~GrassField();
	void initGl();
	void freeGl();
	void clear();

	void addGrass(GrassKey grassKey, const glm::vec3& position);
	void remove(GrassKey grassKey);

	//draw all grass with instantiation : 
	void draw();

	//render all grass with instantiation : 
	void render(const glm::mat4& projection, const glm::mat4& view);

	//update physic : 
	void updatePhysic(float deltaTime, std::vector<Physic::WindZone*>& windZones); //TODO

	void updateVBOPositions();
	void updateVBOAnimPos();

	void computePoint(float deltaTime, int index);
	void computeLink(float deltaTime, int index);

	void resetPhysic();

	void setViscosity(float _viscosity);
	void setRigidity(float _rigidity);
	void setMass(float _mass);
	float getViscosity() const;
	float getRigidity() const;
	float getMass() const;

	void drawUI();

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;
};


class Terrain : public ISerializable
{
public : 
	enum TerrainTools { PARAMETER = 0, DRAW_MATERIAL, DRAW_GRASS, PERLIN };

private:
	enum Vbo_types { VERTICES = 0, NORMALS, UVS, TANGENTS };

	int m_triangleCount;

	std::vector<int> m_triangleIndex;
	std::vector<float> m_uvs;
	std::vector<float> m_vertices;
	std::vector<float> m_normals;
	std::vector<float> m_tangents;

	std::vector<float> m_heightMap;

	GLuint vbo_index;
	GLuint vbo_vertices;
	GLuint vbo_uvs;
	GLuint vbo_normals;
	GLuint vbo_tangents;
	GLuint vao;

	int m_subdivision;

	MaterialTerrain m_material;

	float m_width;
	float m_depth;
	float m_height;

	glm::vec3 m_offset;

	int m_seed;
	//NoiseGenerator m_terrainNoise;
	Perlin2D m_terrainNoise;

	//texture tool : 
	Texture m_noiseTexture;

	MaterialDrawOnTexture m_drawOnTextureMaterial;
	MaterialTerrainEdition m_terrainMaterial;

	Mesh m_quadMesh;

	GLuint m_terrainFbo;
	GLuint m_materialLayoutsFBO;

	Texture* m_filterTexture;
	Texture m_terrainDiffuse;
	Texture m_terrainBump;
	Texture m_terrainSpecular;
	Texture m_drawMatTexture;
	//Texture m_terrainDepth;

	std::vector<ResourcePtr<Material>> m_terrainLayouts;
	std::vector<glm::vec2> m_textureRepetitions;

	float m_noiseMax;
	float m_noiseMin;

	//grass Management :
	GrassField m_grassField;
	std::vector<int> m_grassLayout;
	float m_grassLayoutDelta; //the delta between two grass
	int m_grassLayoutWidth;
	int m_grassLayoutDepth;

	//for UI : 
	TerrainTools m_currentTerrainTool;
	//char m_newLayoutName[30];
	int m_currentMaterialToDrawIdx;
	float m_drawRadius;
	float m_maxGrassDensity;
	float m_grassDensity;
	//char m_newGrassTextureName[30];

	//for physic : 
	btBvhTriangleMeshShape* m_terrainCollider;
	btRigidBody* m_terrainRigidbody;
	btDiscreteDynamicsWorld* m_ptrToPhysicWorld;
	btTriangleIndexVertexArray* m_triangleIndexVertexArray;
	glm::vec3 m_aabbMin;
	glm::vec3 m_aabbMax;


public:

	Terrain(float width = 100, float height = 10, float depth = 100, int subdivision = 30, glm::vec3 offset = glm::vec3(0,0,0));
	~Terrain();
	//initialize vbos and vao, based on the informations of the mesh.
	void initGl();
	void freeGl(); // delete gl information from GPU memory
	void clear(); //delete everything from memory.
	//init physics, must be called before all physic function call : 
	void initPhysics(btDiscreteDynamicsWorld* physicWorld);

	// simply draw the vertices, using vao.
	void render(const glm::mat4& projection, const glm::mat4& view);
	//a simple shortcut for this->m_grassField.render(projection, view)
	void renderGrassField(const glm::mat4& projection, const glm::mat4& view);

	void drawUI();

	void computeNormals();

	//generate new positions for vertices of the terrain, update normals, and update the terrain texture, call this function after modifying the noise of the terrain :  
	void applyNoise(Perlin2D& perlin2D, bool _computeNoiseTexture = true);
	//Quickly update vertices and grass on terrain, call this function after scaling the terrain : 
	void updateTerrain();
	//regenerate a flat terrain :
	void generateTerrain();
	//generate the appropriate btCollider for this terrain, don't set it to the rigidbody :
	void generateCollider();
	//regenerate the appropriate btCollider for this terrain and set it to the rigidbody, removing the old collider : 
	void updateCollider(); 
	//get the generated btCollider : generateCollider
	btBvhTriangleMeshShape* getColliderShape() const;

	void computeNoiseTexture(Perlin2D& perlin2D);
	void generateTerrainTexture();

	void drawMaterialOnTerrain(glm::vec3 position, float radius, int textureIdx);
	void drawMaterialOnTerrain(glm::vec3 position);

	bool isIntersectedByRay(const Ray& ray, CollisionInfo& collisionInfo) const;

	//get terrain height at a given point
	float getHeight(float x, float y);

	void drawGrassOnTerrain(const glm::vec3 position);
	void drawGrassOnTerrain(const glm::vec3 position, float radius, float density, float maxDensity);
	void updateGrassPositions();

	TerrainTools getCurrentTerrainTool() const;

	//update physic : 
	void updatePhysic(float deltaTime, std::vector<Physic::WindZone*>& windZones);

	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;
};

