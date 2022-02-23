add_library(pyis_share STATIC)

target_sources(
    pyis_share
    PRIVATE hardware_utils.cpp
            hardware_utils.h
            binary_serialize_type.h
            binary_serialize_type.cpp
            binary_serialize_helper.h
            binary_serialize_helper.cpp
            binary_deserialize_helper.h
            binary_deserialize_helper.cpp
            cached_object.h
            exception.h
            expected.hpp
            json_persist_helper.h
            json_persist_helper.cpp
            scope_guard.h
            str_utils.h
            str_utils.cpp
            logging.h
            logging.cpp
            model_context.h
            model_context.cpp
            model_storage.h
            model_storage.cpp
            model_storage_local.h
            model_storage_local.cpp
            file_system.h
            file_system.cpp
            ustring.h
            ustring.cpp)

get_filename_component(PYIS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE)
target_include_directories(pyis_share PUBLIC ${PYIS_ROOT_DIR} ${RAPIDJSON_INCLUDE_DIR})

if(USE_FMTLIB)
    target_link_libraries(pyis_share PUBLIC fmt::fmt)
endif()

if(USE_SPDLOG)
    target_link_libraries(pyis_share PUBLIC spdlog::spdlog)
endif()

add_compile_definitions(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(pyis_share stdc++fs)
endif()
