// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace pyis {
namespace python {

namespace py = pybind11;

void init_model_context(py::module& m);
void init_word_dict(py::module& m);

void init_ort_session(py::module& m);
void init_bert_tokenizer(py::module& m);

void init_cedar_trie(py::module& m);
void init_immutable_trie(py::module& m);
void init_foma_fst(py::module& m);

void init_text_feature(py::module& m);
void init_ngram_featurizer(py::module& m);
void init_regex_featurizer(py::module& m);
void init_text_feature_concat(py::module& m);

void init_linear_svm(py::module& m);
void init_immutable_trie(py::module& m);
void init_trie(py::module& m);
void init_linear_chain_crf(py::module& m);
void init_gpt2_tokenizer(py::module& m);

PYBIND11_MODULE(pyis_python, m) {
    init_model_context(m);

    py::module ops = m.def_submodule("ops");
    init_word_dict(ops);

#if defined(ENABLE_OP_ORT_SESSION)
    init_ort_session(ops);
#endif

#if defined(ENABLE_OP_BERT_TOKENIZER)
    init_bert_tokenizer(ops);
#endif

#if defined(ENABLE_OP_GPT2_TOKENIZER)
    init_gpt2_tokenizer(ops);
#endif

#if defined(ENABLE_OP_CEDAR_TRIE)
    init_cedar_trie(ops);
#endif

#if defined(ENABLE_OP_IMMUTABLE_TRIE)
    init_immutable_trie(ops);
#endif

#if defined(ENABLE_OP_FOMA_FST)
    init_foma_fst(ops);
#endif

    init_text_feature(ops);

#if defined(ENABLE_OP_NGRAM_FEATURIZER)
    init_ngram_featurizer(ops);
#endif

#if defined(ENABLE_OP_REGEX_FEATURIZER)
    init_regex_featurizer(ops);
#endif

#if defined(ENABLE_OP_TEXT_FEATURE_CONCAT)
    init_text_feature_concat(ops);
#endif

#if defined(ENABLE_OP_LINEAR_SVM)
    init_linear_svm(ops);
#endif

#if defined(ENABLE_OP_LINEAR_CHAIN_CRF)
    init_linear_chain_crf(ops);
#endif

#if defined(ENABLE_OP_TRIE)
    init_trie(ops);
#endif
}

}  // namespace python
}  // namespace pyis