# pybind11 use the variable PYTHON_EXECUTABLE(case sensitive) to detect python
set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})

pybind11_add_module(pyis_python)

target_sources(
    pyis_python
    PRIVATE "python_exports.cpp"
            "word_dict_exports.cpp"
            "model_context_exports.cpp"
            "ort_session_exports.cpp"
            "bert_tokenizer_exports.cpp"
            "cedar_trie_export.cpp"
            "immutable_trie_export.cpp"
            "trie_export.cpp"
            "linear_svm_exports.cpp"
            "linear_chain_crf_exports.cpp"
            "text_featurizers_exports.cpp")
if(ENABLE_OP_FOMA_FST)
    target_sources(pyis_python PRIVATE foma_fst_exports.cpp)
endif(ENABLE_OP_FOMA_FST)
target_link_libraries(pyis_python PUBLIC pyis_operators)
