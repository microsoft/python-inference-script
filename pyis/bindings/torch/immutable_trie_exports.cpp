// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)
#include "pyis/ops/text/immutable_trie.h"
#include "pyis/share/expected.hpp"

namespace pyis {
namespace torchscript {
using pyis::ops::ImmutableTrie;
class ImmutableTrieAdaptor : public ::torch::CustomClassHolder {
  public:
    explicit ImmutableTrieAdaptor() { obj_ = std::make_shared<ImmutableTrie>(); }
    explicit ImmutableTrieAdaptor(std::shared_ptr<ImmutableTrie>& obj) { obj_ = obj; }

    std::vector<std::tuple<std::string, int64_t>> Items() {
        std::vector<std::tuple<std::string, int64_t>> result;
        auto data = obj_->Items();
        result.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            result[i] = std::make_tuple(std::get<0>(data[i]), static_cast<int64_t>(std::get<1>(data[i])));
        }
        return result;
    }

    int64_t Lookup(const std::string& key) {
        auto result = obj_->Match(key);
        if (result.has_error()) {
            throw result.error();
        }
        return result.value();
    }

    bool Contains(const std::string& key) { return obj_->Contains(key); }

    static void Compile(const std::vector<std::tuple<std::string, int64_t>>& data, const std::string& path) {
        std::vector<std::tuple<std::string, uint32_t>> construct_data;
        construct_data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            construct_data[i] = std::make_tuple(std::get<0>(data[i]), static_cast<uint32_t>(std::get<1>(data[i])));
        }
        auto result = ImmutableTrie::Compile(construct_data, path);
        if (result.has_error()) {
            throw result.error();
        }
    }

    void LoadItems(const std::vector<std::tuple<std::string, int64_t>>& data) {
        std::vector<std::tuple<std::string, uint32_t>> construct_data;
        construct_data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            construct_data[i] = std::make_tuple(std::get<0>(data[i]), static_cast<uint32_t>(std::get<1>(data[i])));
        }
        obj_->LoadItems(construct_data);
    }

    void Load(const std::string& path) {
        auto result = obj_->Load(path);
        if (result.has_error()) {
            throw result.error();
        }
    }

  private:
    std::shared_ptr<ImmutableTrie> obj_;
};

void init_immutable_trie(::torch::Library& m) {
    m.class_<ImmutableTrieAdaptor>("ImmutableTrie")
        .def(::torch::init<>())
        .def("load_items", &ImmutableTrieAdaptor::LoadItems)
        .def("load", &ImmutableTrieAdaptor::Load)
        .def("items", &ImmutableTrieAdaptor::Items)
        .def("lookup", &ImmutableTrieAdaptor::Lookup, "", {torch::arg("key")})
        .def("contains", &ImmutableTrieAdaptor::Contains, "", {torch::arg("key")})
        .def_static("compile", &ImmutableTrieAdaptor::Compile, "");
}

}  // namespace torchscript
}  // namespace pyis