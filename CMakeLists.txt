cmake_minimum_required(VERSION 3.1)
project(ztofs C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS_OUTPUT 
    "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
)

set(CMAKE_CXX_FLAGS "${DEFINE_CLOCK_GETTIME} -DNDEBUG -O2 -D__const__=__unused__ -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")

if(CMAKE_VERSION VERSION_LESS "3.1.3")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(FindThreads)
include(FindProtobuf)

set(PROTO_FILE "${PROJECT_SOURCE_DIR}/src/proto/echo.proto")
set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/proto")

file(MAKE_DIRECTORY ${PROTO_OUT_DIR})
# 这样cmake阶段就可以生成proto
execute_process(
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --proto_path=${PROJECT_SOURCE_DIR}/src/proto
        --cpp_out=${PROTO_OUT_DIR}
        ${PROTO_FILE}
    RESULT_VARIABLE PROTOC_RESULT
)

if(NOT PROTOC_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to generate proto files")
endif()

set(PROTO_SRCS "${PROTO_OUT_DIR}/echo.pb.cc")
set(PROTO_HDRS "${PROTO_OUT_DIR}/echo.pb.h")
include_directories(${PROTO_OUT_DIR})

# brpc include and lib
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/third_party/brpc/build/output")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# option(BUILD_BRPC_TOOLS "Whether to build brpc tools" OFF)
option(LINK_SO "Whether examples are linked dynamically" OFF)
add_subdirectory(third_party/brpc EXCLUDE_FROM_ALL)
include_directories("${CMAKE_PREFIX_PATH}/include")
link_directories(${CMAKE_PREFIX_PATH}/lib)
set(BRPC_LIB brpc)

# gflags include and lib
find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
    message(FATAL_ERROR "Fail to find gflags")
endif()
include_directories(${GFLAGS_INCLUDE_PATH})

# leveldb include and lib
find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif()
include_directories(${LEVELDB_INCLUDE_PATH})


# openssl include and lib
find_package(OpenSSL)
include_directories(${OPENSSL_INCLUDE_DIR})

set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${LEVELDB_LIB}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    dl
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/include)

add_executable(file_server src/server/server.cpp ${PROTO_SRCS})
target_link_libraries(file_server ${BRPC_LIB} ${DYNAMIC_LIB})

add_executable(file_client src/client/client.cpp ${PROTO_SRCS})
target_link_libraries(file_client ${BRPC_LIB} ${DYNAMIC_LIB})