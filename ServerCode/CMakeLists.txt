find_package(ZLIB REQUIRED)


file(GLOB_RECURSE SRC_FILES 
    src/*.cpp 
    src/*.h
    src/*.c
)

add_library(ServerCode STATIC ${SRC_FILES})

target_include_directories(ServerCode PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
    ${CMAKE_SOURCE_DIR}/Shared/ThirdParty
)

target_link_libraries(ServerCode PRIVATE
    ZLIB::ZLIB
)

set_target_properties(ServerCode PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_SOURCE_DIR}/Compiled>"
)