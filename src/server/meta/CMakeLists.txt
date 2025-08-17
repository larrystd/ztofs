file(GLOB_RECURSE META_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
message("META_SOURCES: ${CMAKE_CURRENT_SOURCE_DIR}")

add_library(meta STATIC ${META_SOURCES})
target_include_directories(meta PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# unit tests
add_subdirectory(test)