cmake_minimum_required(VERSION 3.0)

project(imgui)

file(GLOB SRC_FILES *.cpp)
set(GLOB_RECURSE HEARDER_FILES *.h *.hpp)

add_library(imgui STATIC ${SRC_FILES} ${HEARDER_FILES})

target_include_directories(imgui PUBLIC .)