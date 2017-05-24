#pragma once

#include <iostream>

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "Collider.h"

//forwards :
class Terrain;

////small struct to store infos about a collision
//struct CollisionInfo {
//	glm::vec3 normal;
//	glm::vec3 point;
//};

class Ray
{
private:
	glm::vec3 direction;
	glm::vec3 origin;
	float length;

public:
	Ray(glm::vec3 _origin, glm::vec3 _direction, float _length = 100.f);
	glm::vec3 at(float t) const;
	bool intersect(Collider& other, float* t = nullptr);
	bool intersectPlane(const glm::vec3& anchor, const glm::vec3& normal, float* t = nullptr) const;
	bool intersectTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, CollisionInfo& collisionInfo) const;
	bool intersectMesh(const Mesh& mesh, CollisionInfo& collisionInfo) const;
	bool intersectTerrain(const Terrain& terrain, CollisionInfo& collisionInfo) const;
	glm::vec3 getDirection() const;
	glm::vec3 getOrigin() const;
	float getLength() const;
	void debugLog();
};
