find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)


file(GLOB_RECURSE SRC_FILES 
    src/*.cpp 
    src/*.h
    src/*.c

    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_vulkan.cpp
)

add_executable(VoxelSlash_Editor ${SRC_FILES})

target_include_directories(VoxelSlash_Editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
)

target_link_libraries(VoxelSlash_Editor PRIVATE 
    glfw 
    Vulkan::Vulkan
)


set_target_properties(VoxelSlash_Editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_SOURCE_DIR}/Compiled>"
	RUNTIME_OUTPUT_NAME "VoxelSlash_Editor"
)