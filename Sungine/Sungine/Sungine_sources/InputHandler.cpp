

#include "InputHandler.h"

GLFWwindow* InputHandler::m_window = nullptr;
std::map<int, bool> InputHandler::previousMouseState;
std::map<int, bool> InputHandler::previousKeyboardState;
InputHandler::FocusStates InputHandler::m_focusState = InputHandler::FOCUSING_ALL;
std::vector<KeyInput> InputHandler::keyInputs;


InputHandler::InputHandler()
{
	setFocusState(FocusStates::FOCUSING_ALL);
	attachToWindow(nullptr);
}

bool InputHandler::getMouseButtonUp(int button, FocusStates focusContext)
{
	return (!glfwGetMouseButton(m_window, button) && previousMouseState[button]) && ((focusContext & m_focusState) != FocusStates::FOCUSING_NONE);
}

bool InputHandler::getMouseButton(int button, FocusStates focusContext)
{
	return glfwGetMouseButton(m_window, button) && ((focusContext & m_focusState) != FocusStates::FOCUSING_NONE);
}

bool InputHandler::getMouseButtonDown(int button, FocusStates focusContext)
{
	return (glfwGetMouseButton(m_window, button) && !previousMouseState[button]) && ((focusContext & m_focusState) != FocusStates::FOCUSING_NONE);
}

bool InputHandler::getKeyUp(int key, FocusStates focusContext)
{
	return (!glfwGetKey(m_window, key) && previousKeyboardState[key]) && ((focusContext & m_focusState) != FocusStates::FOCUSING_NONE);
}

bool InputHandler::getKey(int key, FocusStates focusContext)
{
	return glfwGetKey(m_window, key) && ((focusContext & m_focusState) != FocusStates::FOCUSING_NONE);
}

bool InputHandler::getKeyDown(int key, FocusStates focusContext)
{
	return (glfwGetKey(m_window, key) && !previousKeyboardState[key]) && ((focusContext & m_focusState) != FocusStates::FOCUSING_NONE);
}

void InputHandler::consumeKeyInput(int key)
{
	previousKeyboardState[key] = glfwGetKey(m_window, key);
}

void InputHandler::consumeMouseInput(int button)
{
	previousMouseState[button] = glfwGetMouseButton(m_window, button);
}

InputHandler::FocusStates InputHandler::getFocusState()
{
	return m_focusState;
}

void InputHandler::setFocusState(FocusStates state)
{
	m_focusState = state;
}

void InputHandler::attachToWindow(GLFWwindow * window)
{
	m_window = window;
}

void InputHandler::pushInputsToReceivers(IInputReceiver& receiver, bool clearAllInputAfterTreatment)
{
	for (KeyInput& keyInput : keyInputs)
	{
		if (!keyInput.consumed)
			receiver.handleKeyInput(keyInput);
	}

	if (clearAllInputAfterTreatment)
	{
		keyInputs.clear();
	}
	else
	{
		for (auto it = keyInputs.begin(); it != keyInputs.end();)
		{
			if (it->consumed)
			{
				if (it == keyInputs.end() - 1)
				{
					keyInputs.pop_back();
					it = keyInputs.end();
				}
				else
				{
					std::iter_swap(it, keyInputs.end() - 1);
					keyInputs.pop_back();
				}
			}
			else
				it++;
		}
	}
}

void InputHandler::pullInputsFromWindow()
{
	// Clear inputs
	keyInputs.clear();

	synchronize();
}

void InputHandler::addKeyInput(const KeyInput& keyInput)
{
	keyInputs.push_back(keyInput);
}

void InputHandler::synchronize()
{
	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
	{
		previousMouseState[i] = glfwGetMouseButton(m_window, i);
	}
	for (int i = 0; i < GLFW_KEY_LAST; i++)
	{
		previousKeyboardState[i] = glfwGetKey(m_window, i);
	}
}

void InputHandler::keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	addKeyInput(KeyInput(key, scancode, action, mods));
}

//void InputHandler::synchronize()
//{
//	get()._synchronize();
//}
//bool InputHandler::getMouseButtonUp(int button, FocusStates focusContext)
//{
//	return get()._getMouseButtonUp(button);
//}
//
//bool InputHandler::getMouseButton(int button, FocusStates focusContext)
//{
//	return get()._getMouseButton(button);
//}
//
//bool InputHandler::getMouseButtonDown(int button, FocusStates focusContext)
//{
//	return get()._getMouseButtonDown(button);
//}
//
//bool InputHandler::getKeyUp(int key, FocusStates focusContext)
//{
//	return get()._getKeyUp(key);
//}
//
//bool InputHandler::getKey(int key, FocusStates focusContext)
//{
//	return get()._getKey(key);
//}
//
//bool InputHandler::getKeyDown(int key, FocusStates focusContext)
//{
//	return get()._getKeyDown(key);
//}
//
//InputHandler::FocusStates InputHandler::getFocusState()
//{
//	return get()._getFocusState();
//}
//
//void InputHandler::setFocusState(InputHandler::FocusStates state)
//{
//	return get()._setFocusState(state);
//}
//
//void InputHandler::attachToWindow(GLFWwindow * window)
//{
//	get()._attachToWindow(window);
//}

//void InputHandler::_synchronize()
//{
//	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
//	{
//		previousMouseState[i] = glfwGetMouseButton(m_window, i);
//	}
//	for (int i = 0; i < GLFW_KEY_LAST; i++)
//	{
//		previousKeyboardState[i] = glfwGetKey(m_window, i);
//	}
//}
//
//bool InputHandler::_getMouseButtonUp(int button)
//{
//	return (!glfwGetMouseButton(m_window, button) && previousMouseState[button]);
//}
//
//bool InputHandler::_getMouseButton(int button)
//{
//	return glfwGetMouseButton(m_window, button);
//}
//
//bool InputHandler::_getMouseButtonDown(int button)
//{
//	return (glfwGetMouseButton(m_window, button) && !previousMouseState[button]);
//}
//
//bool InputHandler::_getKeyUp(int key)
//{
//	return (!glfwGetKey(m_window, key) && previousKeyboardState[key]);
//}
//
//bool InputHandler::_getKey(int key)
//{
//	return glfwGetKey(m_window, key);
//}
//
//bool InputHandler::_getKeyDown(int key)
//{
//	return (glfwGetKey(m_window, key) && !previousKeyboardState[key]);
//}
//
//InputHandler::FocusStates InputHandler::_getFocusState()
//{
//	return m_focusState;
//}
//
//void InputHandler::_setFocusState(InputHandler::FocusStates state)
//{
//	m_focusState = state;
//}
//
//void InputHandler::_attachToWindow(GLFWwindow * window)
//{
//	m_window = window;
//}
