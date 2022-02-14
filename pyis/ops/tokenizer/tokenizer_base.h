// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <codecvt>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "pyis/share/exception.h"

namespace pyis {
namespace ops {

class Tokenizer {
  public:
    Tokenizer() = default;
    explicit Tokenizer(std::string vocab_file, std::string cls_token = "[CLS]", std::string sep_token = "[SEP]",
                       std::string unk_token = "[UNK]", std::string pad_token = "[PAD]",
                       std::string mask_token = "[MASK]");
    void Truncate(std::vector<int64_t>& ids, int64_t max_len);
    void Truncate(std::vector<int64_t>& input1, std::vector<int64_t>& input2, const std::string& truncate_strategy,
                  int64_t max_len);
    // std::string Padding();
    std::vector<int64_t> Encode(const std::string& str, int64_t max_length = 1e15);
    std::vector<int64_t> Encode(const std::string& str1, const std::string& str2, int64_t max_length = 1e15,
                                const std::string& truncation_strategy = "longest_first");
    std::vector<std::tuple<int64_t, int64_t, int64_t>> EncodePlus(const std::string& str, int64_t max_length = 1e15);
    std::vector<std::tuple<int64_t, int64_t, int64_t>> EncodePlus(
        const std::string& str1, const std::string& str2, int64_t max_length = 1e15,
        const std::string& truncation_strategy = "longest_first");
    virtual std::vector<std::string> Tokenize(const std::string& str) = 0;
    virtual std::string Decode(const std::vector<int64_t>& code, bool skip_special_tokens,
                               bool clean_up_tokenization_spaces);
    std::string ConvertIdToToken(int64_t id);
    int64_t ConvertTokenToId(const std::string& str);
    virtual std::vector<int64_t> AddSpecialToken(const std::vector<int64_t>& code);
    virtual std::vector<int64_t> AddSpecialToken(const std::vector<int64_t>& ids1, const std::vector<int64_t>& ids2);
    std::vector<int64_t> GenerateTypeId(const std::vector<int64_t>& ids);
    std::vector<int64_t> GenerateTypeId(const std::vector<int64_t>& ids1, const std::vector<int64_t>& ids2);
    std::map<std::string, int64_t> GetVocab();

  protected:
    virtual void LoadVocabFile();
    void CleanUpTokenization(std::string& str);
    std::string cls_token_;
    std::string sep_token_;
    std::string unk_token_;
    std::string pad_token_;
    std::string mask_token_;

    std::string vocab_file_;
    std::unordered_map<std::string, int64_t> vocab_map_;
    std::unordered_map<int64_t, std::string> vocab_map_reverse_;
};
}  // namespace ops
}  // namespace pyis