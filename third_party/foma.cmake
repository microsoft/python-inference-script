# foma

FetchContent_Declare(
    foma_lib
    GIT_REPOSITORY https://github.com/i-lovelife/foma.git
    GIT_TAG 79d5731239a60d33bed17984cee748ea92ed09b1)
if(NOT foma_lib_POPULATED)
    FetchContent_Populate(foma_lib)
    file(COPY ${foma_lib_SOURCE_DIR}/foma/fomalibconf.h ${foma_lib_SOURCE_DIR}/foma/fomalib.h
              ${foma_lib_SOURCE_DIR}/foma/foma.h DESTINATION ${foma_lib_BINARY_DIR})
    add_subdirectory(${foma_lib_SOURCE_DIR}/foma ${foma_lib_BINARY_DIR})
endif()
set(FOMA_LIB_INCLUDE_DIR
    ${foma_lib_BINARY_DIR}
    CACHE PATH "" FORCE)
