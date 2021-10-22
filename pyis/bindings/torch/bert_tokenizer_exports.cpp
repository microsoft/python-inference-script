// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)
#include <memory>

#include "pyis/ops/tokenizer/bert_tokenizer.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace torchscript {

using pyis::ops::BertTokenizer;

struct BertTokenizerAdapter : public ::torch::CustomClassHolder {
  public:
    BertTokenizerAdapter() = default;

    explicit BertTokenizerAdapter(const std::string& vocab_file_path, bool do_lower_case = true,
                                  bool do_basic_tokenize = true, const std::string& cls_token = "[CLS]",
                                  const std::string& sep_token = "[SEP]", const std::string& unk_token = "[UNK]",
                                  const std::string& pad_token = "[PAD]", const std::string& mask_token = "[MASK]",
                                  bool tokenize_chinese_chars = true, bool strip_accents = false,
                                  const std::string& suffix_indicator = "##") {
        obj_ = std::make_shared<BertTokenizer>(vocab_file_path, do_lower_case, do_basic_tokenize, cls_token, sep_token,
                                               unk_token, pad_token, mask_token, tokenize_chinese_chars, strip_accents,
                                               suffix_indicator);
    }

    explicit BertTokenizerAdapter(std::shared_ptr<BertTokenizer>& obj) { this->obj_ = std::move(obj); }

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

    std::string decode(const std::vector<int64_t>& ids) { return obj_->Decode(ids); }

    std::string convert_id_to_token(int64_t id) { return obj_->ConvertIdToToken(id); }

    int64_t convert_token_to_id(const std::string& str) { return obj_->ConvertTokenToId(str); }

    std::string serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<BertTokenizer> obj_;
};

void init_bert_tokenizer(::torch::Library& m) {
    m.class_<BertTokenizerAdapter>("BertTokenizer")
        .def(::torch::init<std::string, bool, bool, std::string, std::string, std::string, std::string, std::string,
                           bool, bool, std::string>(),
             "",
             {torch::arg("vocab_file"), torch::arg("do_lower_case") = true, torch::arg("do_basic_tokenize") = true,
              torch::arg("cls_token") = "[CLS]", torch::arg("sep_token") = "[SEP]", torch::arg("unk_token") = "[UNK]",
              torch::arg("pad_token") = "[PAD]", torch::arg("mask_token") = "[MASK]",
              torch::arg("tokenize_chinese_chars") = true, torch::arg("strip_accents") = false,
              torch::arg("suffix_indicator") = "##"})
        .def("tokenize", &BertTokenizerAdapter::tokenize, "", {torch::arg("query")})
        .def("encode", &BertTokenizerAdapter::encode, "",
             {torch::arg("str"), torch::arg("max_length") = static_cast<int64_t>(1e15)})
        .def("encode2", &BertTokenizerAdapter::encode2, "",
             {torch::arg("str1"), torch::arg("str2"), torch::arg("max_length") = static_cast<int64_t>(1e15),
              torch::arg("truncation_strategy") = "longest_first"})
        .def("encode_plus", &BertTokenizerAdapter::encode_plus, "",
             {torch::arg("str"), torch::arg("max_length") = static_cast<int64_t>(1e15)})
        .def("encode_plus2", &BertTokenizerAdapter::encode_plus2, "",
             {torch::arg("str1"), torch::arg("str2"), torch::arg("max_length") = static_cast<int64_t>(1e15),
              torch::arg("truncation_strategy") = "longest_first"})
        .def("decode", &BertTokenizerAdapter::decode, "", {torch::arg("ids")})
        .def("convert_id_to_token", &BertTokenizerAdapter::convert_id_to_token, "", {torch::arg("id")})
        .def("convert_token_to_id", &BertTokenizerAdapter::convert_token_to_id, "", {torch::arg("token")})
        .def_pickle(
            [](const c10::intrusive_ptr<BertTokenizerAdapter>& self) -> std::string {
                return self->serialize(ModelContext::GetActive()->Storage());
            },
            [](const std::string& state) -> c10::intrusive_ptr<BertTokenizerAdapter> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<BertTokenizer>(state);
                return c10::make_intrusive<BertTokenizerAdapter>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis