cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(pyis_torch_test VERSION 0.1)

include(FetchContent)
FetchContent_Declare(
    pyis
    GIT_REPOSITORY https://github.com/microsoft/python-inference-script
    GIT_TAG main)
set(PYTHON_BACKEND
    OFF
    CACHE BOOL "" FORCE)
set(TORCH_BACKEND
    ON
    CACHE BOOL "" FORCE)
set(ONNX_BACKEND
    OFF
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(pyis)

add_executable(pyis_torch_test)
target_sources(pyis_torch_test PRIVATE test.cpp)

# link libtorch library
find_package(Torch REQUIRED PATHS ${TORCH_INSTALL_DIR})
target_link_libraries(pyis_torch_test "${TORCH_LIBRARIES}")
target_link_libraries(pyis_torch_test -Wl,--no-as-needed pyis_torch)

if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(TARGET pyis_torch_test
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TORCH_DLLS} $<TARGET_FILE_DIR:pyis_torch_test>)
endif (MSVC)

add_custom_command(TARGET pyis_torch_test
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:pyis_torch> $<TARGET_FILE_DIR:pyis_torch_test>)

# include pyis c api header
target_include_directories(pyis_torch_test PRIVATE ${pyis_SOURCE_DIR}/pyis/c_api/include)
