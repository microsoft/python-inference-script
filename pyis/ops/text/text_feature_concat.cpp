// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "text_feature_concat.h"

#include "pyis/share/exception.h"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/model_storage_local.h"

namespace pyis {
namespace ops {

TextFeatureConcat::TextFeatureConcat() { next_id_ = 0; }

TextFeatureConcat::TextFeatureConcat(uint64_t start_id) { next_id_ = start_id; }

void TextFeatureConcat::Load(const std::string& mapping_file, ModelStorage& storage) {
    auto istream = storage.open_istream(mapping_file);
    std::string line;
    while (std::getline(*istream, line)) {
        if (line.length() == 0) {
            continue;
        }
        std::vector<std::string> tokens = split_str(line);
        auto group_id = static_cast<uint16_t>(stoul(tokens[0]));
        auto feature_id = static_cast<uint64_t>(stoul(tokens[1]));
        auto global_id = static_cast<uint64_t>(stoul(tokens[2]));
        std::tuple<uint16_t, uint64_t> k(group_id, feature_id);
        mapping_[k] = global_id;
        if (global_id >= next_id_) {
            next_id_ = global_id + 1;
        }
    }
}

void TextFeatureConcat::Save(const std::string& data_file, ModelStorage& storage) {
    auto ostream = storage.open_ostream(data_file);
    for (auto& kv : mapping_) {
        std::string line = fmt_str("%" PRIu16
                                   " "
                                   "%" PRIu64
                                   " "
                                   "%" PRIu64 "\n",
                                   std::get<0>(kv.first), std::get<1>(kv.first), kv.second);
        ostream->write(line.c_str(), line.length());
    }
}

void TextFeatureConcat::Fit(const std::vector<std::vector<TextFeature>>& feature_groups) {
    for (auto i = 0; i < feature_groups.size(); i++) {
        const std::vector<TextFeature>& features = feature_groups[i];

        for (const auto& f : features) {
            std::tuple<uint16_t, uint64_t> k(static_cast<uint16_t>(i), f.id());
            if (mapping_.count(k) == 0) {
                mapping_[k] = next_id_;
                next_id_++;
            }
        }
    }
}

std::vector<TextFeature> TextFeatureConcat::Transform(const std::vector<std::vector<TextFeature>>& feature_groups) {
    std::vector<TextFeature> res;
    for (auto i = 0; i < feature_groups.size(); i++) {
        const std::vector<TextFeature>& features = feature_groups[i];

        for (const auto& f : features) {
            std::tuple<uint16_t, uint64_t> k(static_cast<uint16_t>(i), f.id());
            if (mapping_.count(k) > 0) {
                TextFeature new_feature(f);
                new_feature.set_id(mapping_[k]);
                res.emplace_back(new_feature);
            }
        }
    }
    return res;
}

std::string TextFeatureConcat::Serialize(ModelStorage& storage) {
    std::string mapping_file = storage.uniq_file("text_feature_concat", ".mapping.txt");
    Save(mapping_file, storage);

    JsonPersistHelper jph(1);
    jph.add_file("mapping_file", mapping_file);
    std::string state = jph.serialize();
    return state;
}

void TextFeatureConcat::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state, storage);
    int version = jph.version();

    // check version for backward compatibility
    if (1 == version) {
        std::string mapping_file = jph.get_file("mapping_file");
        Load(mapping_file, storage);
        return;
    }

    PYIS_THROW("TextFeatureConcat v%d is incompatible with the runtime", version);
}

}  // namespace ops
}  // namespace pyis