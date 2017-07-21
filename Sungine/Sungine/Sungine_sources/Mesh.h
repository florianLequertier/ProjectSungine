#pragma once

#include <vector>
#include <iostream>
#include <memory>

#include "glew/glew.h"

#include "glm/glm.hpp"
#include "glm/common.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "IDrawable.h"
#include "BatchableWith.h"

#include "Asset.h"
#include "AssetManager.h"


//forwards : 
class Ray;
class CollisionInfo;
class SubMesh;
class AABB;

//struct SubMesh {
//
//	std::vector<int> triangleIndex;
//	std::vector<float> uvs;
//	std::vector<float> vertices;
//	std::vector<float> normals;
//	std::vector<float> tangents;
//
//	GLuint vbo_index;
//	GLuint vbo_vertices;
//	GLuint vbo_uvs;
//	GLuint vbo_normals;
//	GLuint vbo_tangents;
//	GLuint vao;
//
//};

class Skeleton;

class Mesh : public Asset
{
	Assimp::Importer* importer; //TODO : a quoi ça sert de laisser ça là ??? En fait ça doit servir pour gerer la durée de vie des animations, mais c'est pas cool...
	std::vector<std::string> animNames; //pour retrouver le nom des sous-anim et les detruire quand on détruit le mesh.

	glm::vec3 topRight;
	glm::vec3 bottomLeft;
	glm::vec3 origin;
	AABB m_localAABB;

	enum Vbo_usage { USE_INDEX = 1 << 0, USE_VERTICES = 1 << 1, USE_UVS = 1 << 2, USE_NORMALS = 1 << 3, USE_TANGENTS = 1 << 4 , USE_BONES = 1 << 5/* , USE_INSTANTIATION = 1 << 5 */};
	enum Vbo_types { VERTICES = 0, NORMALS, UVS, TANGENTS, BONE_IDS, BONE_WEIGHTS /* INSTANCE_TRANSFORM */, INDEX };

	int subMeshCount;
	int totalTriangleCount;
	std::vector<int> triangleCount;
	std::vector<GLuint> indexOffsets;
	Skeleton* skeleton;
	bool isSkeletalMesh; //true if the mesh owns a skeleton.

	std::vector<int> triangleIndex;
	std::vector<float> uvs;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> tangents;
	
	//std::vector<glm::mat4> transforms;

	GLuint vbo_index;
	GLuint vbo_vertices;
	GLuint vbo_uvs;
	GLuint vbo_normals;
	GLuint vbo_tangents;
	GLuint vbo_bones;
	//GLuint vbo_transforms[4];
	GLuint vao;

	unsigned int vbo_usage;

	int coordCountByVertex;

	GLenum primitiveType;
	GLenum drawUsage;

public:

	//////////////////////////////////////////////////////////
	// Constructors, destroctors, operators
	//////////////////////////////////////////////////////////
	Mesh(GLenum _primitiveType = GL_TRIANGLES, unsigned int _vbo_usage = (USE_INDEX | USE_VERTICES | USE_UVS | USE_NORMALS), int _coordCountByVertex = 3, GLenum _drawUsage = GL_STATIC_DRAW);
	Mesh(const FileHandler::CompletePath& _path, const std::string& meshName = "");
	~Mesh();
	void setMeshDatas(GLenum _primitiveType, unsigned int _vbo_usage, int _coordCountByVertex, GLenum _drawUsage);
	void clear();
	//////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////
	// Asset override
	//////////////////////////////////////////////////////////
	void createNewAssetFile(const FileHandler::CompletePath& filePath) override;
	void loadFromFile(const FileHandler::CompletePath& filePath) override;
	void saveToFile(const FileHandler::CompletePath& filePath) override;
	void saveMetas(const FileHandler::CompletePath& filePath) override;
	void loadMetas(const FileHandler::CompletePath& filePath) override;
	//////////////////////////////////////////////////////////

	//void init(const FileHandler::CompletePath& path, const ID& id) override;
	//void save() override;
	//void resolvePointersLoading() override;

	//initialize vbos and vao, based on the informations of the mesh.
	void initGl();
	void freeGl();

	//update a single vbo.
	void updateVBO(Vbo_types type);
	//update all vbos.
	void updateAllVBOs();

	// simply draw the vertices, using vao.
	void draw() const;
	//draw a specific sub mesh.
	void draw(int idx) const;

	int getSubMeshCount() const;
	std::shared_ptr<SubMesh> makeSharedSubMesh(int subMeshIndex) const;
	const AABB& getLocalAABB() const;

	void computeBoundingBox();

	bool isIntersectedByRay(const Ray& ray, CollisionInfo& collisionInfo) const;

	Skeleton* getSkeleton() const;
	bool getIsSkeletalMesh() const;

private:
	bool initFromScene(const aiScene* pScene, const FileHandler::CompletePath& scenePath);
	void initMesh(unsigned int Index, const aiMesh* paiMesh);
	//Check if the mesh has bones. If true, create the appropriate skeleton :  
	void loadBones(unsigned int meshIndex, const aiMesh * mesh, const aiNode * rootNode, unsigned int firstVertexId);
	void loadAnimations(const FileHandler::CompletePath& scenePath, const aiScene* scene);
};

class Material3DObject;

// Represent an instance of a subpart of the mesh in the world.
class SubMesh final : public IBatchableWith<Material3DObject>
{
private:
	const Mesh* m_meshPtr;
	const int m_subMeshId;
	glm::mat4 m_modelMatrix;
	AABB m_aabb;
	bool m_castShadow;

public:
	SubMesh(const Mesh* mesh, int id) 
		: m_meshPtr(mesh)
		, m_subMeshId(id)
		, m_castShadow(true)
	{
		if (m_meshPtr != nullptr)
			m_aabb = m_meshPtr->getLocalAABB();
	}
	void setModelMatrix(const glm::mat4& modelMatrix);
	void setAABB(const AABB& aabb);

	const Mesh* getMeshPtr() const;

	// Herited from IDrawable
	const AABB & getVisualBoundingBox() const override;
	void draw() const override;
	virtual const glm::mat4& getModelMatrix() const override;
	virtual bool castShadows() const override;
	Skeleton* getSkeletonIfPossible() const override;

	void setExternalsOf(const Material3DObject& material, const glm::mat4& projection, const glm::mat4& view, int* texId = nullptr) const override;

};