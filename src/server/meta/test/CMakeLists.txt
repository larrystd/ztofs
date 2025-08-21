# Enable testing
enable_testing()

add_executable(ztofs_meta_test 
    local_meta_test.cpp
)
target_link_libraries(ztofs_meta_test 
    meta
    storage
    ${BRPC_LIB} ${DYNAMIC_LIB}
)
add_dependencies(ztofs_meta_test thirdparty_deps)
# Register test
add_test(NAME ztofs_meta_test COMMAND ztofs_meta_test)