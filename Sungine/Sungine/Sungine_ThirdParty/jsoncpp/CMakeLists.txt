cmake_minimum_required(VERSION 3.0)

project(jsoncpp)

file(GLOB SRC_FILES *.c *.cpp)
set(GLOB HEARDER_FILES *.h *.hpp)

add_library(jsoncpp ${SRC_FILES} ${HEARDER_FILES})

target_include_directories(jsoncpp PUBLIC .)