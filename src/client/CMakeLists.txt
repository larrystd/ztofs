add_executable(echo_client echo_client.cpp ${PROTO_SRCS})
add_dependencies(echo_client thirdparty_deps)
target_link_libraries(echo_client ${DYNAMIC_LIB})

add_executable(file_client file_client.cpp ${PROTO_SRCS})
add_dependencies(file_client thirdparty_deps)
target_link_libraries(file_client ${DYNAMIC_LIB})

add_executable(write_benchmark benchmark/write_benchmark.cpp ${PROTO_SRCS})
add_dependencies(write_benchmark thirdparty_deps)
target_link_libraries(write_benchmark ${DYNAMIC_LIB})
