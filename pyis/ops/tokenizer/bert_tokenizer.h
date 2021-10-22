// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "pyis/ops/tokenizer/basic_tokenizer.h"
#include "pyis/ops/tokenizer/tokenizer_base.h"
#include "pyis/ops/tokenizer/wordpiece_tokenizer.h"
#include "pyis/share/cached_object.h"
#include "pyis/share/exception.h"
#include "pyis/share/file_system.h"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/model_storage.h"
#include "pyis/share/model_storage_local.h"

namespace pyis {
namespace ops {

class BertTokenizer : public Tokenizer, public CachedObject<BertTokenizer> {
  public:
    BertTokenizer();
    explicit BertTokenizer(const std::string& vocab_file, bool do_lower_case = true, bool do_basic_tokenize = true,
                           const std::string& cls_token = "[CLS]", const std::string& sep_token = "[SEP]",
                           const std::string& unk_token = "[UNK]", const std::string& pad_token = "[PAD]",
                           const std::string& mask_token = "[MASK]", bool tokenize_chinese_chars = true,
                           bool strip_accents = false, const std::string& suffix_indicator = "##");
    std::vector<std::string> Tokenize(const std::string& str) override;
    bool do_lower_case_;
    bool do_basic_tokenize_;
    bool tokenize_chinese_chars_;
    bool strip_accents_;
    std::string suffix_indicator_;
    std::shared_ptr<BasicTokenizer> basic_tokenizer_;
    std::shared_ptr<WordpieceTokenizer> wordpiece_tokenizer_;

    std::string Serialize(ModelStorage& fs);
    void Deserialize(const std::string& state, ModelStorage& fs);
    std::string Decode(const std::vector<int64_t>& code) override;
};

}  // namespace ops
}  // namespace pyis
