add_library(pyis_c_api_export STATIC)
target_sources(pyis_c_api_export PRIVATE cc/pyis_c_api.cc include/pyis/pyis_c_api.h)
target_include_directories(pyis_c_api_export PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(pyis_c_api_export PRIVATE PYIS_DLL_EXPORT)
target_link_libraries(pyis_c_api_export PUBLIC pyis_share)

add_library(pyis_c_api INTERFACE)
target_sources(pyis_c_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/pyis/pyis_c_api.h)
target_include_directories(pyis_c_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
