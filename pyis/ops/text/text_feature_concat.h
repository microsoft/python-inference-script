// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "pyis/ops/text/text_feature.h"
#include "pyis/share/cached_object.h"
#include "pyis/share/model_storage.h"

namespace pyis {
namespace ops {

// No caching support. Very unlikely to be identical.
class TextFeatureConcat {
  public:
    explicit TextFeatureConcat(uint64_t start_id);
    ~TextFeatureConcat() = default;
    // default constructor used for deserilization only.
    TextFeatureConcat();

    // disable the following copy semantics
    TextFeatureConcat(const TextFeatureConcat& o) = delete;
    TextFeatureConcat& operator=(const TextFeatureConcat& o) = delete;

    void Fit(const std::vector<std::vector<TextFeature>>& feature_groups);
    std::vector<TextFeature> Transform(const std::vector<std::vector<TextFeature>>& feature_groups);

    void Load(const std::string& mapping_file, ModelStorage& storage);
    void Save(const std::string& data_file, ModelStorage& storage);
    std::string Serialize(ModelStorage& storage);
    void Deserialize(const std::string& state, ModelStorage& storage);

  private:
    struct TupleHash {
        size_t operator()(std::tuple<uint16_t, uint64_t> const& arg) const noexcept {
            size_t seed = 0;
            seed ^= std::hash<uint16_t>{}(std::get<0>(arg)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<uint64_t>{}(std::get<1>(arg)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    uint64_t next_id_;
    std::unordered_map<std::tuple<uint16_t, uint64_t>, uint64_t, TupleHash> mapping_;
};

}  // namespace ops
}  // namespace pyis
