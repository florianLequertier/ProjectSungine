

#include "Link.h"

namespace Physic {

	Link::Link() : M1(nullptr), M2(nullptr), k(0), z(0), l(0), s1(0), s2(0), s3(0)
	{

	}

	Link::Link(Point* _M1, Point* _M2): M1(_M1), M2(_M2), k(0), z(0), l(0), s1(0), s2(0), s3(0)
	{

	}

	Link::Link(Point* _M1, Point* _M2, float _k, float _z, float _l, float _s1, float _s2, float _s3) : M1(_M1), M2(_M2), k(_k), z(_z), l(_l), s1(_s1), s2(_s2), s3(_s3)
	{

	}

	Link::~Link()
	{
		M1 = nullptr;
		M2 = nullptr;
	}

	void Link::setConnection(Point* _M1, Point* _M2)
	{
		M1 = _M1;
		if (M1 == nullptr)
			std::cout << "warning : M1 est nul dans une connection." << std::endl;
		M2 = _M2;
		if (M2 == nullptr)
			std::cout << "warning : M2 est nul dans une connection." << std::endl;
	}

	void Link::setParameters(float _k, float _z, float _l, float _s1, float _s2, float _s3)
	{
		k = _k;
		z = _z;
		l = _l;
		s1 = _s1;
		s2 = _s2;
		s3 = _s3;
	}

}
