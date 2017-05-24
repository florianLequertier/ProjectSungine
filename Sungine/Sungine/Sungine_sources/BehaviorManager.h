#pragma once

#include <vector>
#include "Behavior.h"

class Entity;

class BehaviorManager
{
public:
	BehaviorManager();
	~BehaviorManager();

	void update(Scene& scene, const std::vector<Behavior*> behaviors);
	void updateCoroutines(const std::vector<Entity*> entities);
};

