cmake_minimum_required(VERSION 3.16)

project(stdutils)

if (DEFINED ENV{CS_DEV_PATH})
    message("-- CovScript SDK detected at $ENV{CS_DEV_PATH}")
    include("$ENV{CS_DEV_PATH}/csbuild.cmake")
    include_directories($ENV{CS_DEV_PATH}/include)
    link_directories($ENV{CS_DEV_PATH}/lib)
else ()
    message(FATAL_ERROR "-- CovScript SDK not detected. Please set environment variable CS_DEV_PATH")
endif ()

if (APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(FFI REQUIRED libffi)
    include_directories(${FFI_INCLUDE_DIRS})
    link_directories(${FFI_LIBRARY_DIRS})
endif ()

add_library(cffi SHARED cffi.cpp)
add_library(bitwise SHARED bitwise.cpp)
add_library(sdk_extension SHARED sdk_extension.cpp)
add_library(test_cffi SHARED test_cffi.c)

target_link_libraries(cffi ffi covscript)
target_link_libraries(bitwise covscript)
target_link_libraries(sdk_extension covscript)

set_target_properties(cffi PROPERTIES OUTPUT_NAME cffi)
set_target_properties(cffi PROPERTIES PREFIX "")
set_target_properties(cffi PROPERTIES SUFFIX ".cse")

set_target_properties(bitwise PROPERTIES OUTPUT_NAME bitwise)
set_target_properties(bitwise PROPERTIES PREFIX "")
set_target_properties(bitwise PROPERTIES SUFFIX ".cse")

set_target_properties(sdk_extension PROPERTIES OUTPUT_NAME sdk_extension)
set_target_properties(sdk_extension PROPERTIES PREFIX "")
set_target_properties(sdk_extension PROPERTIES SUFFIX ".cse")

set_target_properties(test_cffi PROPERTIES OUTPUT_NAME test_cffi)
set_target_properties(test_cffi PROPERTIES PREFIX "")
set_target_properties(test_cffi PROPERTIES SUFFIX ".csx")