file(
    GLOB
    LINEAR_CHAIN_CRF_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(linear_chain_crf STATIC ${LINEAR_CHAIN_CRF_SOURCE_FILES})
target_link_libraries(linear_chain_crf PRIVATE pyis_share)
target_include_directories(linear_chain_crf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
