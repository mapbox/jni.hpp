cmake_minimum_required(VERSION 3.10)

project(jni.hpp VERSION 4.0.1)

option(JNI_HPP_BUILD_TESTS "Build and run tests when BUILD_TESTING is enabled." ON)
option(JNI_HPP_BUILD_EXAMPLES "Build and run examples" ON)

add_library(jni_hpp INTERFACE)
add_library(Mapbox::JNI ALIAS jni_hpp)

target_include_directories(jni_hpp INTERFACE include)
target_compile_features(jni_hpp INTERFACE cxx_std_14)

if(JNI_HPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(JNI_HPP_BUILD_EXAMPLES)
    enable_testing()
    add_subdirectory(examples)
endif()
