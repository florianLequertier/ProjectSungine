

#include "Point.h"

namespace Physic {

	Point::Point() : vitesse(0.f, 0.f, 0.f), masse(0.f), force(0.f, 0.f, 0.f), position(0.f, 0.f, 0.f)
	{

	}

	Point::Point(glm::vec3 _position) : vitesse(0, 0, 0), masse(0.f), force(0.f, 0.f, 0.f), position(_position)
	{

	}

	Point::Point(glm::vec3 _position, glm::vec3 _vitesse, float _masse) : vitesse(_vitesse), masse(_masse), force(0.f, 0.f, 0.f), position(_position)
	{

	}

	void Point::setMasse(double _masse)
	{
		masse = _masse;
	}

	void Point::setForce(glm::vec3 _force)
	{
		force = _force;
	}

	void Point::setVitesse(glm::vec3 _vitesse)
	{
		vitesse = _vitesse;
	}

	void Point::setPosition(glm::vec3 _position)
	{
		position = _position;
	}

}
