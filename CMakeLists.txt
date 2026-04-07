cmake_minimum_required(VERSION 3.16)
project(VoxelSlash LANGUAGES C CXX)
cmake_policy(SET CMP0091 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(Shared)
add_subdirectory(ServerCode)
add_subdirectory(Client)
add_subdirectory(Editor)
add_subdirectory(Server)