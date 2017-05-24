cmake_minimum_required(VERSION 3.0)

project(stb)

file(GLOB SRC_FILES *.c *.cpp)
set(GLOB HEARDER_FILES *.h *.hpp)

add_library(stb ${SRC_FILES} ${HEARDER_FILES})

target_include_directories(stb PUBLIC .)