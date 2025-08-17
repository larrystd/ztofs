file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/common
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

# Meta library
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/meta
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

file(GLOB_RECURSE META_SOURCES "meta/*.cpp")
add_library(meta STATIC ${META_SOURCES})
target_include_directories(meta PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/meta
)
target_link_libraries(meta common)

# Service library
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/service
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

file(GLOB_RECURSE META_SOURCES "service/*.cpp")
add_library(service STATIC ${META_SOURCES})
target_include_directories(service PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/service
)
target_link_libraries(service common)

# server.cpp
file(GLOB_RECURSE SERVER_SOURCES "server.cpp")
add_library(server STATIC ${SERVER_SOURCES})
target_include_directories(server PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)
target_link_libraries(server service storage common)
add_dependencies(file_client thirdparty_deps)

# Install targets (optional)
install(TARGETS server DESTINATION bin)