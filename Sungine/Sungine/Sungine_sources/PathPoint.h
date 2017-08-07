#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "Component.h"

class PathPoint : public Component
{
	COMPONENT_IMPLEMENTATION_HEADER(PathPoint)

	CLASS((PathPoint, Component),
		((PRIVATE)
			(int	, m_pathId		) //the path this point belong to
			(int	, m_pointIdx	) //the index in the current path
		)
	)

private:
	//int m_pathId; //the path this point belong to
	//int m_pointIdx; //the index in the current path

public:
	PathPoint();
	~PathPoint();

	glm::vec3 getPosition();
	int getPointIdx() const;
	int getPathId() const;

	void drawInInspector(Scene& scene) override;
	void drawInInspector(Scene& scene, const std::vector<Component*>& components) override;

	// Hérité via Component
	virtual void save(Json::Value& rootComponent) const override;
	virtual void load(const Json::Value& rootComponent) override;
};

REGISTER_CLASS(PathPoint)