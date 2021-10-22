# rapidjson

FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG v1.1.0)
set(RAPIDJSON_BUILD_DOC
    OFF
    CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(RAPIDJSON_HAS_STDSTRING
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rapidjson)

set(RAPIDJSON_INCLUDE_DIR
    ${rapidjson_SOURCE_DIR}/include
    CACHE PATH "" FORCE)
