// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "wordpiece_tokenizer.h"

pyis::ops::WordpieceTokenizer::WordpieceTokenizer(const std::string& vocab_file, const std::string& cls_token,
                                                  const std::string& sep_token, const std::string& unk_token,
                                                  const std::string& pad_token, const std::string& mask_token,
                                                  std::string suffix_indicator)
    : Tokenizer(vocab_file, cls_token, sep_token, unk_token, pad_token, mask_token),
      word_piece_prefix_(std::move(suffix_indicator)) {}

std::vector<std::string> pyis::ops::WordpieceTokenizer::Tokenize(const std::string& str) {
    std::vector<std::string> result;
    std::string token;
    for (auto c : str) {
        if (c == ' ' && !token.empty()) {
            GreedySearch(token, result);
            token.clear();
            continue;
        }

        token += c;
    }

    if (!token.empty()) {
        GreedySearch(token, result);
    }

    return result;
}

std::vector<std::string> pyis::ops::WordpieceTokenizer::Tokenize(const std::vector<std::string>& tokens) {
    std::vector<std::string> result;
    for (const auto& token : tokens) {
        GreedySearch(token, result);
    }

    return result;
}

inline void pyis::ops::WordpieceTokenizer::GreedySearch(const std::string& token,
                                                        std::vector<std::string>& tokenized_result) {
    int start = 0;
    int end = -1;
    std::string substr;
    for (; start < token.size();) {
        end = token.size();
        bool is_found = false;
        // try to found the longest matched sub-token in vocab
        for (; start < end;) {
            substr = token.substr(start, end - start);
            if (start > 0) {
                substr = word_piece_prefix_ + substr;  // NOLINT
            }
            if (vocab_map_.count(substr) != 0U) {
                is_found = true;
                break;
            }
            end -= 1;
        }
        // token not found in vocab
        if (!is_found) {
            tokenized_result.push_back(unk_token_);
            break;
        }

        tokenized_result.push_back(substr);
        start = end;
    }
}
