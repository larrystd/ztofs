file(GLOB_RECURSE STORAGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(storage STATIC ${STORAGE_SOURCES})
target_include_directories(storage PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(test)