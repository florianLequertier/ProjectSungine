cmake_minimum_required(VERSION 3.0)

project(libhello)

set(SRCS hello.cpp)
set(HEADERS hello.h)

add_library(hello ${SRCS} ${HEADERS})

target_include_directories(hello PUBLIC .)