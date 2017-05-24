

#include "Ray.h"
//forwards : 
#include "Terrain.h"

Ray::Ray(glm::vec3 _origin, glm::vec3 _direction, float _length) : origin(_origin), direction(_direction), length(_length)
{

}

glm::vec3 Ray::at(float t) const
{
	return origin + t*direction;
}

glm::vec3 Ray::getDirection() const
{
	return direction;
}

glm::vec3 Ray::getOrigin() const
{
	return origin;
}

float Ray::getLength() const
{
	return length;
}

bool Ray::intersect(Collider& other, float* t)
{
	return other.isIntersectedByRay(*this, t);
}

bool Ray::intersectPlane(const glm::vec3& anchor, const glm::vec3& normal, float* t) const
{
	float normalDotDirection = glm::dot(normal, direction);
	if (normalDotDirection == 0)
		return false;

	if (t != nullptr)
		*t = glm::dot((anchor - origin), normal) / normalDotDirection;
	return true;
}

bool Ray::intersectTriangle(const glm::vec3 & a, const glm::vec3 & b, const glm::vec3 & c, CollisionInfo & collisionInfo) const
{
	glm::vec3 triangleNormal = glm::cross(b - a, c - a);
	triangleNormal = glm::normalize(triangleNormal);
	collisionInfo.normal = triangleNormal;
	collisionInfo.point = glm::vec3(0, 0, 0);
	
	float denom = glm::dot(triangleNormal, direction);
	//we are strictly perpendicular to the triangle so there are no intersection
	if (denom == 0) 
		return false;
	float k = -glm::dot(triangleNormal, (origin - a)) / denom;

	//collision with plane is OK
	if (k > 0)
	{
		//p is the intersection point with the plane
		glm::vec3 p = origin + k * direction;
		collisionInfo.point = p;
		//is p also inside the triangle ? 
		return glm::dot(glm::cross(a-p, b-p), triangleNormal) > 0.f && glm::dot(glm::cross(b-p,c-p), triangleNormal) > 0.f && glm::dot(glm::cross(c-p,a-p), triangleNormal) > 0.f;
	}
	return false;
}

bool Ray::intersectMesh(const Mesh & mesh, CollisionInfo & collisionInfo) const
{
	return mesh.isIntersectedByRay(*this, collisionInfo);
}

bool Ray::intersectTerrain(const Terrain & terrain, CollisionInfo & collisionInfo) const
{
	return terrain.isIntersectedByRay(*this, collisionInfo);
}

void Ray::debugLog()
{
	std::cout << "ray origin : (" << origin.x <<", "<<origin.y<<", "<<origin.z << "), ray direction : (" << direction.x << ", " << direction.y << ", " << direction.z << "), ray lenght : " << length << std::endl;
}