#pragma once

template<typename T>
class ISingleton
{
private:
	void operator=(const T& other){}

public:
	static T& instance()
	{
		static T instance;

		return instance;
	}
};

#define SINGLETON_IMPL(T) \
	public:\
	T(const T& other) = delete;\
	const T operator=(const T& other) = delete;\
	private:




