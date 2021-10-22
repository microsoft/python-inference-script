// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "basic_tokenizer.h"

std::vector<std::string> pyis::ops::BasicTokenizer::Tokenize(const std::string& str) const {
    std::string text(str);
    std::vector<std::string> result;
    std::string token;
    auto push_current_token_and_clear = [&result, &token]() {
        if (!token.empty()) {
            result.emplace_back(token);
            token.clear();
        }
    };
    auto push_single_char_and_clear = [&result, &token](char32_t c) {
        token += c;
        result.emplace_back(token);
        token.clear();
    };

    if (strip_accents_) {
        for (auto& c : text) {
            c = StripAccent(c);
        }
    }

    if (do_lower_case_) {
        for (auto& c : text) {
            c = ::tolower(c);
        }
    }

    for (auto c : text) {
        if (tokenize_chinese_chars_ && isCJK(c)) {
            push_current_token_and_clear();
            push_single_char_and_clear(c);
            continue;
        }
        if (strip_accents_ && IsAccent(c)) {
            continue;
        }

        if (tokenize_punctuation_ && (::iswpunct(c) != 0 || c == wint_t(0x2019))) {
            push_current_token_and_clear();
            push_single_char_and_clear(c);
            continue;
        }

        if (::iswspace(c) != 0) {
            push_current_token_and_clear();
            continue;
        }

        if (remove_control_chars_ && ::iswcntrl(c) != 0) {
            continue;
        }

        token += c;
    }
    push_current_token_and_clear();
    return result;
}

pyis::ops::BasicTokenizer::BasicTokenizer(bool do_lower_case, bool tokenize_chinese_chars, bool strip_accents,
                                          bool tokenize_punctuation, bool remove_control_chars)
    : do_lower_case_(do_lower_case),
      tokenize_chinese_chars_(tokenize_chinese_chars),
      strip_accents_(strip_accents),
      tokenize_punctuation_(tokenize_punctuation),
      remove_control_chars_(remove_control_chars) {}
