#pragma once

#include "Animation.h"
#include "BSpline.h"

class SplineAnimation : public Animation
{
private:
	Math::BSpline<float> m_spline;

public:
	SplineAnimation(float duration = 0, bool isLooping = false);
	~SplineAnimation();

	void setIsLooping(bool isLooping);
	void setDuration(float duration);
	float getValue() const;

	Math::BSpline<float>& getSpline();
};

