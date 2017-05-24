#pragma once

#include <GLFW/glfw3.h>

class Application {

private:
	GLFWwindow* m_activeWindow;

	int m_windowWidth;
	int m_windowHeight;

	bool m_windowResize;

	double m_deltaTime;
	double m_fixedDeltaTime;
	int m_fps;

public:

	void setActiveWindow(GLFWwindow* activeWindow);

	GLFWwindow* getActiveWindow();

	void setWindowHeight(int height);

	void setWindowWidth(int width);

	int getWindowHeight();

	int getWindowWidth();

	bool getWindowResize();

	void setWindowResize(bool state);

	//elapsed time since the beginning of the program, in seconds
	double getTime() const;

	void setDeltaTime(double deltaTime);

	double getDeltaTime() const;

	void setFixedDeltaTime(double deltaTime);

	double getFixedDeltaTime() const;

	void setFPS(double fps);

	double getFPS() const;


	// singleton implementation :
private:
	Application() : m_fixedDeltaTime(1.f/60.f)
	{}

public:
	inline static Application& get()
	{
		static Application instance;

		return instance;
	}


	Application(const Application& other) = delete;
	void operator=(const Application& other) = delete;

};
