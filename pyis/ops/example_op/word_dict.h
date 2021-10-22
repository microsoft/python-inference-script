// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "pyis/share/cached_object.h"
#include "pyis/share/expected.hpp"
#include "pyis/share/model_storage.h"

namespace pyis {
namespace ops {

class WordDict : public CachedObject<WordDict> {
  public:
    explicit WordDict(const std::string& data_file);
    ~WordDict() = default;
    WordDict(WordDict&& o) = default;
    // default constructor used for deserilization only.
    WordDict();

    // disable the following copy semantics
    WordDict(const WordDict& o) = delete;
    WordDict& operator=(const WordDict& o) = delete;

    std::vector<std::string> Translate(const std::vector<std::string>& tokens);

    std::string Serialize(ModelStorage& storage);
    void Deserialize(const std::string& state, ModelStorage& storage);

  private:
    void Load(const std::string& data_file, ModelStorage& storage);
    void Save(const std::string& data_file, ModelStorage& storage);

    std::unordered_map<std::string, std::string> mapping_;
    int thread_num_;
};

}  // namespace ops
}  // namespace pyis