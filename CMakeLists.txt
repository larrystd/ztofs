cmake_minimum_required(VERSION 3.1)
project(ztofs C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")

set(CMAKE_CXX_FLAGS "${DEFINE_CLOCK_GETTIME} -DNDEBUG -O2 -D__const__=__unused__ -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FindThreads)
include(FindProtobuf)

# brpc include and lib
set(BRPC_PATH ${CMAKE_CURRENT_BINARY_DIR}/third_party/brpc/output)
include_directories(${BRPC_PATH}/include)
link_directories(${BRPC_PATH}/lib)
# option(BUILD_BRPC_TOOLS "Whether to build brpc tools" OFF)
option(LINK_SO "Whether examples are linked dynamically" OFF)
option(WITH_DEBUG_SYMBOLS "With debug symbols" OFF)
add_subdirectory(third_party/brpc EXCLUDE_FROM_ALL)
add_custom_target(thirdparty_deps)
add_dependencies(thirdparty_deps brpc-static)
set(BRPC_LIB brpc)

# gflags include and lib
find_package(GTest REQUIRED)
if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
    message(FATAL_ERROR "Fail to find gflags")
endif()
include_directories(${GFLAGS_INCLUDE_PATH})

# glog include and lib
find_path(GLOG_INCLUDE_PATH glog/logging.h)
find_library(GLOG_LIBRARY NAMES glog libglog)
if((NOT GLOG_INCLUDE_PATH) OR (NOT GLOG_LIBRARY))
    message(FATAL_ERROR "Fail to find glog")
endif()
include_directories(${GLOG_INCLUDE_PATH})

# Google Test
find_package(GTest REQUIRED)
if(NOT GTEST_FOUND)
    message(FATAL_ERROR "Fail to find GTest")
endif()
# Include Google Test headers
include_directories(${GTEST_INCLUDE_DIRS})

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
    ${GLOG_LIBRARY}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    ${LEVELDB_LIB}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    dl
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/include)

# include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src)