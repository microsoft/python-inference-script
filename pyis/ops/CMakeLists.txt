add_library(pyis_operators STATIC)

target_sources(
    pyis_operators
    PRIVATE example_op/word_dict.h
            example_op/word_dict.cpp
            example_ort/language_detector.h
            example_ort/language_detector.cpp
            example_ort/naive_tokenizer.h
            example_ort/naive_tokenizer.cpp
            text/text_feature.h
            text/text_feature_concat.h
            text/text_feature_concat.cpp
            text/ngram_featurizer.h
            text/ngram_featurizer.cpp
            text/regex_featurizer.h
            text/regex_featurizer.cpp
            text/immutable_trie.h
            text/immutable_trie.cpp
            text/cedar_trie.h
            text/cedar_trie.cpp
            text/trie.h
            text/trie.cpp
            tokenizer/bert_tokenizer.h
            tokenizer/bert_tokenizer.cpp
            tokenizer/tokenizer_base.h
            tokenizer/tokenizer_base.cpp
            tokenizer/basic_tokenizer.h
            tokenizer/basic_tokenizer.cpp
            tokenizer/wordpiece_tokenizer.h
            tokenizer/wordpiece_tokenizer.cpp
            tokenizer/gpt2_tokenizer.h
            tokenizer/gpt2_tokenizer.cpp)
target_link_libraries(pyis_operators PUBLIC pyis_share)

if(ENABLE_OP_ORT_SESSION)
    target_sources(
        pyis_operators
        PRIVATE ort_session/ort_session.h
                ort_session/ort_session.cpp
                ort_session/ort_tensor_utils.h
                ort_session/ort_tensor_utils.cpp
                ort_session/ort_globals.h
                ort_session/ort_globals.cpp
                ort_session/ort_batch_context.h
                ort_session/ort_dym_batch_mgr.h
                ort_session/ort_dym_batch_mgr.cpp)
    # do not set ORT_API_MANUAL_INIT. The OrtSession class require the init
    # to be done before any object instantiation
    target_include_directories(pyis_operators PUBLIC ${ONNXRUNTIME_INCLUDE_DIR})
endif(ENABLE_OP_ORT_SESSION)

if(ENABLE_OP_FOMA_FST)
    target_sources(pyis_operators PRIVATE text/foma_fst.h text/foma_fst.cpp)
    target_include_directories(pyis_operators PUBLIC ${FOMA_LIB_INCLUDE_DIR})
    target_link_libraries(pyis_operators PUBLIC foma)
endif(ENABLE_OP_FOMA_FST)

if(ENABLE_OP_LINEAR_SVM)
    target_sources(pyis_operators PRIVATE linear_svm/linear_svm.h linear_svm/linear_svm.cpp)
    target_link_libraries(pyis_operators PRIVATE liblinear)
endif(ENABLE_OP_LINEAR_SVM)

if(ENABLE_OP_LINEAR_CHAIN_CRF)
    add_subdirectory(linear_chain_crf)
    target_link_libraries(pyis_operators PRIVATE linear_chain_crf)
endif(ENABLE_OP_LINEAR_CHAIN_CRF)
