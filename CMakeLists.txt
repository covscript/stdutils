cmake_minimum_required(VERSION 3.10)

project(stdutils)

if(DEFINED ENV{CS_DEV_PATH})
    include_directories($ENV{CS_DEV_PATH}/include)
    link_directories($ENV{CS_DEV_PATH}/lib)
endif()

if(DEFINED ENV{CS_DEV_OUTPUT})
    set(LIBRARY_OUTPUT_PATH $ENV{CS_DEV_OUTPUT})
    set(EXECUTABLE_OUTPUT_PATH $ENV{CS_DEV_OUTPUT})
endif()

# Compiler Options
set(CMAKE_CXX_STANDARD 14)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /EHsc /utf-8 /w")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    if (WIN32)
        set(CMAKE_C_FLAGS "--static -fPIC -s -O3")
        set(CMAKE_CXX_FLAGS "--static -fPIC -s -O3")
    else ()
        set(CMAKE_C_FLAGS "-fPIC -s -O3")
        set(CMAKE_CXX_FLAGS "-fPIC -s -O3")
    endif ()
else ()
    set(CMAKE_C_FLAGS "-fPIC -O3")
    set(CMAKE_CXX_FLAGS "-fPIC -O3")
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