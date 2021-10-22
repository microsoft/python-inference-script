# spdlog

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.9.1)

# use external fmt lib, see: https://github.com/gabime/spdlog/issues/1634
if(USE_FMTLIB)
    set(SPDLOG_FMT_EXTERNAL
        ON
        CACHE BOOL "" FORCE)
endif(USE_FMTLIB)

if(PYIS_NO_EXCEPTIONS)
    set(SPDLOG_NO_EXCEPTIONS
        ON
        CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(spdlog)
