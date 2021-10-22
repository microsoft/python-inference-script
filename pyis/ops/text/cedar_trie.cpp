// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "pyis/ops/text/cedar_trie.h"

namespace pyis {
namespace ops {

pyis::ops::CedarTrie::CedarTrie() { trie_ = new Cedar::Trie(); }

pyis::ops::CedarTrie::~CedarTrie() { delete trie_; }

// dump content of the trie into result

std::vector<std::tuple<std::string, int>> pyis::ops::CedarTrie::Items() const {
    std::vector<std::tuple<std::string, int>> result;
    trie_->Dump(result);
    return result;
}

std::vector<std::tuple<std::string, int>> pyis::ops::CedarTrie::Predict(const std::string& prefix) const {
    auto iterator = trie_->Predict(prefix.c_str());
    std::vector<std::tuple<std::string, int>> result;
    Cedar::TrieResult* current_result;
    while ((current_result = const_cast<Cedar::TrieResult*>(iterator.Next())) != nullptr) {
        result.emplace_back(std::make_tuple(prefix + current_result->Key(), current_result->Value()));
    }
    return result;
}

std::vector<std::tuple<std::string, int>> pyis::ops::CedarTrie::Prefix(const std::string& query) const {
    auto trie_results = trie_->Prefix(query.c_str());
    std::vector<std::tuple<std::string, int>> result;
    result.resize(trie_results.size());
    for (size_t i = 0; i < trie_results.size(); ++i) {
        result[i] = std::make_tuple(trie_results[i].Key(), trie_results[i].Value());
    }
    return result;
}

Expected<std::tuple<std::string, int>> pyis::ops::CedarTrie::LongestPrefix(const std::string& query) const {
    auto query_result = trie_->LongestPrefix(query.c_str());
    if (query_result.Value() == Cedar::trie_t::CEDAR_NO_VALUE) {
        return Expected<std::tuple<std::string, int>>(std::runtime_error("query prefix not exists"));
    }
    return Expected<std::tuple<std::string, int>>(std::make_tuple(query_result.Key(), query_result.Value()));
}

Expected<int> CedarTrie::Lookup(const std::string& key) const {
    int ret = trie_->Lookup(key.c_str());
    if (ret == Cedar::trie_t::CEDAR_NO_VALUE) {
        return Expected<int>(std::runtime_error("key not found"));
    }
    return Expected<int>(ret);
}

int CedarTrie::Erase(const std::string& key) { return trie_->Erase(key.c_str()); }

// 1 for reserved value, 0 for inserted, -1 for updated.

int CedarTrie::Insert(const std::string& key, int value) {
    if (value <= (INT_MIN + 1)) {
        return 1;
    }
    return trie_->Insert(key.c_str(), value);
}

bool CedarTrie::Contains(const std::string& key) const noexcept { return Lookup(key).has_value(); }

size_t CedarTrie::NumKeys() const { return trie_->NumKeys(); }

Expected<void> CedarTrie::Open(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (ifs.fail()) {
        return Expected<void>(std::runtime_error(fmt_str("fail to open %s", path.c_str())));
    }
    Open(ifs);
    ifs.close();
    return Expected<void>();
}

void CedarTrie::Save(std::ostream& os) { trie_->Save(os); }

Expected<void> CedarTrie::Save(const std::string& path) {
    std::ofstream ofs(path, std::ios::binary);
    if (ofs.fail()) {
        return Expected<void>(std::runtime_error(fmt_str("fail to open %s", path.c_str())));
    }
    Save(ofs);
    ofs.close();
    return Expected<void>();
}

void CedarTrie::Reset() {
    delete trie_;
    trie_ = new Cedar::Trie();
}

int CedarTrie::Build(const std::vector<std::tuple<std::string, int>>& data) {
    int cnt = 0;
    for (const auto& record : data) {
        if (Insert(std::get<0>(record), std::get<1>(record)) == 0) {
            cnt++;
        }
    }
    return cnt;
}

Expected<int> CedarTrie::BuildFromFile(const std::string& path) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        return Expected<int>(std::runtime_error(fmt_str("Failed to open file %s.", path.c_str())));
    }
    int cnt = 0;
    std::string line;
    while (getline(ifs, line)) {
        std::stringstream ss(line);
        std::string key;
        int value;
        if (ss >> key) {
            if (!(ss >> value)) {
                value = 0;
            }
            if (Insert(key, value) == 0) {
                cnt++;
            }
        }
    }
    ifs.close();
    return Expected<int>(cnt);
}

std::string CedarTrie::Serialize(ModelStorage& storage) {
    std::string data_file = storage.uniq_file("cedar_trie", ".data.bin");
    Save(*storage.open_ostream(data_file));

    JsonPersistHelper jph(1);
    jph.add_file("data_file", data_file);
    std::string state = jph.serialize();
    return state;
}

void CedarTrie::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state, storage);
    int version = jph.version();
    if (1 == version) {
        std::string data_file = jph.get_file("data_file");
        Open(*storage.open_istream(data_file));
        return;
    }

    PYIS_THROW("CedarTrie v%d is incompatible with the runtime", version);
}

}  // namespace ops
}  // namespace pyis