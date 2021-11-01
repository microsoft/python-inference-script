// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "tokenizer_base.h"

#include <pyis/share/str_utils.h>

namespace pyis {
namespace ops {
Tokenizer::Tokenizer(std::string vocab_file, std::string cls_token, std::string sep_token, std::string unk_token,
                     std::string pad_token, std::string mask_token)
    : vocab_file_(std::move(vocab_file)),
      cls_token_(std::move(cls_token)),
      sep_token_(std::move(sep_token)),
      unk_token_(std::move(unk_token)),
      pad_token_(std::move(pad_token)),
      mask_token_(std::move(mask_token)) {
    LoadVocabFile();
}
void Tokenizer::Truncate(std::vector<int64_t>& ids, int64_t max_len) {
    if (max_len < 0 || max_len >= ids.size()) {
        return;
    }
    ids.resize(max_len);
}

void Tokenizer::Truncate(std::vector<int64_t>& input1, std::vector<int64_t>& input2,
                         const std::string& truncate_strategy, int64_t max_len) {
    if (max_len < 0 || (input1.size() + input2.size() <= max_len)) {
        return;
    }

    auto input1_keep_len = input1.size();
    auto input2_keep_len = input2.size();
    auto half_max_len = max_len / 2;

    if (truncate_strategy == "longest_first" || truncate_strategy == "longest_from_back") {
        if ((input1_keep_len > half_max_len) && (input2_keep_len > half_max_len)) {
            input1_keep_len = max_len - half_max_len;
            input2_keep_len = half_max_len;
        } else if (input2_keep_len > input1_keep_len) {
            input2_keep_len = max_len - input1_keep_len;
        } else {
            input1_keep_len = max_len - input2_keep_len;
        }

        if (truncate_strategy == "longest_first") {
            input1.resize(input1_keep_len);
            input2.resize(input2_keep_len);
        } else {
            input1.erase(input1.begin(), input1.end() - input1_keep_len);
            input2.erase(input2.begin(), input2.end() - input2_keep_len);
        }
        return;
    }
    if (truncate_strategy == "only_first") {
        input1_keep_len = max_len - input2_keep_len;
        if (input1_keep_len <= 0) {
            PYIS_THROW("We need to remove %lld to truncate the input\nbut the first sequence has a length %u.",
                       max_len - input2_keep_len, input1.size());
        }
        input1.resize(input1_keep_len);
        return;
    }
    if (truncate_strategy == "only_second") {
        input2_keep_len = max_len - input1_keep_len;
        if (input2_keep_len <= 0) {
            PYIS_THROW("We need to remove %lld to truncate the input\nbut the second sequence has a length %u.",
                       max_len - input1_keep_len, input2.size());
        }
        input2.resize(input1_keep_len);
        return;
    }
    PYIS_THROW("Unknown truncation strategy %s", truncate_strategy.c_str());
}

std::vector<int64_t> Tokenizer::Encode(const std::string& str, int64_t max_length) {
    std::vector<std::string> tokenized_result = Tokenize(str);
    std::vector<std::int64_t> encoded_result;
    encoded_result.resize(tokenized_result.size());
    for (size_t i = 0; i < tokenized_result.size(); i++) {
        encoded_result[i] = ConvertTokenToId(tokenized_result[i]);
    }
    Truncate(encoded_result, max_length);
    return AddSpecialToken(encoded_result);
}

std::vector<int64_t> Tokenizer::Encode(const std::string& str1, const std::string& str2, int64_t max_length,
                                       const std::string& truncation_strategy) {
    auto code1 = Encode(str1);
    auto code2 = Encode(str2);
    Truncate(code1, code2, truncation_strategy, max_length);
    return AddSpecialToken(code1, code2);
}

std::vector<std::tuple<int64_t, int64_t, int64_t>> Tokenizer::EncodePlus(const std::string& str, int64_t max_length) {
    std::vector<std::tuple<int64_t, int64_t, int64_t>> result;

    auto encode_result = Encode(str, max_length);
    auto type_id = GenerateTypeId(encode_result);
    std::vector<int64_t> attention_mask(encode_result.size(), 1);

    result.resize(encode_result.size());
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = std::make_tuple(encode_result[i], type_id[i], attention_mask[i]);
    }
    return result;
}

std::vector<std::tuple<int64_t, int64_t, int64_t>> Tokenizer::EncodePlus(const std::string& str1,
                                                                         const std::string& str2, int64_t max_length,
                                                                         const std::string& truncation_strategy) {
    auto code1 = Encode(str1);
    auto code2 = Encode(str2);
    Truncate(code1, code2, truncation_strategy, max_length);
    auto input_ids = AddSpecialToken(code1, code2);
    auto type_ids = GenerateTypeId(code1, code2);
    std::vector<int64_t> attention_mask(input_ids.size(), 1);
    std::vector<std::tuple<int64_t, int64_t, int64_t>> result;
    result.resize(input_ids.size());
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = std::make_tuple(input_ids[i], type_ids[i], attention_mask[i]);
    }
    return result;
}

std::string Tokenizer::Decode(const std::vector<int64_t>& code, bool skip_special_tokens,
                              bool clean_up_tokenization_spaces) {
    std::set<std::string> special_tokens({unk_token_, pad_token_, cls_token_, mask_token_, sep_token_});
    std::vector<std::string> sub_texts;
    for (const auto& id : code) {
        std::string current_token = ConvertIdToToken(id);
        if (!skip_special_tokens || special_tokens.count(current_token) == 0U) {
            sub_texts.emplace_back(current_token);
        }
    }
    std::string text = join_str(sub_texts, " ");
    if (clean_up_tokenization_spaces) {
        CleanUpTokenization(text);
    }
    return text;
}

std::string Tokenizer::ConvertIdToToken(int64_t id) {
    auto worditer = vocab_map_reverse_.find(id);
    if (worditer != vocab_map_reverse_.end()) {
        return worditer->second;
    }
    return unk_token_;
}

int64_t Tokenizer::ConvertTokenToId(const std::string& str) {
    auto worditer = vocab_map_.find(str);
    if (worditer != vocab_map_.end()) {
        return worditer->second;
    }
    return -1;
}

std::vector<int64_t> Tokenizer::AddSpecialToken(const std::vector<int64_t>& code) {
    std::vector<int64_t> result;
    result.reserve(code.size() + 2);
    result.emplace_back(ConvertTokenToId(cls_token_));
    result.insert(result.end(), code.begin(), code.end());
    result.emplace_back(ConvertTokenToId(sep_token_));
    return result;
}

std::vector<int64_t> Tokenizer::AddSpecialToken(const std::vector<int64_t>& ids1, const std::vector<int64_t>& ids2) {
    std::vector<int64_t> result;
    result.reserve(ids1.size() + ids2.size() + 3);
    result.push_back(ConvertTokenToId(cls_token_));
    result.insert(result.end(), ids1.begin(), ids1.end());
    result.push_back(ConvertTokenToId(sep_token_));
    result.insert(result.end(), ids2.begin(), ids2.end());
    result.push_back(ConvertTokenToId(sep_token_));
    return result;
}

std::vector<int64_t> Tokenizer::GenerateTypeId(const std::vector<int64_t>& ids) {
    return std::vector<int64_t>(ids.size() + 2, 0);
}

std::vector<int64_t> Tokenizer::GenerateTypeId(const std::vector<int64_t>& ids1, const std::vector<int64_t>& ids2) {
    std::vector<int64_t> result;
    result.reserve(ids1.size() + ids2.size() + 3);
    result.insert(result.end(), ids1.size() + 2, 0);
    result.insert(result.end(), ids2.size() + 1, 1);
    return result;
}

inline std::map<std::string, int64_t> Tokenizer::GetVocab() {
    return std::map<std::string, int64_t>(vocab_map_.begin(), vocab_map_.end());
}

void Tokenizer::LoadVocabFile() {
    auto file = std::ifstream(vocab_file_);
    std::string line;
    int64_t index = 0;
    while (std::getline(file, line)) {
        rtrim_str(line);
        if (line.empty()) {
            continue;
        }
        if (vocab_map_.find(line) != vocab_map_.end()) {
            continue;
        }
        vocab_map_[line] = index;
        vocab_map_reverse_[index] = line;
        ++index;
    }
}

void Tokenizer::CleanUpTokenization(std::string& str) {
    FindAndReplaceAll(str, " .", ".");
    FindAndReplaceAll(str, " ?", "?");
    FindAndReplaceAll(str, " !", "!");
    FindAndReplaceAll(str, " ,", ",");
    FindAndReplaceAll(str, " ' ", "'");
    FindAndReplaceAll(str, " n't", "n't");
    FindAndReplaceAll(str, " 'm", "'m");
    FindAndReplaceAll(str, " 's", "'s");
    FindAndReplaceAll(str, " 've", "'ve");
    FindAndReplaceAll(str, " 're", "'re");
}

}  // namespace ops
}  // namespace pyis