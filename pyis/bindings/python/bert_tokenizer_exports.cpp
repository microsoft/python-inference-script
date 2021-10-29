// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <algorithm>
#include <cstdlib>

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pyis/ops/tokenizer/bert_tokenizer.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace python {

namespace py = pybind11;
using pyis::ops::BertTokenizer;

void init_bert_tokenizer(py::module& m) {
    py::class_<BertTokenizer, std::shared_ptr<BertTokenizer>>(m, "BertTokenizer",
                                                              R"pbdoc(
            For tokenizing text into id sequences (same as Bert Tokenizer Fast)

        )pbdoc")
        .def(py::init<std::string, bool, bool, std::string, std::string, std::string, std::string, std::string, bool,
                      bool, std::string>(),
             py::arg("vocab_file"), py::arg("do_lower_case") = true, py::arg("do_basic_tokenize") = true,
             py::arg("cls_token") = "[CLS]", py::arg("sep_token") = "[SEP]", py::arg("unk_token") = "[UNK]",
             py::arg("pad_token") = "[PAD]", py::arg("mask_token") = "[MASK]", py::arg("tokenize_chinese_chars") = true,
             py::arg("strip_accents") = false, py::arg("suffix_indicator") = "##",
             R"pbdoc(
                Create a BertTokenizer instance

                Args:
                    vocab_file_path (str): path to the vocabulary file
                    do_lower_case (bool): should the tokenizer turn string into lowercase, default to true
                    do_basic_tokenize (bool): should the tokenizer do basic tokenize first, default to true
                    cls_token (str): start token symbol, default to '[CLS]'
                    sep_token (str): end token symbol, default to '[SEP]'
                    unknown_token (str): unknown token symbol, default to '[UNK]'
                    pad_token (str): padding token symbol, default to '[PAD]'
                    mask_token (str): mask token symbol, default to '[MASK]'
                    tokenize_chinese_chars (bool): should the tokenizer tokenize chinese chars, default to true
                    strip_accents (bool): shoule the tokenizer strip accents. default to false
                    suffix_indicator (str): string prefix indicates the token is a suffix of previous token. default to '##'

            )pbdoc")
        .def("tokenize", &BertTokenizer::Tokenize, py::arg("query"), py::return_value_policy::move,
             R"pbdoc(
            Tokenize the input string and return list of tokens.

            Args:
                    query (str): input query to be tokenized
            
            Returns:
                    output list of tokens


        )pbdoc")
        .def("encode",
             static_cast<std::vector<int64_t> (BertTokenizer::*)(const std::string&, int64_t)>(&BertTokenizer::Encode),
             py::arg("query"), py::arg("max_length") = static_cast<int64_t>(1e15),
             R"pbdoc(
            Tokenize the input string and return list of token indices (ids).

            Args:
                    query (str): input query to be tokenized
                    max_length (int): max acceptable length for truncation. default to 1e15
                    
            Returns:
                    output list of token indices (ids)
            )pbdoc")
        .def("encode2",
             static_cast<std::vector<int64_t> (BertTokenizer::*)(const std::string&, const std::string&, int64_t,
                                                                 const std::string&)>(&BertTokenizer::Encode),
             py::arg("str1"), py::arg("str2"), py::arg("max_length") = static_cast<int64_t>(1e15),
             py::arg("truncation_strategy") = "longest_first",

             R"pbdoc(
            Tokenize two input strings and return list of token indices (ids).

            Args:
                    str1, str2 (str): input query to be tokenized
                    max_length (int): max acceptable length for truncation. default to 1e15
                    truncation_strategy (str): truncation strategy, coule be 'longest_first' (default), 'longest_from_back', 'only_first' and 'only_second'
            
            Returns:
                    output list of token indices (ids)
            )pbdoc")
        .def("encode_plus",
             static_cast<std::vector<std::tuple<int64_t, int64_t, int64_t>> (BertTokenizer::*)(
                 const std::string&, int64_t)>(&BertTokenizer::EncodePlus),
             py::arg("str"), py::arg("max_length") = static_cast<int64_t>(1e15),
             R"pbdoc(
            Tokenize the input string and return list of token indices (ids), type ids and attention mask.

            Args:
                    query (str): input query to be tokenized
                    max_length (int): max acceptable length for truncation. default to 1e15
                    
            Returns:
                    output list of tuple, each tuple contains (ids, type ids, attention mask)
            )pbdoc")
        .def("encode_plus2",
             static_cast<std::vector<std::tuple<int64_t, int64_t, int64_t>> (BertTokenizer::*)(
                 const std::string&, const std::string&, int64_t, const std::string&)>(&BertTokenizer::EncodePlus),
             py::arg("str1"), py::arg("str2"), py::arg("max_length") = static_cast<int64_t>(1e15),
             py::arg("truncation_strategy") = "longest_first",
             R"pbdoc(
            Tokenize two input strings and return list of token indices (ids), type ids and attention mask.

            Args:
                    str1, str2 (str): input query to be tokenized
                    max_length (int): max acceptable length for truncation. default to 1e15
                    truncation_strategy (str): truncation strategy, coule be 'longest_first' (default), 'longest_from_back', 'only_first' and 'only_second'
            
            Returns:
                    output list of tuple, each tuple contains (ids, type ids, attention mask)
            )pbdoc")
        .def("decode", &BertTokenizer::Decode, py::arg("ids"), py::arg("skip_special_tokens") = false,
             py::arg("clean_up_tokenization_spaces") = true, R"pbdoc(
            Decode a list of token indices, turn them into a string.

            Args:
                    ids (List[int]): list of token indices
                    skip_special_tokens (bool): should the decoder ignore special tokens. default to false.
                    clean_up_tokenization_spaces (bool): should the output string be clean up to fit for English.

            Returns:
                    output the corresponding string to the list.
        )pbdoc")
        .def("convert_id_to_token", &BertTokenizer::ConvertIdToToken, py::arg("id"), py::return_value_policy::automatic,
             R"pbdoc(
            Convert token id to token text

            Args:
                    id (int): token id
            
            Returns:
                    output token text to the given id


        )pbdoc")
        .def("convert_token_to_id", &BertTokenizer::ConvertTokenToId, py::arg("token"),
             py::return_value_policy::automatic,
             R"pbdoc(
            Convert token text to token id

            Args:
                    token (str): token text
            
            Returns:
                    output token id to the given token text


        )pbdoc")
        .def(py::pickle(
            [](BertTokenizer& self) {
                // __getstate__
                // return a bytes array that fully encodes the state of the object
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                // __setstate__
                // create a new C++ instance from the state and files(optional) saved above
                std::shared_ptr<BertTokenizer> obj = ModelContext::GetActive()->GetOrCreateObject<BertTokenizer>(state);
                return obj;
            }));
    ;
}

}  // namespace python
}  // namespace pyis
