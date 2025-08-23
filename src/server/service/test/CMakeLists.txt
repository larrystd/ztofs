# Enable testing
enable_testing()

add_executable(file_service_test
    file_service_test.cpp
)
target_link_libraries(file_service_test
    service
    meta
    storage
    ${PROTO_SRCS}
    ${DYNAMIC_LIB}
)
add_dependencies(file_service_test thirdparty_deps)
# Register test
add_test(NAME file_service_test COMMAND file_service_test)