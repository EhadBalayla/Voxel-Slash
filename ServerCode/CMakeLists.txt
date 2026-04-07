file(GLOB_RECURSE SRC_FILES 
    src/*.cpp 
    src/*.h
    src/*.c
)

add_library(ServerCode STATIC ${SRC_FILES})

set_target_properties(ServerCode PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_SOURCE_DIR}/Compiled>"
)