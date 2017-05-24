#pragma once

#include "BSpline.h"

class Animation
{
protected:
	bool m_isPlaying;
	bool m_isPaused;
	float m_pauseTime;
	float m_pauseDuration;
	float m_beginTime;
	float m_duration;
	bool m_isLooping;

public:
	Animation(float duration = 0, bool isLooping = false);
	~Animation();

	virtual void play();
	virtual void pause();
	virtual void stop();
	virtual float getDuration() const;
	virtual bool getIsLooping() const;
	virtual float getElapsedTime() const;
	virtual float getRemainingTime() const;
	virtual float getElapsedTimeRatio() const;
	virtual float getRemainingTimeRatio() const;
};

