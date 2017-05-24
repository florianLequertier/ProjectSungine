cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if (NOT CMAKE_VERSION VERSION_LESS "3.1")
	cmake_policy(SET CMP0054 NEW)
endif()

project(glm)
set(GLM_VERSION "0.9.8")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

enable_testing()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

option(GLM_STATIC_LIBRARY_ENABLE "GLM static library" OFF)
if(GLM_STATIC_LIBRARY_ENABLE)
	message(STATUS "GLM is a header only library, no need to build it. Set the option GLM_STATIC_LIBRARY_ENABLE with ON to build an optional static library")
endif()

option(GLM_DYNAMIC_LIBRARY_ENABLE "GLM static library" OFF)
if(GLM_DYNAMIC_LIBRARY_ENABLE)
	message(STATUS "GLM is a header only library, no need to build it. Set the option GLM_DYNAMIC_LIBRARY_ENABLE with ON to build an optional dynamic library")
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
	#add_definitions(-S)
	#add_definitions(-s)
	add_definitions(-O2)

	#add_definitions(-fprofile-arcs -ftest-coverage) gcov
	#ctest_enable_coverage()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(/FAs)
endif()

include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(glm)

set(GLM_INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/glm")
install(DIRECTORY glm DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/glmConfigVersion.cmake"
    VERSION ${GLM_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# build tree package config
configure_file(
    cmake/glmBuildConfig.cmake.in
    glmConfig.cmake
    @ONLY
)

# install tree package config
configure_package_config_file(
    cmake/glmConfig.cmake.in
    ${GLM_INSTALL_CONFIGDIR}/glmConfig.cmake
    INSTALL_DESTINATION ${GLM_INSTALL_CONFIGDIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES 
        "${CMAKE_CURRENT_BINARY_DIR}/${GLM_INSTALL_CONFIGDIR}/glmConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/glmConfigVersion.cmake"
    DESTINATION ${GLM_INSTALL_CONFIGDIR}
)

if (NOT CMAKE_VERSION VERSION_LESS "3.0")
    add_library(glm_lib INTERFACE)
    target_include_directories(glm_lib INTERFACE
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
    install(TARGETS glm_lib EXPORT glmTargets)

    export(
        EXPORT glmTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/glmTargets.cmake"
    )
    
    install(
        EXPORT glmTargets FILE glmTargets.cmake
        DESTINATION ${GLM_INSTALL_CONFIGDIR}
    )
endif()

# build pkg-config file
configure_file(
    "./cmake/glm.pc.in"
    "glm.pc"
    @ONLY
)

# install pkg-config file
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/glm.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

export(PACKAGE glm)
