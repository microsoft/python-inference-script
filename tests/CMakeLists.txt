cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)

add_executable(test_pyis_cpp
    test_share/test_binary_serialize.cpp
    test_share/test_binary_deserialize.cpp
    test_share/test_json_persisit_helper.cpp
    test_share/test_ops_cache.cpp
    test_ngram_featurizer/test_ngram_featurizer.cpp
    test_cedar_trie/test_cedar_trie.cpp
    test_immutable_trie/test_immutable_trie.cpp
)
target_link_libraries(test_pyis_cpp
    PRIVATE
    gtest_main
    pyis_share
    pyis_operators)

if (ENABLE_OP_ORT_SESSION)
    target_sources(test_pyis_cpp PRIVATE 
    test_ort_session/test_ort_session.cpp)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        target_link_libraries(test_pyis_cpp PRIVATE ${CMAKE_DL_LIBS})
    endif()
    ADD_CUSTOM_COMMAND(TARGET test_pyis_cpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${ONNXRUNTIME_DLLS} $<TARGET_FILE_DIR:test_pyis_cpp>
    )
endif ()

if (ENABLE_OP_FOMA_FST)
    target_sources(test_pyis_cpp PRIVATE
    test_foma_fst/test_foma_fst.cpp)
endif ()

add_test(
    NAME test_pyis_cpp
    COMMAND $<TARGET_FILE:test_pyis_cpp>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
)