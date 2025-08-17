# Enable testing
enable_testing()

add_executable(meta_test 
    local_meta_test.cpp
)
target_link_libraries(meta_test 
    meta
    ${BRPC_LIB} ${DYNAMIC_LIB}
)
add_dependencies(meta_test thirdparty_deps)
# Register test
add_test(NAME meta_test COMMAND meta_test)