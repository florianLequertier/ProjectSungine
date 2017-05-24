

#include "Application.h"

void Application::setActiveWindow(GLFWwindow* activeWindow)
{
	m_activeWindow = activeWindow;
}

GLFWwindow* Application::getActiveWindow()
{
	return m_activeWindow;
}

void Application::setWindowHeight(int height)
{
	m_windowHeight = height;
}

void Application::setWindowWidth(int width)
{
	m_windowWidth = width;
}

int Application::getWindowHeight()
{
	return m_windowHeight;
}

int Application::getWindowWidth()
{
	return m_windowWidth;
}

bool Application::getWindowResize()
{
	return m_windowResize;
}

void Application::setWindowResize(bool state)
{
	m_windowResize = state;
}

double Application::getTime() const
{
	return glfwGetTime();
}

void Application::setDeltaTime(double deltaTime)
{
	m_deltaTime = deltaTime;
}

double Application::getDeltaTime() const
{
	return m_deltaTime;
}

void Application::setFixedDeltaTime(double deltaTime)
{
	m_fixedDeltaTime = deltaTime;
}

double Application::getFixedDeltaTime() const
{
	return m_fixedDeltaTime;
}

void Application::setFPS(double fps)
{
	m_fps = fps;
}

double Application::getFPS() const
{
	return m_fps;
}

