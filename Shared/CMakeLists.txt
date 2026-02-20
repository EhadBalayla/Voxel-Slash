find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)



file(GLOB_RECURSE SRC_FILES 
    src/*.cpp 
    src/*.h
    src/*.c
)

add_library(Shared STATIC ${SRC_FILES})

target_link_libraries(Shared PRIVATE 
    glfw
    Vulkan::Vulkan
)


set_target_properties(Shared PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_SOURCE_DIR}/Compiled>"
)