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

if (APPLE OR UNIX)
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

foreach (target cffi bitwise sdk_extension)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME ${target}
        PREFIX ""
        SUFFIX ".cse"
    )
endforeach ()

set_target_properties(test_cffi PROPERTIES
    OUTPUT_NAME test_cffi
    PREFIX ""
    SUFFIX ".csx"
)

install(TARGETS cffi bitwise sdk_extension
    LIBRARY DESTINATION imports
    RUNTIME DESTINATION imports
)

install(TARGETS test_cffi
    LIBRARY DESTINATION tests
    RUNTIME DESTINATION tests
)

install(FILES stdutils.csp
    DESTINATION imports
)
