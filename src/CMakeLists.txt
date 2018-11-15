cmake_minimum_required(VERSION 2.8)
project(variadic_table CXX)

set(conan_cmake_file "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
if(EXISTS ${conan_cmake_file})
    include(${conan_cmake_file})
    conan_basic_setup()
endif()

add_library(variadic_table vt_utils.cpp)
