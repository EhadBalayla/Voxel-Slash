file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*.h
    src/*.c
)

add_executable(VoxelSlash_Server ${SRC_FILES})

target_link_libraries(VoxelSlash_Server PRIVATE 
    ServerCode
)

target_include_directories(VoxelSlash_Server PRIVATE
    ${CMAKE_SOURCE_DIR}/ServerCode/src
    ${CMAKE_SOURCE_DIR}/ServerCode/ThirdParty
    ${CMAKE_SOURCE_DIR}/Shared/ThirdParty
)

set_target_properties(VoxelSlash_Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_SOURCE_DIR}/Compiled>"
	RUNTIME_OUTPUT_NAME "VoxelSlash_Server"
)