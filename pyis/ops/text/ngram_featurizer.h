// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "pyis/ops/text/text_feature.h"
#include "pyis/ops/text/trie.h"
#include "pyis/share/cached_object.h"
#include "pyis/share/model_storage.h"

namespace pyis {
namespace ops {

class NGramFeaturizer : public CachedObject<NGramFeaturizer> {
  public:
    NGramFeaturizer(int order, bool boundaries);
    ~NGramFeaturizer() = default;
    // default constructor used for deserilization only.
    NGramFeaturizer() = default;

    // disable the following copy semantics
    NGramFeaturizer(const NGramFeaturizer& o) = delete;
    NGramFeaturizer& operator=(const NGramFeaturizer& o) = delete;

    void Fit(const std::vector<std::string>& tokens);
    std::vector<TextFeature> Transform(const std::vector<std::string>& tokens) const;

    void DumpNGram(std::string& ngram_file);
    void LoadNGram(std::string& ngram_file);
    std::string Serialize(ModelStorage& storage);
    void Deserialize(const std::string& state, ModelStorage& storage);

  private:
    void AddNGram(std::vector<std::string>& tokens, int begin, int end);
    void AddNGram(const std::string& ngram, uint32_t id);

    Trie trie_;
    int order_;
    bool boundaries_;
    uint32_t next_id_;  // cedar's value is 4 bytes

    static const std::string BOS_MARK;
    static const std::string EOS_MARK;
};

}  // namespace ops
}  // namespace pyis
