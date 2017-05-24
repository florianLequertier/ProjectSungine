#pragma once

#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "BasicColliders.h"

namespace OctreeHelper {

	template<typename ItemType>
	inline float distanceMin(const ItemType& item, const glm::vec3& position)
	{
		assert(false && "wrong item for distance computation.");
		return 0.f;
	}

	template<typename ItemType>
	inline float distanceMax(const ItemType& item, const glm::vec3& position)
	{
		assert(false && "wrong item for distance computation.");
		return 0.f;
	}

	template<>
	float distanceMin<AABB>(const AABB& item, const glm::vec3& position);

	template<>
	float distanceMax<glm::vec3>(const glm::vec3& item, const glm::vec3& position);
}

//A node placed in an octree. It is an AABB bounding box with 8 childs.
//Its size is 2*halfSize and its position is its parameter center.
template<typename ItemType, typename spatialKeyType>
struct OctreeNode
{
	OctreeNode* childs[8];
	AABB* childBounds[8];

	glm::vec3 center;
	float halfSize;

	std::vector<spatialKeyType> spatialKeys;
	std::vector<ItemType*> elements;

	OctreeNode(glm::vec3 _center, float _halfSize) : center(_center), halfSize(_halfSize)
	{
		for (int i = 0; i < 8; i++)
			childs[i] = nullptr;

		childBounds[0] = new AABB(_center + glm::vec3(0.5f, 0.5f, 0.5f)*_halfSize, _halfSize*0.5f);

		childBounds[1] = new AABB(_center + glm::vec3(0.5f, 0.5f, -0.5f)*_halfSize, _halfSize*0.5f);
		childBounds[2] = new AABB(_center + glm::vec3(0.5f, -0.5f, 0.5f)*_halfSize, _halfSize*0.5f);
		childBounds[3] = new AABB(_center + glm::vec3(-0.5f, 0.5f, 0.5f)*_halfSize, _halfSize*0.5f);

		childBounds[4] = new AABB(_center + glm::vec3(0.5f, -0.5f, -0.5f)*_halfSize, _halfSize*0.5f);
		childBounds[5] = new AABB(_center + glm::vec3(-0.5f, 0.5f, -0.5f)*_halfSize, _halfSize*0.5f);
		childBounds[6] = new AABB(_center + glm::vec3(-0.5f, -0.5f, 0.5f)*_halfSize, _halfSize*0.5f);

		childBounds[7] = new AABB(_center + glm::vec3(-0.5f, -0.5f, -0.5f)*_halfSize, _halfSize*0.5f);
	}

	~OctreeNode()
	{
		for (int i = 0; i < 8; i++)
		{
			delete childBounds[i];
			if(childs[i] != nullptr)
				delete childs[i];
		}
	}

	void add(ItemType* item, const spatialKeyType& spatialKey, int currentDepth, int maxDepth);
	bool remove(ItemType* item, const spatialKeyType& spatialKey, int currentDepth, int maxDepth);
	bool remove(ItemType* item, int currentDepth, int maxDepth);
	ItemType* find(const spatialKeyType& spatialKey, int currentDepth, int maxDepth) const;
	void findAll(const spatialKeyType& spacialKey, int currentDepth, int maxDepth, std::vector<ItemType*>& results) const;
	//find all neightbors, in a sphere of center : "center" and radius : "radius".
	//includes items which are not completly contained in the sphere but just intersected by it
	void findNeighborsContainedOrIntersected(const glm::vec3& center, float radius, int currentDepth, int maxDepth, std::vector<ItemType*>& results) const;
	//find all neightbors, in a sphere of center : "center" and radius : "radius".
	//includes only items which are completly contained in the sphere.
	void findNeighborsContained(const glm::vec3& center, float radius, int currentDepth, int maxDepth, std::vector<ItemType*>& results) const;
	//recursivly get all centers and sizes of active nodes : 
	void getAllCenterAndSize(std::vector<glm::vec3>& centers, std::vector<float>& halfSizes) const;
	//get all elements contains in the current node and its childrens : 
	void getAllElements(std::vector<ItemType*>& elements) const;
	//get the number of childs which are active (ie contains at least one item) : 
	int getActiveChildCount() const;
	//get number of elements contained in the node : 
	int getElementCount() const;
	//get all elements which are visible by the camera modelized by the givent view and projection matrices :
	void findVisibleElements(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, const glm::vec3& cameraForward, std::vector<ItemType*>& results, int currentDepth, int maxDepth) const;

	void drawDebug() const;

	//bool contains(const glm::vec3& point);
	//bool contains(ItemType* item);
	//bool containedIn(const glm::vec3& position, float halfSize);
	////get all elements contains in the current node and its childrens if they are contains into or intersected by a sphere of given center and radius : 
	//void getAllElementsContainedOrIntersected(std::vector<ItemType*>& _elements, const glm::vec3& center, float radius);
	////get all elements contains in the current node and its childrens if they are contains into a sphere of given center and radius : 
	//void getAllElementsContained(std::vector<ItemType*>& _elements, const glm::vec3& center, float radius);
};


template<typename ItemType, typename spatialKeyType>
void OctreeNode<ItemType, spatialKeyType>::drawDebug() const
{
	ImGui::Text("center : %f, %f, %f", center.x, center.y, center.z);
	ImGui::Text("halfSize : %f", halfSize);
	ImGui::Text("element count : %d", elements.size());

	for (int i = 0; i < 8; i++)
	{
		if (childs[i] != nullptr)
		{
			if (ImGui::TreeNode((void*)childs[i], "Child %d", i))
			{
					childs[i]->drawDebug();
				ImGui::TreePop();
			}
		}
	}
}

template<typename ItemType, typename spatialKeyType>
void OctreeNode<ItemType, spatialKeyType>::findVisibleElements(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, const glm::vec3& cameraForward, std::vector<ItemType*>& results, int currentDepth, int maxDepth) const
{
	//max depth reached : 
	if (currentDepth >= maxDepth)
	{
		int index = 0;
		for (auto& spatialKey : spatialKeys)
		{
			if (spatialKey.containedInFrustum(view, projection, cameraPosition, cameraForward))
				results.push_back(elements[index]);
			index++;
		}
		return;
	}

	//max depth not reached, we test childs :
	for (int i = 0; i < 8; i++)
	{
		if (childBounds[i]->containedInFrustum(view, projection, cameraPosition, cameraForward))
		{
			if (childs[i] != nullptr)
				return childs[i]->findVisibleElements(view, projection, cameraPosition, cameraForward, results, (currentDepth+1), maxDepth);
		}
	}

	//none of childs contains the current position, and max depth not reached (shouldn't happend for insertion based only on position...) :
	int index = 0;
	for (auto& spatialKey : spatialKeys)
	{
		if (spatialKey.containedInFrustum(view, projection, cameraPosition, cameraForward))
			results.push_back(elements[index]);
		index++;
	}
}

template<typename ItemType, typename spatialKeyType>
int OctreeNode<ItemType, spatialKeyType>::getElementCount() const
{
	return elements.size();
}

template<typename ItemType, typename spatialKeyType>
int OctreeNode<ItemType, spatialKeyType>::getActiveChildCount() const
{
	int sum = 0;
	for (int i = 0; i < 8; i++){
		if (childs[i] != nullptr){
			sum++;
		}
	}
	return sum;
}

template<typename ItemType, typename spatialKeyType>
void OctreeNode<ItemType, spatialKeyType>::getAllCenterAndSize(std::vector<glm::vec3>& centers, std::vector<float>& halfSizes) const
{
	for (int i = 0; i < 8; i++)
	{
		if (childs[i] != nullptr && (childs[i]->getElementCount() > 0 || childs[i]->getActiveChildCount() > 0) )
		{
			centers.push_back(childBounds[i]->center);
			halfSizes.push_back(childBounds[i]->halfSizes[0]);
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (childs[i] != nullptr && (childs[i]->getElementCount() > 0 || childs[i]->getActiveChildCount() > 0))
		{
			childs[i]->getAllCenterAndSize(centers, halfSizes);
		}
	}
}

template<typename ItemType, typename spatialKeyType>
void OctreeNode<ItemType, spatialKeyType>::getAllElements(std::vector<ItemType*>& _elements) const
{
	for (int i = 0; i < elements.size(); i++)
	{
		_elements.push_back(elements[i]);
	}

	for (int i = 0; i < 8; i++)
	{
		if (childs[i] != nullptr)
		{
			childs[i]->getAllElements(_elements);
		}
	}
}

template<typename ItemType, typename spatialKeyType>
void OctreeNode<ItemType, spatialKeyType>::add(ItemType* item, const spatialKeyType& spatialKey, int currentDepth, int maxDepth)
{
	//max depth reached, we add the item to the current node : 
	if (currentDepth >= maxDepth)
	{
		elements.push_back(item);
		spatialKeys.push_back(spatialKey);

		return;
	}

	//max depth not reached, we test childs :
	for (int i = 0; i < 8; i++)
	{
		if ( childBounds[i]->contains(spatialKey) )
		{
			if (childs[i] == nullptr)
				childs[i] = new OctreeNode<ItemType, spatialKeyType>(childBounds[i]->center, childBounds[i]->halfSizes[0]);

			childs[i]->add(item, spatialKey, (currentDepth+1), maxDepth);

			return;
		}
	}
	//none of childs contains the current spatialKey, and max depth not reached (shouldn't happend for insertion based only on position...), 
	//we add the item to the current node :
	elements.push_back(item);
	spatialKeys.push_back(spatialKey);
}

template<typename ItemType, typename spatialKeyType>
bool OctreeNode<ItemType, spatialKeyType>::remove(ItemType* item, const spatialKeyType& spatialKey, int currentDepth, int maxDepth)
{
	//max depth reached, we try to remove the item to the current node : 
	if (currentDepth >= maxDepth)
	{
		std::vector<ItemType*>::iterator foundIt = std::find(elements.begin(), elements.end(), item);
		if (foundIt != elements.end())
		{
			std::iter_swap(foundIt, elements.end() - 1);
			elements.erase(elements.end() - 1, elements.end());

			int eraseIdx = std::distance(elements.begin(), foundIt);
			assert(eraseIdx < spatialKeys.size());
			std::iter_swap(spatialKeys.begin() + eraseIdx, spatialKeys.end() - 1);
			spatialKeys.erase(spatialKeys.end() - 1, spatialKeys.end());

			return true;
		}
		return false;
	}

	//max depth not reached, we test childs :
	for (int i = 0; i < 8; i++)
	{
		if (childBounds[i]->contains(spatialKey))
		{
			if (childs[i] != nullptr)
			{
				bool successfullyRemoved = childs[i]->remove(item, spatialKey, (currentDepth + 1), maxDepth);

				if (childs[i]->getElementCount() <= 0 && childs[i]->getActiveChildCount() <= 0) 
				{
					delete childs[i];
					childs[i] = nullptr;
				}

				// We just found and remove the item no need to continue.
				if(successfullyRemoved)
					return true;
			}
		}
	}

	//none of childs contains the current spatialKey, and max depth not reached (shouldn't happend for insertion based only on position...), 
	//we try to remove the item to the current node :
	std::vector<ItemType*>::iterator foundIt = std::find(elements.begin(), elements.end(), item);
	if (foundIt != elements.end())
	{
		std::iter_swap(foundIt, elements.end() - 1);
		elements.erase(elements.end() - 1, elements.end());

		int eraseIdx = std::distance(elements.begin(), foundIt);
		assert(eraseIdx < spatialKeys.size());
		std::iter_swap(spatialKeys.begin() + eraseIdx, spatialKeys.end() - 1);
		spatialKeys.erase(spatialKeys.end() - 1, spatialKeys.end());

		return true;
	}
	return false;
}

template<typename ItemType, typename spatialKeyType>
bool OctreeNode<ItemType, spatialKeyType>::remove(ItemType* item, int currentDepth, int maxDepth)
{
	//We search the item in the current node : 
	std::vector<ItemType*>::iterator foundIt = std::find(elements.begin(), elements.end(), item);

	//if the item is found we remove it from the container : 
	if (foundIt != elements.end())
	{
		std::iter_swap(foundIt, elements.end() - 1);
		elements.erase(elements.end() - 1, elements.end());

		int eraseIdx = std::distance(elements.begin(), foundIt);
		assert(eraseIdx < spatialKeys.size());
		std::iter_swap(spatialKeys.begin() + eraseItx, spatialKeys.end() - 1);
		spatialKeys.erase(spatialKeys.end() - 1, spatialKeys.end());

		return true;
	}
	//otherwise, we launch the remove call recursivly on childs :  
	else 
	{
		for (int i = 0; i < 8; i++) 
		{
			if (childBounds[i]->contains(item)) 
			{
				if (childs[i] != nullptr)
				{
					bool successfullyRemoved = childs[i]->remove(item, (currentDepth + 1), maxDepth);

					if (childs[i]->getElementCount() <= 0 && childs[i]->getActiveChildCount() <= 0)	
					{
						delete childs[i];
						childs[i] = nullptr;
					}

					if(successfullyRemoved)
						return true;
				}
			}
		}

		// We have test all childs without finding the item
		return false;
	}
}

template<typename ItemType, typename spatialKeyType>
ItemType*  OctreeNode<ItemType, spatialKeyType>::find(const spatialKeyType& spatialKey, int currentDepth, int maxDepth) const
{
	//max depth reached : 
	if (currentDepth >= maxDepth)
	{
		auto findIt = std::find(spatialKeys.begin(), spatialKeys.end(), spatialKey);
		if (findIt != spatialKeys.end())
			return elements[findIt - spatialKeys.begin()];
		else
			return nullptr;
	}

	//max depth not reached, we test childs :
	glm::vec3 nodeCenter = currentNode->center;
	float nodeHalfSize = currentNode->halfSize;

	for (int i = 0; i < 8; i++)
	{
		if (childBounds[i]->contains(spatialKey))
		{
			return childs[i]->find(spatialKey, (currentDepth + 1), maxDepth);
		}
	}
	//none of childs contains the current spatialKey, and max depth not reached (shouldn't happend for insertion based only on position...) :
	auto findIt = std::find(spatialKeys.begin(), spatialKeys.end(), spatialKey);
	if (findIt != spatialKeys.end())
		return elements[findIt - spatialKeys.begin()];
	else
		return nullptr;
}

template<typename ItemType, typename spatialKeyType>
void  OctreeNode<ItemType, spatialKeyType>::findAll(const spatialKeyType& spatialKey, int currentDepth, int maxDepth, std::vector<ItemType*>& results) const
{
	//max depth reached :
	if (currentDepth >= maxDepth)
	{
		auto findIt = std::find(spatialKeys.begin(), spatialKeys.end(), spatialKey);
		if (findIt != spatialKeys.end())
			results.push_back( elements[findIt - spatialKeys.begin()] );

		return;
	}

	//max depth not reached, we test childs :
	glm::vec3 nodeCenter = currentNode->center;
	float nodeHalfSize = currentNode->halfSize;

	for (int i = 0; i < 8; i++)
	{
		if (childBounds[i]->contains(spatialKey))
		{
			return childs[i]->findAll(spatialKey, (currentDepth + 1), maxDepth, results);
		}
	}
	//none of childs contains the current spatialKey, and max depth not reached (shouldn't happend for insertion based only on position...) :
	auto findIt = std::find(spatialKeys.begin(), spatialKeys.end(), spatialKey);
	if (findIt != spatialKeys.end())
		results.push_back(elements[findIt - spatialKeys.begin()]);
}


template<typename ItemType, typename spatialKeyType>
void OctreeNode<ItemType, spatialKeyType>::findNeighborsContained(const glm::vec3& center, float radius, int currentDepth, int maxDepth, std::vector<ItemType*>& results) const
{
	//max depth reached (leaf) :
	if (currentDepth >= maxDepth)
	{
		for (int i = 0; i < spatialKeys.size(); i++)
		{
			if (Collisions::containedInSphere(spatialKeys[i], center, radius))
				results.push_back(elements[i]);
		}
		return;
	}

	//max depth not reached, we test childs :
	for (int i = 0; i < 8; i++)
	{
		if (childBounds[i]->containedInSphere(center, radius))
		{
			//child entirely contained in sphere, we take all its items.
			//recusivly get all elements stored in child node and its children : 
			if (childs[i] != nullptr)
				childs[i]->getAllElements(results);
		}
		else if (childBounds[i]->containedInOrIntersectedBySphere(center, radius))
		{
			if (childs[i] != nullptr)
				childs[i]->findNeighborsContained(center, radius, (currentDepth + 1), maxDepth, results);
		}
	}

}

template<typename ItemType, typename spatialKeyType>
void OctreeNode<ItemType, spatialKeyType>::findNeighborsContainedOrIntersected(const glm::vec3& center, float radius, int currentDepth, int maxDepth, std::vector<ItemType*>& results) const
{
	//max depth reached (leaf) :
	if (currentDepth >= maxDepth)
	{
		for (int i = 0; i < spatialKeys.size(); i++)
		{
 			if(Collisions::containedInOrIntersectedBySphere(spatialKeys[i], center, radius))
				results.push_back(elements[i]);
		}
		return;
	}

	//max depth not reached, we test childs :
	for (int i = 0; i < 8; i++)
	{
		if (childBounds[i]->containedInSphere(center, radius))
		{
			//recusivly get all elements stored in child node and its children : 
			if (childs[i] != nullptr)
				childs[i]->getAllElements(results);
		}
		else if (childBounds[i]->containedInOrIntersectedBySphere(center, radius))
		{
			if (childs[i] != nullptr)
				childs[i]->findNeighborsContainedOrIntersected(center, radius, (currentDepth+1), maxDepth, results);
		}
	}
}


//A simple octree with a maximal depth of maxDepth. The depth is the only parameter which influences the insertion of an item in the octree.
template<typename ItemType, typename spatialKeyType>
class Octree
{
private:
	OctreeNode<ItemType, spatialKeyType>* m_root;
	int m_maxDepth;
	glm::vec3 m_center;
	float m_halfSize;

public:
	Octree(const glm::vec3& center, float halfSize, int maxDepth = 3);
	~Octree();

	//add an item at the given spatialKey :
	void add(ItemType* item, const spatialKeyType& spatialKey);
	//remove an item at the given spatialKey :
	bool remove(ItemType* item, const spatialKeyType& spatialKey);
	//remove an item, searchinf the all tree :
	bool remove(ItemType* item);

	//find an item at the given spatialKey :
	ItemType* find(const spatialKeyType& spatialKey) const;
	//find all elements which are at the given spatialKey :
	void findAll(const spatialKeyType& spatialKey, std::vector<ItemType*>& results) const;
	//return all elements near the given spatialKey (inside the radius) :
	void findNeighborsContained(const spatialKeyType& spatialKey, float radius, std::vector<ItemType*>& results) const;
	void findNeighborsContainedOrIncluded(const spatialKeyType& spatialKey, float radius, std::vector<ItemType*>& results) const;
	//get all elements which are visible by the camera modelized by the givent view and projection matrices :
	void findVisibleElements(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, const glm::vec3& cameraForward, std::vector<ItemType*>& results) const;

	//move an item from previousPosition to newPosition in the octree :
	void update(ItemType* item, const spatialKeyType& previousSpatialKey, const spatialKeyType& newSpatialKey);

	//half size of the root :
	float getHalfSize() const;
	//center of the root :
	glm::vec3 getCenter() const;

	void getAllCenterAndSize(std::vector<glm::vec3>& centers, std::vector<float>& halfSizes) const;

	void drawDebug() const;
};


template<typename ItemType, typename spatialKeyType>
Octree<ItemType, spatialKeyType>::Octree(const glm::vec3& center, float halfSize, int maxDepth) : m_maxDepth(maxDepth), m_center(center), m_halfSize(halfSize)
{
	m_root = new OctreeNode<ItemType, spatialKeyType>(center, halfSize);
}

template<typename ItemType, typename spatialKeyType>
Octree<ItemType, spatialKeyType>::~Octree()
{
	delete m_root;
}

template<typename ItemType, typename spatialKeyType>
void Octree<ItemType, spatialKeyType>::add(ItemType * item, const spatialKeyType& spatialKey)
{
	m_root->add(item, spatialKey, 0, m_maxDepth);
}
 
template<typename ItemType, typename spatialKeyType>
bool Octree<ItemType, spatialKeyType>::remove(ItemType * item, const spatialKeyType& spatialKey)
{
	return m_root->remove(item, spatialKey, 0, m_maxDepth);
}

template<typename ItemType, typename spatialKeyType>
bool Octree<ItemType, spatialKeyType>::remove(ItemType* item)
{
	return m_root->remove(item, 0, m_maxDepth);
}

template<typename ItemType, typename spatialKeyType>
ItemType* Octree<ItemType, spatialKeyType>::find(const spatialKeyType& spatialKey) const
{
	return m_root->find(spatialKey, 0, m_maxDepth);
}

template<typename ItemType, typename spatialKeyType>
void Octree<ItemType, spatialKeyType>::findAll(const spatialKeyType& spatialKey, std::vector<ItemType*>& results) const
{
	m_root->findAll(spatialKey, 0, m_maxDepth, results);
}

template<typename ItemType, typename spatialKeyType>
void Octree<ItemType, spatialKeyType>::findNeighborsContained(const spatialKeyType& spatialKey, float radius, std::vector<ItemType*>& results) const
{
	m_root->findNeighborsContained(spatialKey, radius, 0, m_maxDepth, results);
}

template<typename ItemType, typename spatialKeyType>
void Octree<ItemType, spatialKeyType>::findNeighborsContainedOrIncluded(const spatialKeyType& spatialKey, float radius, std::vector<ItemType*>& results) const
{
	m_root->findNeighborsContainedOrIncluded(spatialKey, radius, 0, m_maxDepth, results);
}

template<typename ItemType, typename spatialKeyType>
void Octree<ItemType, spatialKeyType>::findVisibleElements(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition, const glm::vec3& cameraForward, std::vector<ItemType*>& results) const
{
	m_root->findVisibleElements(view, projection, cameraPosition, cameraForward, results, 0, m_maxDepth);
}

template<typename ItemType, typename spatialKeyType>
void Octree<ItemType, spatialKeyType>::update(ItemType * item, const spatialKeyType& previousSpatialKey, const spatialKeyType& newSpatialKey)
{
	m_root->remove(item, previousSpatialKey, 0, m_maxDepth);
	m_root->add(item, newSpatialKey, 0, m_maxDepth);
}

template<typename ItemType, typename spatialKeyType>
float Octree<ItemType, spatialKeyType>::getHalfSize() const
{
	if (m_root != nullptr)
		return m_root->halfSize;
	else
		return 0;
}

template<typename ItemType, typename spatialKeyType>
glm::vec3 Octree<ItemType, spatialKeyType>::getCenter() const
{
	if (m_root != nullptr)
		return m_root->center;
	else
		return glm::vec3(0, 0, 0);
}

template<typename ItemType, typename spatialKeyType>
void Octree<ItemType, spatialKeyType>::getAllCenterAndSize(std::vector<glm::vec3>& centers, std::vector<float>& halfSizes) const
{
	centers.push_back(m_root->center);
	halfSizes.push_back(m_root->halfSize);

	m_root->getAllCenterAndSize(centers, halfSizes);
}

template<typename ItemType, typename spatialKeyType>
inline void Octree<ItemType, spatialKeyType>::drawDebug() const
{
	if (ImGui::TreeNode((void*)this, "Octree"))
	{
		m_root->drawDebug();

		ImGui::TreePop();
	}
}




//
//template<typename ItemType, typename spatialKeyType>
//void OctreeNode<ItemType, spatialKeyType>::getAllElementsContainedOrIntersected(std::vector<ItemType*>& _elements, const glm::vec3& center, float radius)
//{
//	for (int i = 0; i < spatialKeys.size(); i++)
//	{
//		if (OctreeHelper::distanceMin(spatialKeys[i], center) < radius)
//			_elements.push_back(elements[i]);
//	}
//
//	for (int i = 0; i < 8; i++)
//	{
//		if (childs[i] != nullptr)
//		{
//			childs[i]->getAllElementsContainedOrIntersected(_elements, center, radius);
//		}
//	}
//}
//
//template<typename ItemType, typename spatialKeyType>
//void OctreeNode<ItemType, spatialKeyType>::getAllElementsContained(std::vector<ItemType*>& _elements, const glm::vec3& center, float radius)
//{
//	for (int i = 0; i < spatialKeys.size(); i++)
//	{
//		if (OctreeHelper::distanceMax(spatialKeys[i], center) < radius)
//			_elements.push_back(elements[i]);
//	}
//
//	for (int i = 0; i < 8; i++)
//	{
//		if (childs[i] != nullptr)
//		{
//			childs[i]->getAllElementsContained(_elements, center, radius);
//		}
//	}
//}


//template<typename ItemType>
//bool OctreeNode<ItemType>::contains(const glm::vec3& point)
//{
//	//is position inside current node ?
//	return(nodeCenter.x - nodeHalfSize < position.x && nodeCenter.x + nodeHalfSize > position.x &&
//		nodeCenter.y - nodeHalfSize < position.y && nodeCenter.y + nodeHalfSize > position.y &&
//		nodeCenter.z - nodeHalfSize < position.z && nodeCenter.z + nodeHalfSize > position.z);
//}
//
//template<typename ItemType>
//bool OctreeNode<ItemType>::contains(ItemType* item)
//{
//	auto findIt = std::find(elements.begin(), elements.end(), item);
//
//	return (findIt != elements.end());
//}
//
//template<typename ItemType>
//bool OctreeNode<ItemType>::containedIn(const glm::vec3& position, float halfSize)
//{
//	return(nodeCenter.x - nodeHalfSize > position.x - halfSize && nodeCenter.x + nodeHalfSize < position.x + halfSize &&
//		nodeCenter.y - nodeHalfSize > position.y - halfSize && nodeCenter.y + nodeHalfSize < position.y + halfSize &&
//		nodeCenter.z - nodeHalfSize > position.z - halfSize && nodeCenter.z + nodeHalfSize < position.z + halfSize);
//}