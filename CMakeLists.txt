
cmake_minimum_required(VERSION "3.5.0")

project(libE4 VERSION 0.1.0)

add_subdirectory(glfw/)
include_directories(glfw/include)

file(GLOB_RECURSE SRC_FILES src/*.c)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/res/out/font.o COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/makeres.sh DEPENDS ${SRC_FILES} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE RES_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/out/*.o)
# CMake requires a file to target for the command above to work. Why??
add_library(libE4 ${SRC_FILES} ${RES_FILES} res/out/font.o)

target_include_directories(libE4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

