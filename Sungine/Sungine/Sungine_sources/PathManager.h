#pragma once

#include <algorithm>

#include "Path.h"
#include "Materials.h"
#include "Camera.h"
#include "ResourcePointer.h"

class PathManager
{
private:
	ResourcePtr<Material> m_material;
	std::vector<Path*> m_paths;

public:
	PathManager();
	~PathManager();

	void updatePathId(int pathId, int oldPathId, PathPoint* pathPoint);
	void updatePointIdx(int pathId);
	void updateVisual(int pathId);

	Path* findPath(int pathId);
	PathPoint* findPointInPath(int pathId, int pointIdx);

	void add(PathPoint* pathPoint);
	void erase(PathPoint* pathPoint);

	void render(const BaseCamera& camera);
};

