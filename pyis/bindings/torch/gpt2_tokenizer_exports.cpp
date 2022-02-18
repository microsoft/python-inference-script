// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)
#include <memory>

#include "pyis/ops/tokenizer/gpt2_tokenizer.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace torchscript {

using pyis::ops::GPT2Tokenizer;

struct GPT2TokenizerAdaptor : public ::torch::CustomClassHolder {
  public:
    GPT2TokenizerAdaptor() = default;

    explicit GPT2TokenizerAdaptor(const std::string& vocab_file_path, const std::string& merges_file_path,
                                  const std::string& unk_token = "<|endoftext|>",
                                  const std::string& bos_token = "<|endoftext|>",
                                  const std::string& eos_token = "<|endoftext|>", bool add_prefix_space = false) {
        obj_ = std::make_shared<GPT2Tokenizer>(vocab_file_path, merges_file_path, unk_token, bos_token, eos_token,
                                               add_prefix_space);
    }

    explicit GPT2TokenizerAdaptor(std::shared_ptr<GPT2Tokenizer>& obj) { this->obj_ = std::move(obj); }

    std::vector<std::string> tokenize(const std::string& str) { return obj_->Tokenize(str); }

    std::vector<int64_t> encode(const std::string& str, int64_t max_length) { return obj_->Encode(str, max_length); }

    std::vector<int64_t> encode2(const std::string& str1, const std::string& str2, int64_t max_length,
                                 const std::string& truncation_strategy) {
        return obj_->Encode(str1, str2, max_length, truncation_strategy);
    }

    std::vector<std::tuple<int64_t, int64_t, int64_t> > encode_plus(const std::string& str, int64_t max_length) {
        return obj_->EncodePlus(str, max_length);
    }

    std::vector<std::tuple<int64_t, int64_t, int64_t> > encode_plus2(const std::string& str1, const std::string& str2,
                                                                     int64_t max_length,
                                                                     const std::string& truncation_strategy) {
        return obj_->EncodePlus(str1, str2, max_length, truncation_strategy);
    }

    std::string decode(const std::vector<int64_t>& ids, bool skip_special_tokens = false,
                       bool clean_up_tokenization_spaces = true) {
        return obj_->Decode(ids, skip_special_tokens, clean_up_tokenization_spaces);
    }

    std::string convert_id_to_token(int64_t id) { return obj_->ConvertIdToToken(id); }

    int64_t convert_token_to_id(const std::string& str) { return obj_->ConvertTokenToId(str); }

    std::string serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<GPT2Tokenizer> obj_;
};

void init_bert_tokenizer(::torch::Library& m) {
    m.class_<GPT2TokenizerAdaptor>("BertTokenizer")
        .def(::torch::init<std::string, std::string, std::string, std::string, std::string, bool>(), "",
             {torch::arg("vocab_file"), torch::arg("merges_file") = true, torch::arg("bos_token") = "<|endoftext|>",
              torch::arg("eos_token") = "<|endoftext|>", torch::arg("unk_token") = "<|endoftext|>",
              torch::arg("add_prefix_space") = false})
        .def("tokenize", &GPT2TokenizerAdaptor::tokenize, "", {torch::arg("query")})
        .def("encode", &GPT2TokenizerAdaptor::encode, "",
             {torch::arg("str"), torch::arg("max_length") = static_cast<int64_t>(1e15)})
        .def("encode2", &GPT2TokenizerAdaptor::encode2, "",
             {torch::arg("str1"), torch::arg("str2"), torch::arg("max_length") = static_cast<int64_t>(1e15),
              torch::arg("truncation_strategy") = "longest_first"})
        .def("encode_plus", &GPT2TokenizerAdaptor::encode_plus, "",
             {torch::arg("str"), torch::arg("max_length") = static_cast<int64_t>(1e15)})
        .def("encode_plus2", &GPT2TokenizerAdaptor::encode_plus2, "",
             {torch::arg("str1"), torch::arg("str2"), torch::arg("max_length") = static_cast<int64_t>(1e15),
              torch::arg("truncation_strategy") = "longest_first"})
        .def("decode", &GPT2TokenizerAdaptor::decode, "",
             {torch::arg("ids"), torch::arg("skip_special_tokens") = false,
              torch::arg("clean_up_tokenization_spaces") = true})
        .def("convert_id_to_token", &GPT2TokenizerAdaptor::convert_id_to_token, "", {torch::arg("id")})
        .def("convert_token_to_id", &GPT2TokenizerAdaptor::convert_token_to_id, "", {torch::arg("token")})
        .def_pickle(
            [](const c10::intrusive_ptr<GPT2TokenizerAdaptor>& self) -> std::string {
                return self->serialize(ModelContext::GetActive()->Storage());
            },
            [](const std::string& state) -> c10::intrusive_ptr<GPT2TokenizerAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<GPT2Tokenizer>(state);
                return c10::make_intrusive<GPT2TokenizerAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis