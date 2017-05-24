

#include "Octree.h"

const glm::vec3 AABB::pointOffsets[8] = 
{ glm::vec3(0.5f, 0.5f, 0.5f),
glm::vec3(0.5f, 0.5f, -0.5f),
glm::vec3(0.5f, -0.5f, 0.5f),
glm::vec3(-0.5f, 0.5f, 0.5f),
glm::vec3(0.5f, -0.5f, -0.5f),
glm::vec3(-0.5f, 0.5f, -0.5f),
glm::vec3(-0.5f, -0.5f, 0.5f),
glm::vec3(-0.5f, -0.5f, -0.5f) };

namespace OctreeHelper {

// For points : 
template<>
float distanceMin<glm::vec3>(const glm::vec3& item, const glm::vec3& position)
{
	return glm::distance(item, position);
}
template<>
float distanceMax<glm::vec3>(const glm::vec3& item, const glm::vec3& position)
{
	return glm::distance(item, position);
}

// For AABB :
template<>
float distanceMin<AABB>(const AABB& item, const glm::vec3& position)
{
	float minDistance = 0;
	for (int i = 0; i < 8; i++)
	{
		minDistance = glm::min(glm::distance(item.getPointPosition(i), position), minDistance);
	}
	return minDistance;
}
template<>
float distanceMax<AABB>(const AABB& item, const glm::vec3& position)
{
	float maxDistance = glm::distance(item.getPointPosition(0), position);
	for (int i = 1; i < 8; i++)
	{
		maxDistance = glm::min(glm::distance(item.getPointPosition(i), position), maxDistance);
	}
	return maxDistance;
}

}