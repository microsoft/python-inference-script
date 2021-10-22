// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>

#include "pyis/share/exception.h"

namespace pyis {
namespace ops {

class NaiveTokenizer {
  public:
    explicit NaiveTokenizer(std::string vocab_file_path) : vocab_file_path_(std::move(vocab_file_path)) {
        this->initialize_vocab_map();
    };

    std::vector<int64_t> tokenize(const std::string& str) {
        size_t start = 0;
        size_t end = str.find(delimiter_);
        std::vector<int64_t> indx_list{};
        while (end != std::string::npos) {
            auto substr = str.substr(start, end - start);
            indx_list.emplace_back(vocab_map_[substr]);
            start = end + delimiter_.length();
            end = str.find(delimiter_, start);
        }
        indx_list.emplace_back(vocab_map_[str.substr(start, end - start)]);
        indx_list.resize(10);
        return indx_list;
    }

  private:
    const std::string delimiter_ = " ";
    std::unordered_map<std::string, int64_t> vocab_map_;
    std::string vocab_file_path_;

    void initialize_vocab_map() {
        std::ifstream file(vocab_file_path_);
        std::string line;
        int64_t index = 0;
        while (std::getline(file, line)) {
            line = std::regex_replace(line, std::regex("\r"), "");
            if (line.empty()) {
                continue;
            }
            if (vocab_map_.find(line) != vocab_map_.end()) {
                PYIS_THROW("duplicate word %s", line.c_str());
            }
            vocab_map_[line] = index;
            ++index;
        }
    }
};

}  // namespace ops
}  // namespace pyis
