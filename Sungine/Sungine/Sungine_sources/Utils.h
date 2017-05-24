#pragma once

#include "glew/glew.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr
#include "GLFW/glfw3.h"

#include "assimp/matrix4x4.h"

#include <vector>
#include <map>
#include <sstream>
#include <iostream>

struct BaseCamera;
#include "Point.h"
#include "Link.h"
#include "FileHandler.h"


#define ENUM_MASK_IMPLEMENTATION(T)\
inline T operator~ (T a) { return (T)~(int)a; }\
inline T operator| (T a, T b) { return (T)((int)a | (int)b); }\
inline T operator& (T a, T b) { return (T)((int)a & (int)b); }\
inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }\
inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }\
inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }\
inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

namespace patch
{
	template < typename T > std::string to_string(const T& n)
	{
		std::ostringstream stm;
		stm << n;
		return stm.str();
	}
}

//conversion flag bit to int :
int flagBitToInt(int flagBit);

//assimp to glm conversion : 
glm::mat4 assimpMat4ToglmMat4(const aiMatrix4x4& aiMat);
void assimpMat4ToglmMat4(const aiMatrix4x4 & aiMat, glm::mat4& glMat);

// Shader utils
int check_link_error(GLuint program);
int check_compile_error(GLuint shader, const char ** sourceBuffer);
void fillShaderStream(std::stringstream& stream, const FileHandler::CompletePath& shaderPath);
GLuint compile_shader_from_string(GLenum shaderType, const std::string& source);
GLuint compile_shader(GLenum shaderType, const char * sourceBuffer, int bufferSize);
GLuint compile_shader_from_file(GLenum shaderType, const char * fileName);

// OpenGL utils
//bool checkError(const char* title);

//return the point in world coordinate matching the given mouse position in screen space : 
glm::vec3 screenToWorld(float mouse_x, float mouse_y, int width, int height, BaseCamera& camera);

double interpolation_cos2D(double a, double b, double c, double d, double x, double y);
double interpolation_cos1D(double a, double b, double x);

glm::vec3 vertexFrom3Floats(const std::vector<float>& vertices, int indice);

//array :
int idx2DToIdx1D(int i, int j, int array2DWidth);

//physic : 
bool rayOBBoxIntersect(glm::vec3 Start, glm::vec3 Dir, glm::vec3 P, glm::vec3 H[3], glm::vec3 E, float* t);
bool raySlabIntersect(float start, float dir, float min, float max, float* tfirst, float* tlast);

namespace Physic {

	void computeLink(float deltaTime, Link* link);
	void computePoint(float deltaTime, Point* point);
}

//resize a 2D vector without losing information already stored in it
template<typename T>
inline void resize2DArray(std::vector<T>& container, int width, int height, int newWidth, int newHeight)
{
	if (newWidth == width)
	{
		container.resize(newWidth * newHeight);
		return;
	}

	if (newWidth > width)
	{
		container.resize(newWidth*newHeight);
		for (int i = ((newWidth - width) + (std::abs(newHeight - height)*width)); i > (newWidth - width); i--)
		{
			for (int k = 0; k < width; k++, i--)
				container[i] = container[i - width];
			i -= (newWidth - width);
		}
	}
	else
	{
		for (int i = newWidth; i < newWidth * newHeight; i++)
		{
			for (int k = 0; k < newWidth; k++, i++)
				container[i] = container[i + (width - newWidth)];
			i += (width - newWidth);
		}
		container.resize(newWidth*newHeight);
	}
}

class AutoCompletion
{
private:
	int m_currentItem;
	std::vector<const char*> m_words;
	bool m_isOpen;
	bool m_downKey;
	bool m_upKey;
	bool m_autocompletionJustClosed;
	bool m_isActive;
	bool m_hasFocus;
	bool m_itemSelected;

public:
	void addWord(std::string word);
	void clearWords();
	bool apply(std::string& result);
	bool apply(char* result);
	void toggleOpen();
	bool getIsOpen();
	void setIsOpen(bool state);
	bool isAutocompletionJustClosed() const;
	bool getIsActive() const;
	void setIsActive(bool state);
	bool getHasFocus() const;
	void setHasFocus(bool state);
	bool getItemSelected() const;
	void setItemSelected(bool state);

private:
	AutoCompletion();
public:
	inline static AutoCompletion& get()
	{
		static AutoCompletion instance;

		return instance;
	}


	AutoCompletion(const AutoCompletion& other) = delete;
	void operator=(const AutoCompletion& other) = delete;
};
