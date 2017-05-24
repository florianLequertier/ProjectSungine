#pragma once

#include <glm/glm.hpp>

struct AABB
{
	static const glm::vec3 pointOffsets[8];

	glm::vec3 center;
	glm::vec3 halfSizes;

	AABB() : center(0), halfSizes(0)
	{

	}

	AABB(const glm::vec3& _center, float _halfSize) : center(_center), halfSizes(_halfSize, _halfSize, _halfSize)
	{

	}

	AABB(const glm::vec3& _center, const glm::vec3& _halfSize) : center(_center), halfSizes(_halfSize)
	{

	}

	glm::vec3 getPointPosition(int i) const
	{
		assert(i >= 0 && i < 8);
		return center + pointOffsets[i] * halfSizes;
	}

	bool contains(const glm::vec3& point) const
	{
		//is position inside current node ?
		return(center.x - halfSizes.x <= point.x && center.x + halfSizes.x > point.x &&
			center.y - halfSizes.y <= point.y && center.y + halfSizes.y > point.y &&
			center.z - halfSizes.z <= point.z && center.z + halfSizes.z > point.z);
	}

	bool contains(const AABB& other) const
	{
		return contains(other.center, other.halfSizes);
	}

	bool contains(const glm::vec3& otherCenter, const glm::vec3& otherHalfSize) const
	{
		return(center.x - halfSizes.x <= otherCenter.x - otherHalfSize.x && center.x + halfSizes.x > otherCenter.x + otherHalfSize.x &&
			center.y - halfSizes.y <= otherCenter.y - otherHalfSize.y && center.y + halfSizes.y > otherCenter.y + otherHalfSize.y &&
			center.z - halfSizes.z <= otherCenter.z - otherHalfSize.z && center.z + halfSizes.z > otherCenter.z + otherHalfSize.z);
	}

	bool containsOrIntersects(const AABB& other) const
	{
		return containsOrIntersects(other.center, other.halfSizes);
	}

	bool containsOrIntersects(const glm::vec3& otherCenter, const glm::vec3& otherHalfSize) const
	{
		return(((center.x - halfSizes.x < otherCenter.x - otherHalfSize.x && center.x + halfSizes.x >= otherCenter.x - otherHalfSize.x) || (center.x - halfSizes.x < otherCenter.x + otherHalfSize.x && center.x + halfSizes.x >= otherCenter.x + otherHalfSize.x)) &&
			((center.y - halfSizes.y < otherCenter.y - otherHalfSize.y && center.y + halfSizes.y >= otherCenter.y - otherHalfSize.y) || (center.y - halfSizes.y < otherCenter.y + otherHalfSize.y && center.y + halfSizes.y >= otherCenter.y + otherHalfSize.y)) &&
			((center.z - halfSizes.z < otherCenter.z - otherHalfSize.z && center.z + halfSizes.z >= otherCenter.z - otherHalfSize.z) || (center.z - halfSizes.z < otherCenter.z + otherHalfSize.z && center.z + halfSizes.z >= otherCenter.z + otherHalfSize.z)));
	}

	bool containedIn(const AABB& other) const
	{
		return containedIn(other.center, other.halfSizes);
	}

	bool containedIn(const glm::vec3& otherCenter, const glm::vec3& otherHalfSize) const
	{
		return((center.x - halfSizes.x > otherCenter.x - otherHalfSize.x && center.x + halfSizes.x <= otherCenter.x + otherHalfSize.x) &&
			center.y - halfSizes.y > otherCenter.y - otherHalfSize.y && center.y + halfSizes.y <= otherCenter.y + otherHalfSize.y &&
			center.z - halfSizes.z > otherCenter.z - otherHalfSize.z && center.z + halfSizes.z <= otherCenter.z + otherHalfSize.z);
	}

	bool containedInSphere(const glm::vec3& otherCenter, float otherRadius) const
	{
		for (int i = 0; i < 8; i++)
		{
			if (glm::distance(getPointPosition(i), otherCenter) > otherRadius)
				return false;
		}
		return true;
	}

	bool containedInOrIntersectedBySphere(const glm::vec3& otherCenter, float otherRadius) const
	{
		int intersectCount = 0;
		for(int i = 0; i < 3; i++)
			intersectCount += (glm::min(glm::distance(center[i] + halfSizes[i], otherCenter[i]), glm::distance(center[i] - halfSizes[i], otherCenter[i])) < otherRadius) ? 1 : 0;
		if (intersectCount >= 2)
			return true;
		else
			return false;
	}

	bool containedInOrIntersectedBy(const AABB& other) const
	{
		return containedInOrIntersectedBy(other.center, other.halfSizes);
	}

	bool containedInOrIntersectedBy(const glm::vec3& otherCenter, const glm::vec3& otherHalfSize) const
	{
		return(((center.x - halfSizes.x > otherCenter.x - otherHalfSize.x && center.x - halfSizes.x <= otherCenter.x + otherHalfSize.x) || (center.x + halfSizes.x > otherCenter.x - otherHalfSize.x && center.x + halfSizes.x <= otherCenter.x + otherHalfSize.x)) &&
			((center.y - halfSizes.y > otherCenter.y - otherHalfSize.y && center.y - halfSizes.y <= otherCenter.y + otherHalfSize.y) || (center.y + halfSizes.y > otherCenter.y - otherHalfSize.y && center.y + halfSizes.y <= otherCenter.y + otherHalfSize.y)) &&
			((center.z - halfSizes.z > otherCenter.z - otherHalfSize.z && center.z - halfSizes.z <= otherCenter.z + otherHalfSize.z) || (center.z + halfSizes.z > otherCenter.z - otherHalfSize.z && center.z + halfSizes.z <= otherCenter.z + otherHalfSize.z)));
	}

	bool containedInFrustum(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, const glm::vec3& cameraForward) const
	{
		float maxX = 1, maxY = 1, minX = -1, minY = -1;
		const glm::vec3 topRight = center + halfSizes;
		const glm::vec3 bottomLeft = center - halfSizes;

		//is camera inside bounding box ?
		if (!(cameraPosition.x > bottomLeft.x && cameraPosition.x < topRight.x && cameraPosition.y > bottomLeft.y && cameraPosition.y < topRight.y && cameraPosition.z > bottomLeft.z && cameraPosition.z < topRight.z))
		{
			//if outside : 

			const glm::mat4 vp = projection * view;
			glm::vec4 tmpColliderPoint01;
			glm::vec3 tmpColliderPoint02;
			float outsideFactor = 0;
			bool behindCamera = false;

			for (int i = 0; i < 8; i++)
			{
				tmpColliderPoint02 = getPointPosition(i) - cameraPosition ;
				outsideFactor = cameraForward.x * tmpColliderPoint02.x + cameraForward.y * tmpColliderPoint02.y + cameraForward.z * tmpColliderPoint02.z;
				behindCamera &= (outsideFactor < 0);
			}
			//is the AABB behind the camera ?
			if (behindCamera)
				return false;

			for (int i = 0; i < 8; i++)
			{
				tmpColliderPoint01 = vp * glm::vec4(getPointPosition(i), 1);
				tmpColliderPoint02 = glm::vec3(tmpColliderPoint01.x / tmpColliderPoint01.w, tmpColliderPoint01.y / tmpColliderPoint01.w, tmpColliderPoint01.z / tmpColliderPoint01.w);

				if (tmpColliderPoint02.x > maxX) maxX = tmpColliderPoint02.x;
				if (tmpColliderPoint02.x < minX) minX = tmpColliderPoint02.x;
				if (tmpColliderPoint02.y > maxY) maxY = tmpColliderPoint02.y;
				if (tmpColliderPoint02.y < minY) minY = tmpColliderPoint02.y;
			}

			//is AABB outside the camera frustum ? 
			if ((maxX < -1 && minX < -1)
				|| (maxY < -1 && minY < -1)
				|| (maxX > 1 && minX > 1)
				|| (maxY > 1 && minY > 1))
			{
				return false;
			}
			return true;
		}
		// If inside : 
		return true;
	}

};

namespace Collisions
{
	/////////////////////////////////////////
	//// BEGIN : ContainedInSphere
	template<typename T>
	inline bool containedInSphere(const T& item, const glm::vec3& center, float radius)
	{
		assert(false && "not implemented";)
	}

	template<>
	inline bool containedInSphere<AABB>(const AABB& item, const glm::vec3& center, float radius)
	{
		return item.containedInSphere(center, radius);
	}

	template<>
	inline bool containedInSphere<glm::vec3>(const glm::vec3& item, const glm::vec3& center, float radius)
	{
		return glm::distance(item, center) < radius;
	}

	//// END : ContainedInSphere
	/////////////////////////////////////////

	/////////////////////////////////////////
	//// BEGIN : ContainedInSphere

	template<typename T>
	inline bool containedInOrIntersectedBySphere(const T& item, const glm::vec3& center, float radius)
	{
		assert(false && "not implemented";)
	}

	template<>
	inline bool containedInOrIntersectedBySphere<AABB>(const AABB& item, const glm::vec3& center, float radius)
	{
		return item.containedInOrIntersectedBySphere(center, radius);
	}

	template<>
	inline bool containedInOrIntersectedBySphere<glm::vec3>(const glm::vec3& item, const glm::vec3& center, float radius)
	{
		return glm::distance(item, center) <= radius;
	}
}