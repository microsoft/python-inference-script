# fmtlib

FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG 2021-09-01)

FetchContent_MakeAvailable(re2)

set_target_properties(re2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
