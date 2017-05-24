#pragma once

#include <functional>

class BaseCoroutine 
{
public:
	BaseCoroutine();
	virtual ~BaseCoroutine();

	virtual float getNextExecutionTime() = 0;
	virtual void execute(float t) = 0;
};

template<typename  R, typename ... Args>
class Coroutine : public BaseCoroutine
{
private:
	std::function<R(Args...)> m_action;
	float m_callDeltaTime;
	float m_lastCallTime;

public:
	Coroutine(std::function<R(Args...)> action, float callDeltaTime = 0);
	virtual ~Coroutine();
	virtual float getNextExecutionTime() override;
	virtual void execute(float t) override;

	friend bool operator<(const Coroutine& a, const Coroutine& b);

};

template<typename R, typename... Args>
bool operator<(const Coroutine<R, Args...>& a, const Coroutine<R, Args...>& b)
{
	return a.getNextExecutionTime() < b.getNextExecutionTime();
}


template<typename  R, typename... Args>
Coroutine<R, Args...>::Coroutine(std::function<R(Args...)> action, float callDeltaTime): m_action(action), m_callDeltaTime(callDeltaTime), m_lastCallTime(0)
{
}

template<typename  R, typename... Args>
Coroutine<R, Args...>::~Coroutine()
{
}

template<class R, class... Args>
float Coroutine<R, Args...>::getNextExecutionTime()
{
	return m_lastCallTime + m_callDeltaTime;
}

template<class R, class... Args>
void Coroutine<R, Args...>::execute(float t)
{
	m_action();
	m_lastCallTime = t;
}
