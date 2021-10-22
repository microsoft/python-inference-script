# ziplib

FetchContent_Declare(
    ziplib
    GIT_REPOSITORY https://bitbucket.org/wbenny/ziplib.git
    GIT_TAG v0.01)
FetchContent_MakeAvailable(ziplib)

file(COPY ziplib.cmake DESTINATION ${ziplib_SOURCE_DIR})
file(RENAME ${ziplib_SOURCE_DIR}/ziplib.cmake ${ziplib_SOURCE_DIR}/CMakeLists.txt)
