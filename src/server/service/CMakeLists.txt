file(GLOB_RECURSE SERVICE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(service STATIC ${SERVICE_SOURCES})
target_include_directories(service PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)