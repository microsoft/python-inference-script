cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(FetchContent)

include(fmtlib.cmake)
include(spdlog.cmake)
include(gtest.cmake)
include(pybind11.cmake)
include(rapidjson.cmake)
#include(libzip.cmake)
#include(re2.cmake)
add_subdirectory(liblinear)

if(TORCH_BACKEND)
    include(libtorch/libtorch_prebuilt.cmake)
endif()

if(ONNX_BACKEND OR ENABLE_OP_ORT_SESSION)
    include(onnxruntime.cmake)
endif()

if(ENABLE_OP_FOMA_FST)
    include(foma.cmake)
endif()
