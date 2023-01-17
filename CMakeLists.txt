
cmake_minimum_required(VERSION "3.0.0")

project(E4 VERSION 0.1.0)

add_subdirectory(glfw/)
include_directories(glfw/include)

file(GLOB_RECURSE SRC_FILES src/*.c)
file(GLOB_RECURSE RES_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/out/*.o)

add_library(E4 ${SRC_FILES} ${RES_FILES})

target_include_directories(E4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

