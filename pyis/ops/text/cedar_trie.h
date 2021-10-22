// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <climits>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "pyis/share/cached_object.h"
#include "pyis/share/expected.hpp"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/model_storage_local.h"
#include "third_party/cedar/cedar_trie_impl.h"

namespace pyis {
namespace ops {

class CedarTrie : public CachedObject<CedarTrie> {
  private:
    Cedar::Trie* trie_;

  public:
    enum error_code { CEDAR_NO_VALUE = INT_MIN, CEDAR_NO_PATH = (INT_MIN + 1) };

    CedarTrie();

    ~CedarTrie();

    // dump content of the trie into result
    std::vector<std::tuple<std::string, int>> Items() const;

    std::vector<std::tuple<std::string, int>> Predict(const std::string& prefix) const;

    std::vector<std::tuple<std::string, int>> Prefix(const std::string& query) const;

    Expected<std::tuple<std::string, int>> LongestPrefix(const std::string& query) const;

    Expected<int> Lookup(const std::string& key) const;

    int Erase(const std::string& key);

    // 1 for reserved value, 0 for inserted, -1 for updated.
    int Insert(const std::string& key, int value = 0);

    bool Contains(const std::string& key) const noexcept;

    size_t NumKeys() const;

    void Open(std::istream& is) { trie_->Open(is); }

    Expected<void> Open(const std::string& path);

    void Save(std::ostream& os);

    Expected<void> Save(const std::string& path);

    void Reset();

    int Build(const std::vector<std::tuple<std::string, int>>& data);

    Expected<int> BuildFromFile(const std::string& path);

    std::string Serialize(ModelStorage& storage);

    void Deserialize(const std::string& state, ModelStorage& storage);
};

}  // namespace ops
}  // namespace pyis