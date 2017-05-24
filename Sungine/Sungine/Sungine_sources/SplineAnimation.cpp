
#include "SplineAnimation.h"
//forwards : 
#include "Application.h"

SplineAnimation::SplineAnimation(float duration, bool isLooping): Animation(duration, isLooping)
{
}


SplineAnimation::~SplineAnimation()
{
}


float SplineAnimation::getValue() const
{

	float clampedElapsedTime = m_duration == 0.f ? 0.f : getElapsedTime() / m_duration;

	return m_spline.get(clampedElapsedTime);
}

Math::BSpline<float>& SplineAnimation::getSpline()
{
	return m_spline;
}

void SplineAnimation::setDuration(float duration)
{
	m_duration = duration;
}


void SplineAnimation::setIsLooping(bool isLooping)
{
	m_isLooping = isLooping;
}
