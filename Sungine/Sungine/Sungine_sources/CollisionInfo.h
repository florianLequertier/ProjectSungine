#pragma once

#include "glm/glm.hpp"
#include "glm/common.hpp"

class Rigidbody;

// Store infos about the collision
struct CollisionInfo
{
	Rigidbody* receiver;
	Rigidbody* rigidbody;
	glm::vec3 point;
	glm::vec3 normal;

	CollisionInfo(Rigidbody* _rigidbody = nullptr, const glm::vec3& _point = glm::vec3(0, 0, 0), const glm::vec3& _normal = glm::vec3(0, 0, 0));
};