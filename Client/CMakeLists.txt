# Link dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB_RECURSE SRC_FILES 
    src/*.cpp 
    src/*.h
    src/*.c
)

add_executable(VoxelSlash_Client ${SRC_FILES})

target_include_directories(VoxelSlash_Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
    ${CMAKE_SOURCE_DIR}/Shared/src
    ${CMAKE_SOURCE_DIR}/Shared/ThirdParty
    ${CMAKE_SOURCE_DIR}/ServerCode/src
)

target_link_libraries(VoxelSlash_Client PRIVATE 
    Shared
    glfw
    Vulkan::Vulkan
    ServerCode
)


set_target_properties(VoxelSlash_Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_SOURCE_DIR}/Compiled>"
	RUNTIME_OUTPUT_NAME "VoxelSlash_Client"
)