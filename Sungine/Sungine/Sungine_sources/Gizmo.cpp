

#include "Gizmo.h"
#include "Editor.h"

Gizmo::Gizmo(ResourcePtr<MaterialInstance> _material, Editor* _editor) 
	: position(0,0,0)
	, material(_material)
	, mesh(GL_TRIANGLES, (Mesh::USE_INDEX | Mesh::USE_VERTICES))
	, editor(_editor)
{
	//init mesh
	mesh.triangleIndex = { 0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 19, 17, 20, 21, 22, 23, 24, 25, 26, };
	mesh.vertices = { -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5 };
	mesh.initGl();

	for (int i = 0; i < 3; i++)
	{
		collider[i].setVisual(&mesh, material);
	}

	collider[0].applyScale(glm::vec3(2.f, 0.2f, 0.2f)); // x
	collider[1].applyScale(glm::vec3(0.2f, 2.f, 0.2f)); // y
	collider[2].applyScale(glm::vec3(0.2f, 0.2f, 2.f)); // z

	collider[0].applyTranslation(glm::vec3(1.f, 0.f, 0.f)); // x
	collider[1].applyTranslation(glm::vec3(0.f, 1.f, 0.f)); // y
	collider[2].applyTranslation(glm::vec3(0.f, 0.f, 1.f)); // z
}

void Gizmo::setTarget(Entity* entity)
{
	if (!targets.empty())
	{
		for (auto t : targets)
		{
			if(t != nullptr)
				t->deselect();
		}
	}

	//set target : 
	targets.clear();
	if (entity == nullptr)
		return;

	targets.push_back(entity);

	for (auto t : targets)
		t->select();

	position = targets.front()->getTranslation();

	//update collider position : 
	for (int i = 0; i < 3; i++)
	{
		collider[i].applyTranslation(position);
	}

	collider[0].appendTranslation(glm::vec3(1.f, 0.f, 0.f)); // x
	collider[1].appendTranslation(glm::vec3(0.f, 1.f, 0.f)); // y
	collider[2].appendTranslation(glm::vec3(0.f, 0.f, 1.f)); // z
}

void Gizmo::setTargets(std::vector<Entity*> entity)
{
	if (!targets.empty())
	{
		for (auto t : targets)
		{
			if(t != nullptr)
				t->deselect();
		}
	}

	//set target : 
	targets.clear();
	if (entity.empty())
		return;

	targets.insert(targets.end(), entity.begin(), entity.end());

	glm::vec3 barycentre(0, 0, 0);
	for (auto t : targets)
	{
		barycentre += t->getTranslation();
		t->select();
	}
	barycentre /= targets.size();

	position = barycentre;
	//update collider position : 
	for (int i = 0; i < 3; i++)
	{
		collider[i].applyTranslation(position);
	}

	collider[0].appendTranslation(glm::vec3(1.f, 0.f, 0.f)); // x
	collider[1].appendTranslation(glm::vec3(0.f, 1.f, 0.f)); // y
	collider[2].appendTranslation(glm::vec3(0.f, 0.f, 1.f)); // z
}

void Gizmo::render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
{
	if (targets.empty())
		return;

	glm::mat4 modelMatrix(1);
	glm::mat4 translation(1);

	translation = glm::translate(glm::mat4(1), position);

	MaterialUnlit* castedMaterial = static_cast<MaterialUnlit*>(material.get()); //TODO : changer ça appres la refonte du pipeline de rendu

	castedMaterial->use();
	//castedMaterial->setUniform_normalMatrix(glm::mat4(1));

	//x : 
	modelMatrix = translation * glm::translate(glm::mat4(1), glm::vec3(scale, 0.f, 0.f)) * glm::scale(glm::mat4(1), glm::vec3(scale*2.f, scale*0.1f, scale*0.1f));
	//castedMaterial->setUniform_MVP(projectionMatrix * viewMatrix * modelMatrix);
	castedMaterial->setUniformModelMatrix(modelMatrix);
	castedMaterial->setUniformViewMatrix(viewMatrix);
	castedMaterial->setUniformProjectionMatrix(projectionMatrix);
	castedMaterial->setUniform_color(glm::vec3(1, 0, 0));
	mesh.draw();

	//y : 
	castedMaterial->setUniform_color(glm::vec3(0, 1, 0));
	modelMatrix = translation * glm::translate(glm::mat4(1), glm::vec3(0.f, scale, 0.f)) * glm::scale(glm::mat4(1), glm::vec3(scale*0.1f, scale*2.f, scale*0.1f));
	//castedMaterial->setUniform_MVP(projectionMatrix * viewMatrix * modelMatrix);
	castedMaterial->setUniformModelMatrix(modelMatrix);
	castedMaterial->setUniformViewMatrix(viewMatrix);
	castedMaterial->setUniformProjectionMatrix(projectionMatrix);
	mesh.draw();

	//z : 
	castedMaterial->setUniform_color(glm::vec3(0, 0, 1));
	modelMatrix = translation * glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, scale)) * glm::scale(glm::mat4(1), glm::vec3(scale*0.1f, scale*0.1f, scale*2.f));
	//castedMaterial->setUniform_MVP(projectionMatrix * viewMatrix * modelMatrix);
	castedMaterial->setUniformModelMatrix(modelMatrix);
	castedMaterial->setUniformViewMatrix(viewMatrix);
	castedMaterial->setUniformProjectionMatrix(projectionMatrix);
	mesh.draw();
}

Gizmo::GizmoArrowType Gizmo::checkIntersection(const Ray & ray, glm::vec3& intersectionPoint)
{
	float t;

	for (int i = 0; i < 3; i++)
	{
		if (collider[i].isIntersectedByRay(ray, &t))
		{
			intersectionPoint = ray.at(t);
			return (Gizmo::GizmoArrowType)i;
		}
	}
	return GIZMO_ARROW_NONE;
}

void Gizmo::translate(const glm::vec3 & t)
{
	if (targets.empty())
		return;

	for (auto target : targets)
		target->translate(t);

	position += t;

	//update collider position : 
	for (int i = 0; i < 3; i++)
	{
		collider[i].applyTranslation(position);
	}

	collider[0].appendTranslation(glm::vec3(1.f, 0.f, 0.f)); // x
	collider[1].appendTranslation(glm::vec3(0.f, 1.f, 0.f)); // y
	collider[2].appendTranslation(glm::vec3(0.f, 0.f, 1.f)); // z
}

void Gizmo::setTranslation(const glm::vec3 & t)
{
	if (targets.empty())
		return;

	for (auto target : targets)
		target->setTranslation(t);

	position = t;

	//update collider position : 
	for (int i = 0; i < 3; i++)
	{
		collider[i].applyTranslation(position);
	}

	collider[0].appendTranslation(glm::vec3(1.f, 0.f, 0.f)); // x
	collider[1].appendTranslation(glm::vec3(0.f, 1.f, 0.f)); // y
	collider[2].appendTranslation(glm::vec3(0.f, 0.f, 1.f)); // z
}

void Gizmo::setScale(float s)
{
	scale = s;


	collider[0].applyScale(glm::vec3(scale*2.f, scale*0.1f, scale*0.1f)); // x
	collider[1].applyScale(glm::vec3(scale*0.1f, scale*2.f, scale*0.1f)); // y
	collider[2].applyScale(glm::vec3(scale*0.1f, scale*0.1f, scale*2.f)); // z

	//update collider position : 
	for (int i = 0; i < 3; i++)
	{
		collider[i].applyTranslation(position);
	}

	collider[0].appendTranslation(glm::vec3(scale, 0.f, 0.f)); // x
	collider[1].appendTranslation(glm::vec3(0.f, scale, 0.f)); // y
	collider[2].appendTranslation(glm::vec3(0.f, 0.f, scale)); // z
}

glm::vec3 Gizmo::getPosition() const
{
	return position;
}

float Gizmo::getScale() const
{
	return scale;
}
