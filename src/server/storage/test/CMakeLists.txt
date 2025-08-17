# Enable testing
enable_testing()

add_executable(storage_test 
    local_storage_test.cpp
)
target_link_libraries(storage_test
    meta
    storage
    ${BRPC_LIB} ${DYNAMIC_LIB}
)
add_dependencies(storage_test thirdparty_deps)
# Register test
add_test(NAME storage_test COMMAND storage_test)