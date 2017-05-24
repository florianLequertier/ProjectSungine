#pragma once

#include "glew/glew.h"
#include "stb/stb_image.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>

namespace ErrorHandler {

	inline bool checkError(const char* title)
	{
		int error;
		if ((error = glGetError()) != GL_NO_ERROR)
		{
			std::string errorString;
			switch (error)
			{
			case GL_INVALID_ENUM:
				errorString = "GL_INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				errorString = "GL_INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				errorString = "GL_INVALID_OPERATION";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;
			case GL_OUT_OF_MEMORY:
				errorString = "GL_OUT_OF_MEMORY";
				break;
			default:
				errorString = "UNKNOWN";
				break;
			}
			fprintf(stdout, "OpenGL Error(%s): %s\n", errorString.c_str(), title);
		}
		return error == GL_NO_ERROR;
	}


	inline std::string GLErrorToString(int error)
	{
		if (error != GL_NO_ERROR)
		{
			switch (error)
			{
			case GL_INVALID_ENUM:
				return "GL_INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				return "GL_INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				return "GL_INVALID_OPERATION";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				return "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;
			case GL_OUT_OF_MEMORY:
				return "GL_OUT_OF_MEMORY";
				break;
			default:
				return "UNKNOWN";
				break;
			}
		}
		else
			return "NO_ERROR";
	}

	inline bool printAssert(std::string message)
	{
		std::cerr << "----------------- Assert reached -----------------" << std::endl;
		std::cerr << "- In file : " << __FILE__ << std::endl;
		std::cerr << "- At line : " << __LINE__ << std::endl;
		std::cerr << "- Message : " << message << std::endl;
		return true;
	}

	inline bool printGLAssert(std::string message, int error)
	{
		std::cerr << "----------------- Assert reached, error in OpenGL API -----------------" << std::endl;
		std::cerr << "- In file : " << __FILE__ << std::endl;
		std::cerr << "- At line : " << __LINE__ << std::endl;
		std::cerr << "- GL message : " << ErrorHandler::GLErrorToString(error) << std::endl;
		std::cerr << "- Message : " << message << std::endl;
		return true;
	}

	inline void printFormatedMessage(const char* fmt, va_list argp)
	{
		printf(fmt, argp);
	}

	inline bool checkSTBIError(const char *fmt, ...)
	{
		if (stbi_failure_reason() != nullptr)
		{
			va_list argp;
			va_start(argp, fmt);

			std::cerr << "----------------- Error in STBI image loading -----------------" << std::endl;
			std::cerr << "- In file : " << __FILE__ << std::endl;
			std::cerr << "- At line : " << __LINE__ << std::endl;
			std::cerr << "- STBI message : " << stbi_failure_reason() << std::endl;
			std::cerr << "- Message : ";
			vfprintf(stderr, fmt, argp);
			std::cerr << std::endl << std::endl;

			va_end(argp);
		}

		return true;
	}

	inline void error(const char *fmt, ...)
	{
		va_list argp;
		va_start(argp, fmt);
		vfprintf(stderr, fmt, argp);
		va_end(argp);
	}

}

//ErrorHandler::printSTBIAssert(messageStr, argp);\
// Assert in case of internal error in STBI library
#define CHECK_STBI_ERROR(message, ...)\
{\
	ErrorHandler::checkSTBIError(message, ##__VA_ARGS__);\
}

// Print the opengl error status
#define PRINT_GL_ERROR(error, message)\
{\
	std::cerr << "----------------- Error detected -----------------" << std::endl;\
	std::cerr << "- In file : " << __FILE__ << std::endl;\
	std::cerr << "- At line : " << __LINE__ << std::endl;\
	std::cerr << "- GL message : " << ErrorHandler::GLErrorToString(error) << std::endl; \
	std::cerr << "- Message : "<< message << std::endl;\
}

// Check the opengl error status. If there are errors, it display it on console
#define CHECK_GL_ERROR(message)\
{\
	int error = glGetError();\
	if (error) PRINT_GL_ERROR(error, message);\
}\

// Assert in case of internal error in opengl
#define ASSERT_GL_ERROR(message)\
{\
	int error = glGetError();\
	assert(error == GL_NO_ERROR && ErrorHandler::printGLAssert(message, error));\
}

#define PRINT_WARNING(message)\
{\
	std::cerr << "----------------- Warning -----------------" << std::endl;\
	std::cerr << "- In file : " << __FILE__ << std::endl;\
	std::cerr << "- At line : " << __LINE__ << std::endl;\
	std::cerr << "- Message : " << message << std::endl;\
}

#define PRINT_ERROR(message)\
{\
	std::cerr << "----------------- Error detected -----------------" << std::endl;\
	std::cerr << "- In file : " << __FILE__ << std::endl;\
	std::cerr << "- At line : " << __LINE__ << std::endl;\
	std::cerr << "- Message : " << message << std::endl;\
}

#define ASSERT(instruction, message)\
{\
	assert(instruction && ErrorHandler::printAssert(message));\
}