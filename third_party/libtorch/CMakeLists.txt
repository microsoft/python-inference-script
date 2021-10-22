# libtorch

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(libtorch_from_src)

set(TORCH
    "1.9.0"
    CACHE STRING "LibTorch version")
set(PYTHON
    "3.7"
    CACHE STRING "Python version")

get_filename_component(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} ABSOLUTE)
message("LIBTORCH_INSTALL_DIR=${CMAKE_INSTALL_PREFIX}")

find_package(Python3 ${PYTHON} EXACT COMPONENTS Interpreter Development)
message(PYTHON_EXECUTABLE=${Python3_EXECUTABLE})

include(ExternalProject)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    ExternalProject_Add(
        libtorch
        GIT_REPOSITORY https://github.com/pytorch/pytorch.git
        GIT_TAG "v${TORCH}"
        CMAKE_ARGS
            "-DBUILD_SHARED_LIBS:BOOL=OFF"
            "-DBUILD_TEST=OFF"
            "-DUSE_DISTRIBUTED=OFF"
            "-DUSE_CUDA=OFF"
            "-DUSE_OPENMP=OFF"
            "-DUSE_MKLDNN=OFF"
            "-DUSE_FBGEMM=OFF"
            "-DUSE_QNNPACK=OFF"
            "-DBUILD_PYTHON=OFF"
            "-DUSE_XNNPACK=OFF"
            "-DUSE_NNPACK=OFF"
            "-DBUILD_CAFFE2=OFF"
            "-DPYTHON_EXECUTABLE:PATH=${Python3_EXECUTABLE}"
            "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
            # "-A x64" is unnecessary, it will be inherited from master cmake command
            # -G "Visual Studio 16 2019" is unnecessary, it will be inherited from master cmake command
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --target install
        INSTALL_COMMAND ""
        TEST_COMMAND "")
endif()
