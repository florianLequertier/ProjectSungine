#pragma once

#include <algorithm>
#include <vector>
#include "PathPoint.h"
#include "CSpline.h"
#include "Mesh.h"

class Path
{
private:

	Mesh m_mesh;
	int m_pathId;
	std::vector<PathPoint*> m_pathPoints;

public:
	Path(int pathId = 0);
	~Path();

	std::vector<PathPoint*>::iterator begin();
	std::vector<PathPoint*>::iterator end();

	void push_back(PathPoint* p);
	void insert(int idx, PathPoint* p);
	void pop_back();
	void erase(int idx);
	void erase(PathPoint* p);

	void clear();
	void append(const std::vector<PathPoint*>& points);

	int size();

	//sort by pointIdx
	void sort();

	//return the position of the idx control point
	PathPoint* getPathPoint(int idx) const;

	//return the position in the curve, betwwen begin and end, at position t (0 <= t < 1).
	glm::vec3 get(float t) const;

	int getPathId() const;
	void setPathId(int id);

	void updateVisual();
	void draw();
};

