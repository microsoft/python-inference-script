cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(pyis VERSION 0.1)

include(CMakeDependentOption)

option(PYTHON_BACKEND "bind operators to onnx or not" ON)
option(TORCH_BACKEND "bind operators to libtorch or not" ON)
option(ONNX_BACKEND "bind operators to onnx or not" ON)
option(USE_SPDLOG "use spdlog for logging or not" OFF)
option(USE_FMTLIB "use fmtlib for string formating or not" OFF)
option(PYIS_NO_EXCEPTIONS "turn c++ exceptions on or off(call abort() on any throw statement)" OFF)
option(PYIS_NO_RTTI "turn c++ RTTI on or off" OFF)
set(PYTHON
    ""
    CACHE STRING "set which version of python to use, default to auto detect")
set(TORCH
    "1.9.0"
    CACHE STRING "set which version of torch to use, deafult to 1.9.0 LTS")
set(TORCH_INSTALL_DIR
    ""
    CACHE PATH "set a prebuilt libtorch to link to, default to auto downloading")
set(ORT
    "1.8.0"
    CACHE STRING "set which version of onnxruntime to use, default to 1.8.0")
option(ENABLE_OP_ORT_SESSION "onnxruntime extension" ON)
option(ENABLE_OP_BERT_TOKENIZER "bert tokenizer" ON)
option(ENABLE_OP_GPT2_TOKENIZER "GPT2Tokenizer" ON)
option(ENABLE_OP_NGRAM_FEATURIZER "ngram featurizer extension" ON)
option(ENABLE_OP_REGEX_FEATURIZER "regex featurizer extension" ON)
option(ENABLE_OP_TEXT_FEATURE_CONCAT "text feature concat extension" ON)
option(ENABLE_OP_CEDAR_TRIE "cedar trie extension" ON)
option(ENABLE_OP_IMMUTABLE_TRIE "immutable trie extension" ON)
option(ENABLE_OP_TRIE "trie extension" ON)
option(ENABLE_OP_LINEAR_SVM "liblinear extension" ON)
option(ENABLE_OP_LINEAR_CHAIN_CRF "linear chain crf extension" ON)
option(ENABLE_OP_FOMA_FST "foma fst extension" ON)

# c/c++ common compliation options
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# torch in PyPI is not built with cxx11 abi. As a result,
# 1. For users who install pytorch from PyPI, to ensure pyis_torch could be
#    consumed by  We have to build everything for pyis_torch without cxx11 abi.
# 2. If you build your own libtorch or pytorch, you could set them to use cxx11 abi.
#    Then we don't need to set the following.
if(TORCH_BACKEND AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

# force msvc to use utf-8 encoding
if(MSVC)
    add_compile_options("/utf-8")
endif()

if(USE_SPDLOG)
    add_definitions(-DUSE_SPDLOG)
endif()

if(USE_FMTLIB)
    add_definitions(-DUSE_FMTLIB)
endif()

if(PYIS_NO_EXCEPTIONS)
    add_definitions(-DPYIS_NO_EXCEPTIONS -DORT_NO_EXCEPTIONS)
endif()

# define a conditional compliation macro for each operator.
if(ENABLE_OP_ORT_SESSION)
    add_definitions(-DENABLE_OP_ORT_SESSION)
    add_definitions(-DORT_API_MANUAL_INIT)
endif()

if(ENABLE_OP_BERT_TOKENIZER)
    add_definitions(-DENABLE_OP_BERT_TOKENIZER)
endif()

if(ENABLE_OP_GPT2_TOKENIZER)
    add_definitions(-DENABLE_OP_GPT2_TOKENIZER)
endif()

if(ENABLE_OP_NGRAM_FEATURIZER)
    add_definitions(-DENABLE_OP_NGRAM_FEATURIZER)
endif()

if(ENABLE_OP_REGEX_FEATURIZER)
    add_definitions(-DENABLE_OP_REGEX_FEATURIZER)
endif()

if(ENABLE_OP_TEXT_FEATURE_CONCAT)
    add_definitions(-DENABLE_OP_TEXT_FEATURE_CONCAT)
endif()

if(ENABLE_OP_FOMA_FST)
    add_definitions(-DENABLE_OP_FOMA_FST)
endif()

if(ENABLE_OP_CEDAR_TRIE)
    add_definitions(-DENABLE_OP_CEDAR_TRIE)
endif()

if(ENABLE_OP_IMMUTABLE_TRIE)
    add_definitions(-DENABLE_OP_IMMUTABLE_TRIE -DVALIDATE_TAGS)
endif()

if(ENABLE_OP_TRIE)
    add_definitions(-DENABLE_OP_TRIE)
endif()

if(ENABLE_OP_LINEAR_SVM)
    add_definitions(-DENABLE_OP_LINEAR_SVM)
endif()

if(ENABLE_OP_LINEAR_CHAIN_CRF)
    add_definitions(-DENABLE_OP_LINEAR_CHAIN_CRF)
endif()

include(cmake/handle_exception.cmake)
include(cmake/handle_rtti.cmake)
include(cmake/detect_python.cmake)

add_subdirectory(third_party)
#set(ENV{CTEST_OUTPUT_ON_FAILURE} "TRUE")
enable_testing()
add_subdirectory(pyis)
add_subdirectory(tests)
add_subdirectory(py)
