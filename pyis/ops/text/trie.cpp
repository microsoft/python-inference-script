// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "pyis/ops/text/trie.h"

namespace pyis {
namespace ops {

Trie::Trie() {
    cedar_trie_ = new CedarTrie();
    immutable_trie_ = nullptr;
}

Trie::Trie(const std::string& path) {
    cedar_trie_ = nullptr;
    immutable_trie_ = new ImmutableTrie(path);
}

Trie::~Trie() {
    delete cedar_trie_;
    delete immutable_trie_;
}

void Trie::Insert(const std::string& key, const uint32_t& value) {
    if (cedar_trie_ != nullptr) {
        cedar_trie_->Insert(key, static_cast<int>(value));
        return;
    }
    PYIS_THROW("The trie is frozen and thus read-only.");
}

void Trie::Erase(const std::string& key) {
    if (cedar_trie_ != nullptr) {
        cedar_trie_->Erase(key);
        return;
    }
    PYIS_THROW("The trie is frozen and thus read-only.");
}

bool Trie::Contains(const std::string& key) const { return Lookup(key).has_value(); }

Expected<uint32_t> Trie::Lookup(const std::string& key) const {
    if (cedar_trie_ != nullptr) {
        auto result = cedar_trie_->Lookup(key);
        if (result.has_error()) {
            return Expected<uint32_t>(result.error());
        }
        return Expected<uint32_t>(static_cast<uint32_t>(result.value()));
    }
    return immutable_trie_->Match(key);
}

void Trie::Save(const std::string& path) {
    if (cedar_trie_ != nullptr) {
        std::vector<std::tuple<std::string, uint32_t>> data;
        auto content = cedar_trie_->Items();
        data.resize(content.size());
        for (size_t i = 0; i < data.size(); i++) {
            data[i] = std::make_tuple(std::get<0>(content[i]), static_cast<uint32_t>(std::get<1>(content[i])));
        }
        ImmutableTrieConstructor constructor(data);
        constructor.WriteToFile(path);
        return;
    }
    immutable_trie_->Save(path);
}

void Trie::Save(std::shared_ptr<std::ostream> os) {
    if (cedar_trie_ != nullptr) {
        std::vector<std::tuple<std::string, uint32_t>> data;
        auto content = cedar_trie_->Items();
        data.resize(content.size());
        for (size_t i = 0; i < data.size(); i++) {
            data[i] = std::make_tuple(std::get<0>(content[i]), static_cast<uint32_t>(std::get<1>(content[i])));
        }
        ImmutableTrieConstructor constructor(data);
        constructor.WriteToFile(os);
        return;
    }
    BinaryWriter writer(std::move(os));
    immutable_trie_->Save(writer);
}

void Trie::Load(const std::string& path) {
    delete cedar_trie_;
    cedar_trie_ = nullptr;
    delete immutable_trie_;
    immutable_trie_ = new ImmutableTrie(path);
}

void Trie::Load(std::shared_ptr<std::istream> is) {
    delete cedar_trie_;
    cedar_trie_ = nullptr;
    delete immutable_trie_;
    BinaryReader reader(std::move(is));
    immutable_trie_ = new ImmutableTrie(reader);
}

std::vector<std::tuple<std::string, uint32_t>> Trie::Items() {
    if (cedar_trie_ != nullptr) {
        std::vector<std::tuple<std::string, uint32_t>> data;
        auto content = cedar_trie_->Items();
        data.resize(content.size());
        for (size_t i = 0; i < data.size(); i++) {
            data[i] = std::make_tuple(std::get<0>(content[i]), static_cast<uint32_t>(std::get<1>(content[i])));
        }
        return data;
    }
    return immutable_trie_->Items();
}

void Trie::Freeze() {
    if (cedar_trie_ == nullptr) {
        return;
    }
    auto data = Items();
    delete cedar_trie_;
    cedar_trie_ = nullptr;
    immutable_trie_ = new ImmutableTrie(data);
}

void Trie::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state, storage);
    int version = jph.version();
    if (1 == version) {
        std::string data_file = jph.get_file("data");
        Load(storage.open_istream(data_file));
        return;
    }

    PYIS_THROW("ImmutableTrie v%d is incompatible with the runtime", version);
}

std::string Trie::Serialize(ModelStorage& storage) {
    std::string data_file = storage.uniq_file("trie", ".data.bin");

    Save(storage.open_ostream(data_file));

    JsonPersistHelper jph(1);
    jph.add_file("data", data_file);

    std::string config_file = storage.uniq_file("trie", ".config.json");
    std::string state = jph.serialize(config_file, storage);
    return state;
}

}  // namespace ops
}  // namespace pyis