find_package(Torch REQUIRED PATHS ${TORCH_INSTALL_DIR})

add_library(pyis_torch_static STATIC)

target_sources(
    pyis_torch_static
    PRIVATE "word_dict_exports.cpp"
            "model_context_exports.cpp"
            "ort_session_exports.cpp"
            "bert_tokenizer_exports.cpp"
            "gpt2_tokenizer_exports.cpp"
            "text_featurizers_exports.cpp"
            "cedar_trie_exports.cpp"
            "immutable_trie_exports.cpp"
            "trie_exports.cpp"
            "linear_svm_exports.cpp"
            "linear_chain_crf_exports.cpp")
target_link_libraries(pyis_torch_static PUBLIC "${TORCH_LIBRARIES}" pyis_operators)

add_library(pyis_torch SHARED $<TARGET_OBJECTS:pyis_c_api_export>)
string(REPLACE "." "" TORCH_VERSION_STD ${TORCH})
set_target_properties(pyis_torch PROPERTIES OUTPUT_NAME "pyis_torch${TORCH_VERSION_STD}")
target_sources(pyis_torch PRIVATE "torch_exports.cpp")
target_link_libraries(pyis_torch PUBLIC pyis_torch_static)
