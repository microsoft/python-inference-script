// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)

namespace pyis {
namespace torchscript {

void init_model_context(::torch::Library& m);
void init_word_dict(::torch::Library& m);

void init_ort_session(::torch::Library& m);
void init_bert_tokenizer(::torch::Library& m);

void init_cedar_trie(::torch::Library& m);
void init_immutable_trie(::torch::Library& m);
void init_trie(::torch::Library& m);

void init_text_feature(::torch::Library& m);
void init_ngram_featurizer(::torch::Library& m);
void init_regex_featurizer(::torch::Library& m);
void init_text_feature_concat(::torch::Library& m);

void init_linear_svm(::torch::Library& m);
void init_linear_chain_crf(::torch::Library& m);

TORCH_LIBRARY(pyis, m) {
    init_model_context(m);
    init_word_dict(m);

#if defined(ENABLE_OP_ORT_SESSION)
    init_ort_session(m);
#endif

#if defined(ENABLE_OP_BERT_TOKENIZER)
    init_bert_tokenizer(m);
#endif

    init_text_feature(m);

#if defined(ENABLE_OP_NGRAM_FEATURIZER)
    init_ngram_featurizer(m);
#endif

#if defined(ENABLE_OP_REGEX_FEATURIZER)
    init_regex_featurizer(m);
#endif

#if defined(ENABLE_OP_TEXT_FEATURE_CONCAT)
    init_text_feature_concat(m);
#endif

#if defined(ENABLE_OP_CEDAR_TRIE)
    init_cedar_trie(m);
#endif

#if defined(ENABLE_OP_IMMUTABLE_TRIE)
    init_immutable_trie(m);
#endif

#if defined(ENABLE_OP_TRIE)
    init_trie(m);
#endif

#if defined(ENABLE_OP_LINEAR_SVM)
    init_linear_svm(m);
#endif

#if defined(ENABLE_OP_LINEAR_CHAIN_CRF)
    init_linear_chain_crf(m);
#endif
}

}  // namespace torchscript
}  // namespace pyis