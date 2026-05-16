cmake_minimum_required(VERSION 3.24)
project(InfiniteTTTViewer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)

add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

add_executable(infinite_ttt src/main.cpp)
target_link_libraries(infinite_ttt PRIVATE SDL3::SDL3 glm::glm glad)
