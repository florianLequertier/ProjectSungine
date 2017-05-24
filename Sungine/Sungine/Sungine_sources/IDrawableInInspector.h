#pragma once

#include <vector>

class Scene;

class IDrawableInInspector
{
public:
	virtual void drawInInspector(Scene& scene, const std::vector<IDrawableInInspector*>& selection) = 0;
	virtual void drawInInspector(Scene& scene) = 0;
};