#pragma once

#include "glm/common.hpp"

namespace Physic {

	struct Point
	{
		glm::vec3 position;
		glm::vec3 vitesse;
		glm::vec3 force;
		float masse;

		Point();
		Point(glm::vec3 _position);
		Point(glm::vec3 _position, glm::vec3 _vitesse, float _masse);

		void setMasse(double _masse);
		void setForce(glm::vec3 _force);
		void setVitesse(glm::vec3 _vitesse);
		void setPosition(glm::vec3 _position);
	};

}

