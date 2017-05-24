

#include "Animation.h"

#include "Application.h"

Animation::Animation(float duration, bool isLooping) : m_isPlaying(false), m_isPaused(false), m_pauseTime(0.f), m_beginTime(0.f), m_pauseDuration(0.f), m_duration(duration), m_isLooping(isLooping)
{
}

Animation::~Animation()
{
}


void Animation::play()
{
	if (m_isPaused)
	{
		m_pauseDuration += Application::get().getTime() - m_pauseTime;
	}
	else
	{
		m_beginTime = Application::get().getTime();
	}

	m_isPaused = false;
	m_isPlaying = true;
}

void Animation::pause()
{
	m_isPaused = true;
	m_pauseTime = Application::get().getTime();
}

void Animation::stop()
{
	m_isPlaying = true;
	m_isPaused = false;
	m_beginTime = 0;
	m_pauseDuration = 0;
	m_pauseTime = 0;
}

float Animation::getElapsedTime() const
{
	if (m_duration == 0)
		return 0;

	float elapsedTime = 0;

	if (m_isPaused)
		elapsedTime = m_pauseTime;
	else
		elapsedTime = Application::get().getTime() - m_beginTime - m_pauseDuration;

	if (elapsedTime > m_duration) {
		if (m_isLooping)
			elapsedTime = fmod(elapsedTime, m_duration);
		else
			elapsedTime = m_duration;
	}

	return elapsedTime;
}

float Animation::getRemainingTime() const
{
	return getDuration() - getElapsedTime();
}

float Animation::getElapsedTimeRatio() const
{
	return getElapsedTime() / (getDuration() + 0.000001);
}

float Animation::getRemainingTimeRatio() const
{
	return getRemainingTime() / (getDuration() + 0.000001);
}

float Animation::getDuration() const
{
	return m_duration;
}

bool Animation::getIsLooping() const
{
	return m_isLooping;
}
