cmake_minimum_required(VERSION 3.24)

project(TicTacToe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG)
add_subdirectory(external/glm)
add_library(glm_system INTERFACE)
target_link_libraries(glm_system INTERFACE glm::glm)
target_include_directories(glm_system SYSTEM INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/glm>"
)

add_executable(TicTacToe
    src/main.cpp
    src/Renderer.cpp
    src/TicTacToe.cpp
)

target_link_libraries(TicTacToe PRIVATE
    glm_system
    SDL3::SDL3
)

target_compile_options(TicTacToe PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()