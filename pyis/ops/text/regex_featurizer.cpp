// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "regex_featurizer.h"

#include <fstream>  // std::ifstream
#include <sstream>

#include "pyis/share/exception.h"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/scope_guard.h"
#include "pyis/share/str_utils.h"

namespace pyis {
namespace ops {

RegexFeaturizer::RegexFeaturizer(const std::vector<std::string>& regexes) {
    for (const auto& p : regexes) {
        regex_patterns_.emplace_back(p);
        regexes_.emplace_back(p);
    }
}

void RegexFeaturizer::AddRegex(const std::string& regex) {
    regex_patterns_.emplace_back(regex);
    regexes_.emplace_back(regex);
}

std::vector<TextFeature> RegexFeaturizer::Transform(const std::vector<std::string>& tokens) const {
    std::vector<TextFeature> res;
    auto token_count = static_cast<uint32_t>(tokens.size());

    // compute sentence length
    uint32_t sentence_len = 0;
    for (const auto& token : tokens) {
        sentence_len += static_cast<uint32_t>(token.length());
    }
    sentence_len += (token_count - 1);
    if (sentence_len <= 0) {
        return res;
    }

    std::vector<char> sentence(sentence_len + 1);
    std::vector<std::pair<uint32_t, uint32_t>> token_inverse_indexes(sentence_len + 1,
                                                                     std::pair<uint32_t, uint32_t>(-1, -1));

    uint32_t offset = 0;
    const char delim = ' ';
    uint32_t index = 0;
    for (const auto& token : tokens) {
        // copy value to query
        memcpy(sentence.data() + offset, token.c_str(), token.length());
        std::pair<uint32_t, uint32_t> token_index(-1, -1);
        std::pair<uint32_t, uint32_t> token_inverse_index(-1, -1);
        token_inverse_indexes[offset].first = index;
        offset += token.length();
        token_inverse_indexes[offset].second = index;

        memcpy(sentence.data() + offset, &delim, 1);
        offset += 1;
        index += 1;
    }
    sentence[sentence.size() - 1] = '\0';

    uint32_t id = 0;
    for (const auto& p : regexes_) {
        std::regex_iterator<const char*> ite(sentence.data(), sentence.data() + sentence_len, p);
        std::regex_iterator<const char*> rend;

        while (ite != rend) {
            uint32_t match_pos = ite->position();
            uint32_t match_length = ite->length();

            uint32_t start_index = token_inverse_indexes[match_pos].first;
            uint32_t end_index = token_inverse_indexes[match_pos + match_length].second;
            if (start_index != -1 && end_index != -1) {
                res.emplace_back(id, 1.0, start_index, end_index);
            }
            ite++;
        }
        id++;
    }
    return res;
}

void RegexFeaturizer::Save(const std::string& regex_file, ModelStorage& storage) {
    auto ostream = storage.open_ostream(regex_file);
    for (auto& p : regex_patterns_) {
        ostream->write(p.c_str(), p.length());
        ostream->write("\n", 1);
    }
}

void RegexFeaturizer::Load(const std::string& regex_file, ModelStorage& storage) {
    auto istream = storage.open_istream(regex_file);
    std::string line;
    while (std::getline(*istream, line)) {
        line = trim_str(line);
        if (line.length() == 0) {
            continue;
        }
        regex_patterns_.emplace_back(line);
        regexes_.emplace_back(line);
    }
}

std::string RegexFeaturizer::Serialize(ModelStorage& storage) {
    std::string regex_file = storage.uniq_file("regex", ".txt");
    Save(regex_file, storage);

    JsonPersistHelper jph(1);
    jph.add_file("regex_file", regex_file);

    return jph.serialize();
}

void RegexFeaturizer::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state);
    int version = jph.version();

    if (1 == version) {
        std::string regex_file = jph.get_file("regex_file");
        Load(regex_file, storage);
    } else {
        PYIS_THROW("RegexFeaturizer v%d is incompatible with the runtime", version);
    }
}

}  // namespace ops
}  // namespace pyis
