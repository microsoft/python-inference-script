// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "pyis/ops/text/cedar_trie.h"
#include "pyis/ops/text/immutable_trie.h"

namespace pyis {
namespace ops {

class Trie : public CachedObject<Trie> {
  public:
    Trie();
    explicit Trie(const std::string& path);

    ~Trie();

    void Insert(const std::string& key, const uint32_t& value);
    void Erase(const std::string& key);
    Expected<uint32_t> Lookup(const std::string& key) const;
    bool Contains(const std::string& key) const;
    void Save(const std::string& path);
    void Save(std::shared_ptr<std::ostream> os);
    void Load(const std::string& path);
    void Load(std::shared_ptr<std::istream> is);
    std::vector<std::tuple<std::string, uint32_t>> Items();
    void Freeze();

    void Deserialize(const std::string& state, ModelStorage& storage);
    std::string Serialize(ModelStorage& storage);

  private:
    ImmutableTrie* immutable_trie_;
    CedarTrie* cedar_trie_;
};

}  // namespace ops
}  // namespace pyis