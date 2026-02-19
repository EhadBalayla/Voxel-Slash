cmake_minimum_required(VERSION 3.16)
project(VoxelSlash LANGUAGES C CXX)


# Requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(Client)
add_subdirectory(Editor)