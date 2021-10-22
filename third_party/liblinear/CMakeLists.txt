# liblinear

FetchContent_Declare(
    liblinear_src
    GIT_REPOSITORY https://github.com/cjlin1/liblinear.git
    GIT_TAG v243)
FetchContent_MakeAvailable(liblinear_src)

add_library(liblinear STATIC)
file(
    GLOB
    LIBLINEAR_SOURCE_FILES
    ${liblinear_src_SOURCE_DIR}/*.h
    ${liblinear_src_SOURCE_DIR}/*.c
    ${liblinear_src_SOURCE_DIR}/*.cpp
    ${liblinear_src_SOURCE_DIR}/blas/*.h
    ${liblinear_src_SOURCE_DIR}/blas/*.c
    liblinear_api.h
    liblinear_api.cpp)
target_sources(liblinear PRIVATE ${LIBLINEAR_SOURCE_FILES})
target_include_directories(liblinear PUBLIC ${liblinear_src_SOURCE_DIR}
                                            ${CMAKE_CURRENT_SOURCE_DIR})
