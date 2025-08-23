file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/common
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

# Meta
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/meta
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )
add_subdirectory(meta)

# Service
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/service
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )
add_subdirectory(service)

# Storage
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/storage
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )
add_subdirectory(storage)

# server binary
add_executable(file_server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp ${PROTO_SRCS})
target_include_directories(file_server PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)
target_link_libraries(file_server meta storage service ${DYNAMIC_LIB})
add_dependencies(file_server thirdparty_deps)
install(TARGETS file_server DESTINATION bin)

