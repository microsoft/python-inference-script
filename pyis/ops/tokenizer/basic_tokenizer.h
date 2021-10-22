// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <codecvt>
#include <locale>

#include "pyis/ops/tokenizer/tokenizer_base.h"
#include "pyis/share/str_utils.h"

namespace pyis {
namespace ops {

class BasicTokenizer {
  public:
    std::vector<std::string> Tokenize(const std::string& str) const;
    explicit BasicTokenizer(bool do_lower_case, bool tokenize_chinese_chars, bool strip_accents,
                            bool tokenize_punctuation, bool remove_control_chars);

  private:
    bool do_lower_case_;
    bool strip_accents_;
    bool tokenize_chinese_chars_;
    bool tokenize_punctuation_;
    bool remove_control_chars_;
};

}  // namespace ops
}  // namespace pyis