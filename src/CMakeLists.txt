file(GLOB PROTO_FILES "${PROJECT_SOURCE_DIR}/src/proto/*.proto")

set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/proto")

file(MAKE_DIRECTORY ${PROTO_OUT_DIR})
# 这样cmake阶段就可以生成proto
execute_process(
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --proto_path=${PROJECT_SOURCE_DIR}/src/proto
        --cpp_out=${PROTO_OUT_DIR}
        ${PROTO_FILES}
    RESULT_VARIABLE PROTOC_RESULT
)

if(NOT PROTOC_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to generate proto files")
endif()

file(GLOB PROTO_SRCS "${PROTO_OUT_DIR}/*.pb.cc")

add_executable(file_client client/echo_client.cpp ${PROTO_SRCS})
add_dependencies(file_client thirdparty_deps)
target_link_libraries(file_client ${BRPC_LIB} ${DYNAMIC_LIB})

add_subdirectory(server)
