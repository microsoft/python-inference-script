// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <algorithm>
#include <cstdlib>

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pyis/ops/tokenizer/gpt2_tokenizer.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace python {

namespace py = pybind11;
using pyis::ops::GPT2Tokenizer;

void init_gpt2_tokenizer(py::module& m) {
    py::class_<GPT2Tokenizer, std::shared_ptr<GPT2Tokenizer>>(m, "GPT2Tokenizer",
                                                              R"pbdoc(
            GPT2Tokenizer

        )pbdoc")
        .def(py::init<std::string, std::string, std::string, std::string, std::string, bool>(), py::arg("vocab_file"),
             py::arg("merges_file"), py::arg("unk_token") = "<|endoftext|>", py::arg("bos_token") = "<|endoftext|>",
             py::arg("eos_token") = "<|endoftext|>", py::arg("add_prefix_space") = false,
             R"pbdoc(
                Create a GPT2Tokenizer instance

                Args:
                    vocab_file_path (str): path to the vocabulary file.
                    merges_file_path (str): path to the merges file.
                    unk_token (str): unknown token, default to '<|endoftext|>'
                    bos_token (str): begin of sentence token, default to '<|endoftext|>'
                    eos_token (str): end of sentence token, default to '<|endoftext|>'
                    add_prefix_space: Whether or not to add an initial space to the input. This allows to treat the leading word just as any other word. default to false.
            )pbdoc")
        .def("tokenize", &GPT2Tokenizer::Tokenize, py::arg("query"), py::return_value_policy::move,
             R"pbdoc(
            Tokenize the input string and return list of tokens.

            Args:
                    query (str): input query to be tokenized
            
            Returns:
                    output list of tokens
        )pbdoc")
        .def("encode",
             static_cast<std::vector<int64_t> (GPT2Tokenizer::*)(const std::string&, int64_t)>(&GPT2Tokenizer::Encode),
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
             static_cast<std::vector<int64_t> (GPT2Tokenizer::*)(const std::string&, const std::string&, int64_t,
                                                                 const std::string&)>(&GPT2Tokenizer::Encode),
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
             static_cast<std::vector<std::tuple<int64_t, int64_t, int64_t>> (GPT2Tokenizer::*)(
                 const std::string&, int64_t)>(&GPT2Tokenizer::EncodePlus),
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
             static_cast<std::vector<std::tuple<int64_t, int64_t, int64_t>> (GPT2Tokenizer::*)(
                 const std::string&, const std::string&, int64_t, const std::string&)>(&GPT2Tokenizer::EncodePlus),
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
        .def("decode", &GPT2Tokenizer::Decode, py::arg("ids"), py::arg("skip_special_tokens") = false,
             py::arg("clean_up_tokenization_spaces") = true, R"pbdoc(
            Decode a list of token indices, turn them into a string.

            Args:
                    ids (List[int]): list of token indices
                    skip_special_tokens (bool): should the decoder ignore special tokens. default to false.
                    clean_up_tokenization_spaces (bool): should the output string be clean up to fit for English.

            Returns:
                    output the corresponding string to the list.
        )pbdoc")
        .def("convert_id_to_token", &GPT2Tokenizer::ConvertIdToToken, py::arg("id"), py::return_value_policy::automatic,
             R"pbdoc(
            Convert token id to token text

            Args:
                    id (int): token id
            
            Returns:
                    output token text to the given id


        )pbdoc")
        .def("convert_token_to_id", &GPT2Tokenizer::ConvertTokenToId, py::arg("token"),
             py::return_value_policy::automatic,
             R"pbdoc(
            Convert token text to token id

            Args:
                    token (str): token text
            
            Returns:
                    output token id to the given token text


        )pbdoc");
    ;
}

}  // namespace python
}  // namespace pyis
