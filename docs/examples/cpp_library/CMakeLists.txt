cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(cpp_library_test VERSION 0.1)

include(FetchContent)
FetchContent_Declare(
    pyis
    GIT_REPOSITORY https://github.com/microsoft/python-inference-script
    GIT_TAG main)
set(PYTHON_BACKEND
    OFF
    CACHE BOOL "" FORCE)
set(TORCH_BACKEND
    OFF
    CACHE BOOL "" FORCE)
set(ONNX_BACKEND
    OFF
    CACHE BOOL "" FORCE)

add_executable(cpp_library_test cpp_library_test.cpp)
target_link_libraries(cpp_library_test PRIVATE pyis_operators)
