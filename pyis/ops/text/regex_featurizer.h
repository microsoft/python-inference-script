// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <regex>
#include <string>
#include <vector>

#include "pyis/ops/text/cedar_trie.h"
#include "pyis/ops/text/text_feature.h"
#include "pyis/share/cached_object.h"
#include "pyis/share/model_storage.h"

namespace pyis {
namespace ops {

class RegexFeaturizer {
  public:
    explicit RegexFeaturizer(const std::vector<std::string>& regexes);
    ~RegexFeaturizer() = default;
    // default constructor used for deserilization only.
    RegexFeaturizer() = default;

    // disable the following copy semantics
    RegexFeaturizer(const RegexFeaturizer& o) = delete;
    RegexFeaturizer& operator=(const RegexFeaturizer& o) = delete;

    void AddRegex(const std::string& regex);
    std::vector<TextFeature> Transform(const std::vector<std::string>& tokens) const;

    void Load(const std::string& regex_file, ModelStorage& storage);
    void Save(const std::string& regex_file, ModelStorage& storage);
    std::string Serialize(ModelStorage& storage);
    void Deserialize(const std::string& state, ModelStorage& storage);

  private:
    std::vector<std::string> regex_patterns_;
    std::vector<std::regex> regexes_;
};

}  // namespace ops
}  // namespace pyis
