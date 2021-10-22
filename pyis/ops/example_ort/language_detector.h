// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <cctype>
#include <string>

namespace pyis {
namespace ops {

class NaiveLanguageDetector {
  public:
    NaiveLanguageDetector() = default;

    int64_t detect(const std::string& str) {
        for (const auto& c : str) {
            if ((ispunct(c) != 0) || (isalpha(c) != 0) || (isspace(c) != 0)) {
                continue;
            }
            return 1;
        }
        return 0;
    }

    static bool is_english(int64_t lang_code) { return lang_code == 0; }
};

}  // namespace ops
}  // namespace pyis
