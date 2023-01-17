
cmake_minimum_required(VERSION "3.0.0")

project(Test VERSION 0.1.0)


file(GLOB_RECURSE SRC_FILES src/*.c)
add_executable(Test ${SRC_FILES})

target_include_directories(Test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Test E4 glfw GL)

