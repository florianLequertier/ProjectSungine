cmake_minimum_required(VERSION 3.0)

#set(PROJECT_NAME "MyProject" CACHE STRING "the name of your project")
#option(PROJECT_NAME "the name of your project" MyProject)

project(Sungine)

set(EXECUTABLE_NAME Sungine_exe)
set(LIBRARY_NAME Sungine)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(EXTERNAL_LIB_PATH ${ENGINE_ROOT_PATH}/Sungine_ThirdParty)
set(SOURCES_PATH ${ENGINE_ROOT_PATH}/Sungine_sources)

#add_definitions(-lstdc++)
add_definitions(-std=c++11)

# set(CMAKE_CXX_FLAGS -std=c++11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_VERBOSE_MAKEFILE 1)
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)

###Add files to project
file(GLOB HEADER_FILES ${SOURCES_PATH}/*.hpp ${SOURCES_PATH}/*.h)
file(GLOB SRC_FILES ${SOURCES_PATH}/*.cpp)

add_library(${LIBRARY_NAME} ${SRC_FILES} ${HEADER_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES
                      FOLDER "Sungine/Engine")
			
###Add preprocessor option to link GLEW lib			
target_compile_definitions(${LIBRARY_NAME} PRIVATE GLEW_STATIC)
#####################################


###Add externals libraries to project : 

include_directories(${EXTERNAL_LIB_PATH})
include_directories(${EXTERNAL_LIB_PATH}/assimp/include)
include_directories(${EXTERNAL_LIB_PATH}/boost/preprocessor)
include_directories(${EXTERNAL_LIB_PATH}/bullet3/src)
include_directories(${EXTERNAL_LIB_PATH}/cereal)
include_directories(${EXTERNAL_LIB_PATH}/glfw/include)
include_directories(${EXTERNAL_LIB_PATH}/glew)
include_directories(${EXTERNAL_LIB_PATH}/glm)
include_directories(${EXTERNAL_LIB_PATH}/jsoncpp)
include_directories(${EXTERNAL_LIB_PATH}/libhello)
include_directories(${EXTERNAL_LIB_PATH}/stb)

#libhello
add_subdirectory(${EXTERNAL_LIB_PATH}/libhello)

#imgui
add_subdirectory(${EXTERNAL_LIB_PATH}/imgui)

#glew
add_subdirectory(${EXTERNAL_LIB_PATH}/glew)

#glfw
add_subdirectory(${EXTERNAL_LIB_PATH}/glfw)

#bullet
add_subdirectory(${EXTERNAL_LIB_PATH}/bullet3) # => BulletDynamics BulletCollision LinearMath

#glm
add_subdirectory(${EXTERNAL_LIB_PATH}/glm)

#jsoncpp
add_subdirectory(${EXTERNAL_LIB_PATH}/jsoncpp)

#stb
add_subdirectory(${EXTERNAL_LIB_PATH}/stb)

#assimp
add_subdirectory(${EXTERNAL_LIB_PATH}/assimp)

if(WIN32)

	#glfw
	# set(GLFW_INCLUDE_DIR 
		# ${EXTERNAL_LIB_PATH}/glfw/include
		# )
	# set(GLFW_LIBRARY 
		# ${EXTERNAL_LIB_PATH}/glfw/libglfw3dll.a
		# )
	
	
	#bullet
	# set(BULLET_INCLUDE_DIR 
		# ${EXTERNAL_LIB_PATH}/bullet3/src
		# )
	# set(BULLET_LIBRARY 
		# ${EXTERNAL_LIB_PATH}/bullet3/libBulletCollision.dll.a
		# ${EXTERNAL_LIB_PATH}/bullet3/libBulletDynamics.dll.a
		# ${EXTERNAL_LIB_PATH}/bullet3/libLinearMath.dll.a
		# )

	#zlib
	set(ZLIB_INCLUDE_DIR 
		${EXTERNAL_LIB_PATH}/zlib/include
		)
	set(ZLIB_LIBRARY 
		${EXTERNAL_LIB_PATH}/zlib/libzlib.dll.a
		)
		
	#assimp
	# set(ASSIMP_INCLUDE_DIR 
		# ${EXTERNAL_LIB_PATH}/assimp/include
		# )
	# set(ASSIMP_LIBRARY 
		# ${EXTERNAL_LIB_PATH}/assimp/libassimp.dll.a
		# )
		
	set(DLL_BINARIES 
		# ${EXTERNAL_LIB_PATH}/assimp/libassimp.dll
		${EXTERNAL_LIB_PATH}/zlib/libzlib.dll
		# ${EXTERNAL_LIB_PATH}/bullet3/libBulletCollision.dll
		# ${EXTERNAL_LIB_PATH}/bullet3/libBulletDynamics.dll
		# ${EXTERNAL_LIB_PATH}/bullet3/libLinearMath.dll
		# #${EXTERNAL_LIB_PATH}/glfw/libglfw3dll
		)
	
else(WIN32)

	# find_package( ZLIB REQUIRED )
	# if ( ZLIB_FOUND )
		# include_directories( ${ZLIB_INCLUDE_DIRS} )
		# target_link_libraries( ${EXECUTABLE_NAME} ${ZLIB_LIBRARIES} )
	# endif( ZLIB_FOUND )
	
	# find_package(bullet REQUIRED)
endif(WIN32)

set(ALL_LIBS 
			assimp
			BulletDynamics BulletCollision LinearMath BulletInverseDynamics Bullet3Common Bullet3Collision Bullet3Dynamics Bullet3Geometry Bullet3OpenCL_clew Bullet2FileLoader
			glew
			glfw
			glm_lib
			hello
			imgui
			jsoncpp
			stb
			)#${ZLIB_LIBRARY})# # ${ASSIMP_LIBRARY} # ${BULLET_LIBRARY} # ${GLFW_LIBRARY}
			
#target_link_libraries(${EXECUTABLE_NAME} ${ALL_LIBS} ) #-static-libgcc -static-libstdc++ -lstdc++
target_link_libraries(${LIBRARY_NAME} ${ALL_LIBS} )

set_target_properties( 
			assimp 
			BulletSoftBody BulletDynamics BulletCollision LinearMath BulletInverseDynamics Bullet3Common Bullet3Collision Bullet3Dynamics Bullet3Geometry Bullet3OpenCL_clew Bullet2FileLoader
			glew 
			glfw 
			glm 
			hello
			imgui 
			jsoncpp 
			stb
			zlibstatic 
			PROPERTIES FOLDER "Sungine/ThirdParties")

if(WIN32)
    # on windows, copy dll files to the executable directory
    file(COPY ${DLL_BINARIES} DESTINATION ${PROJECT_EXECUTABLE_OUTPUT_PATH})
endif(WIN32)

#####################################