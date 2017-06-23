cmake_minimum_required(VERSION 3.0)

add_definitions(-std=c++11)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_VERBOSE_MAKEFILE 1)
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)

###Set variables
set(PROJECT_NAME "MyProject" CACHE STRING "the name of your project")
project(${PROJECT_NAME})

set(ENGINE_ROOT_PATH ${CMAKE_SOURCE_DIR}/../Sungine CACHE STRING "the path to engine library directory")

set(ENGINE_LIBRARY_PATH ${ENGINE_ROOT_PATH}/Sungine_build)
set(ENGINE_INCLUDES_PATH ${ENGINE_ROOT_PATH}/Sungine_sources)
set(ENGINE_THIRD_PARTY_PATH ${ENGINE_ROOT_PATH}/Sungine_ThirdParty)
set(ENGINE_DEFAULT_RESOURCES_PATH ${ENGINE_ROOT_PATH}/Sungine_defaultResources/)

set(EXECUTABLE_NAME ${PROJECT_NAME})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(PROJECT_EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})
set(PROJECT_SOURCES ${PROJECT_BINARY_DIR}/src)
set(PROJECT_DEFAULT_RESOURCES ${PROJECT_BINARY_DIR}/engineResources)
set(PROJECT_SCENES ${PROJECT_BINARY_DIR}/scenes)
set(PROJECT_ASSETS ${PROJECT_BINARY_DIR}/assets)
set(PROJECT_INFO_DIR ${PROJECT_BINARY_DIR}/projectInfos)
#####################################

##Remove Zero-check project
set(CMAKE_SUPPRESS_REGENERATION true)
#####################################

###Add files to project
file(GLOB HEADER_FILES_TEMPLATE src/*.hpp src/*.h)
file(GLOB SRC_FILES_TEMPLATE src/*.cpp)

file(COPY ${HEADER_FILES_TEMPLATE} DESTINATION ${PROJECT_SOURCES})
file(COPY ${SRC_FILES_TEMPLATE} DESTINATION ${PROJECT_SOURCES})

file(GLOB HEADER_FILES ${PROJECT_SOURCES}/*.hpp ${PROJECT_SOURCES}/*.h)
file(GLOB SRC_FILES ${PROJECT_SOURCES}/*.cpp)

add_executable(${EXECUTABLE_NAME} ${SRC_FILES} ${HEADER_FILES})
#####################################

###Copy default resources to project folder
file(COPY ${ENGINE_DEFAULT_RESOURCES_PATH} DESTINATION ${PROJECT_DEFAULT_RESOURCES})
#####################################

###Create empty directories for assets, project infos and scenes
file(MAKE_DIRECTORY ${PROJECT_SCENES})
file(MAKE_DIRECTORY ${PROJECT_ASSETS})
file(MAKE_DIRECTORY ${PROJECT_INFO_DIR})
#####################################

###Add externals libraries to project : 

include_directories(${ENGINE_THIRD_PARTY_PATH})
include_directories(${ENGINE_THIRD_PARTY_PATH}/assimp/include)
include_directories(${ENGINE_THIRD_PARTY_PATH}/boost/preprocessor)
include_directories(${ENGINE_THIRD_PARTY_PATH}/bullet3/src)
include_directories(${ENGINE_THIRD_PARTY_PATH}/cereal)
include_directories(${ENGINE_THIRD_PARTY_PATH}/glfw/include)
include_directories(${ENGINE_THIRD_PARTY_PATH}/glew)
include_directories(${ENGINE_THIRD_PARTY_PATH}/glm)
include_directories(${ENGINE_THIRD_PARTY_PATH}/jsoncpp)
include_directories(${ENGINE_THIRD_PARTY_PATH}/libhello)
include_directories(${ENGINE_THIRD_PARTY_PATH}/stb)

include_directories(${ENGINE_INCLUDES_PATH})

add_subdirectory(${ENGINE_ROOT_PATH} ${ENGINE_LIBRARY_PATH})

set(ALL_LIBS 
			glu32.lib
			opengl32.lib 
			Sungine)
			
target_link_libraries( ${EXECUTABLE_NAME} ${ALL_LIBS} )

#####################################