cmake_minimum_required(VERSION 3.0)

project(glew)

file(GLOB SRC_FILES *.c)
set(GLOB HEARDER_FILES *.h *.hpp)

add_library(glew STATIC ${SRC_FILES} ${HEARDER_FILES})

target_include_directories(glew PUBLIC .)