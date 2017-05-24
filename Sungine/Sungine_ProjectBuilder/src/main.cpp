#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include <cmath>

#include "glew/glew.h"

#include "GLFW/glfw3.h"
#include "stb/stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

#include "InputHandler.h"
#include "Camera.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Materials.h"
#include "LightManager.h"
#include "Application.h"
#include "Collider.h"
#include "Utils.h"
#include "Entity.h"
#include "Editor.h"
#include "Lights.h"
#include "Editor.h"
#include "Ray.h"
#include "Renderer.h"
#include "Scene.h"
#include "Factories.h"
#include "Flag.h"
#include "Scene.h"

#include "jsoncpp/json/json.h"

#include "Project.h"

#include "btBulletDynamicsCommon.h"

#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

#if DEBUG_PRINT == 0
#define debug_print(FORMAT, ...) ((void)0)
#else
#ifdef _MSC_VER
#define debug_print(FORMAT, ...) \
	fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
		__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define debug_print(FORMAT, ...) \
	fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
		__func__, __FILE__, __LINE__, __VA_ARGS__)
#endif
#endif

// Font buffers
extern const unsigned char DroidSans_ttf[];
extern const unsigned int DroidSans_ttf_len;

#include "TestMetaData02.h"

int main(int argc, char** argv)
{
	std::cout << argv[0] << std::endl;

	//Begin Test
	testMetaData();
	//End Test

	//init project : 
	Project project;
	project.init();

	//open default project : 
	project.open(FileHandler::Path(argv[0]));

	//edit or play project : 
	//project.play();
	project.edit();

	//close window : 
	project.exitApplication();
}