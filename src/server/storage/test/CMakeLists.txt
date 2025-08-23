# Enable testing
enable_testing()

add_executable(ztofs_storage_test 
    local_storage_test.cpp
)
target_link_libraries(ztofs_storage_test
    meta
    storage
    ${DYNAMIC_LIB}
)
add_dependencies(ztofs_storage_test thirdparty_deps)
# Register test
add_test(NAME ztofs_storage_test COMMAND ztofs_storage_test)