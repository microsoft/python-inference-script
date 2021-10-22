// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)
#include "pyis/ops/text/cedar_trie.h"
#include "pyis/share/expected.hpp"
#include "pyis/share/model_context.h"

namespace pyis {
namespace torchscript {

using pyis::ops::CedarTrie;

class CedarTrieAdaptor : public ::torch::CustomClassHolder {
  public:
    CedarTrieAdaptor() { obj_ = std::make_shared<CedarTrie>(); }
    explicit CedarTrieAdaptor(std::shared_ptr<CedarTrie>& obj) { obj_ = obj; }

    int64_t Lookup(const std::string& key) {
        Expected<int> result = obj_->Lookup(key);
        if (result.has_error()) {
            throw result.error();
        }
        return result.value();
    }

    bool Contains(const std::string& key) { return obj_->Contains(key); }

    std::vector<std::tuple<std::string, int64_t>> Items() {
        std::vector<std::tuple<std::string, int64_t>> result;
        for (const auto& ite : obj_->Items()) {
            result.emplace_back(std::make_tuple(std::get<0>(ite), static_cast<int64_t>(std::get<1>(ite))));
        }
        return result;
    }

    std::vector<std::tuple<std::string, int64_t>> Predict(const std::string& prefix) {
        std::vector<std::tuple<std::string, int64_t>> result;
        for (const auto& ite : obj_->Predict(prefix)) {
            result.emplace_back(std::make_tuple(std::get<0>(ite), static_cast<int64_t>(std::get<1>(ite))));
        }
        return result;
    }

    std::vector<std::tuple<std::string, int64_t>> Prefix(const std::string& query) {
        std::vector<std::tuple<std::string, int64_t>> result;
        for (const auto& ite : obj_->Prefix(query)) {
            result.emplace_back(std::make_tuple(std::get<0>(ite), static_cast<int64_t>(std::get<1>(ite))));
        }
        return result;
    }

    std::tuple<std::string, int64_t> LongestPrefix(const std::string& query) {
        auto query_result = obj_->LongestPrefix(query);
        if (query_result.has_error()) {
            throw query_result.error();
        }
        return std::make_tuple(std::get<0>(query_result.value()),
                               static_cast<int64_t>(std::get<1>(query_result.value())));
    }

    int64_t Erase(const std::string& key) { return obj_->Erase(key); }

    int64_t Insert(const std::string& key, int64_t value) { return obj_->Insert(key, value); }

    int64_t NumKeys() { return obj_->NumKeys(); }

    void Load(const std::string& path) {
        auto result = obj_->Open(path);
        if (result.has_error()) {
            throw result.error();
        }
    }

    void Save(const std::string& path) {
        auto result = obj_->Save(path);
        if (result.has_error()) {
            throw result.error();
        }
    }

    void Reset() { obj_->Reset(); }

    int64_t Build(const std::vector<std::tuple<std::string, int64_t>>& data) {
        std::vector<std::tuple<std::string, int>> args;
        args.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            args[i] = std::make_tuple(std::get<0>(data[i]), static_cast<int>(std::get<1>(data[i])));
        }
        return obj_->Build(args);
    }

    int64_t BuildFromFile(const std::string& path) {
        auto result = obj_->BuildFromFile(path);
        if (result.has_error()) {
            throw result.error();
        }
        return result.value();
    }

    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<CedarTrie> obj_;
};

void init_cedar_trie(::torch::Library& m) {
    m.class_<CedarTrieAdaptor>("CedarTrie")
        .def(::torch::init<>())
        .def("lookup", &CedarTrieAdaptor::Lookup, "", {torch::arg("key")})
        .def("contains", &CedarTrieAdaptor::Contains, "", {torch::arg("key")})
        .def("items", &CedarTrieAdaptor::Items)
        .def("predict", &CedarTrieAdaptor::Predict, "", {torch::arg("prefix")})
        .def("prefix", &CedarTrieAdaptor::Prefix, "", {torch::arg("query")})
        .def("longest_prefix", &CedarTrieAdaptor::LongestPrefix, "", {torch::arg("query")})
        .def("erase", &CedarTrieAdaptor::Erase, "", {torch::arg("key")})
        .def("insert", &CedarTrieAdaptor::Insert, "", {torch::arg("key"), torch::arg("value") = 0})
        .def("numkeys", &CedarTrieAdaptor::NumKeys)
        .def("load", &CedarTrieAdaptor::Load, "", {torch::arg("path")})
        .def("save", &CedarTrieAdaptor::Save, "", {torch::arg("path")})
        .def("reset", &CedarTrieAdaptor::Reset)
        .def("build", &CedarTrieAdaptor::Build, "", {torch::arg("data")})
        .def("build_from_file", &CedarTrieAdaptor::BuildFromFile)
        .def_pickle(
            [](const c10::intrusive_ptr<CedarTrieAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<CedarTrieAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<CedarTrie>(state);
                return c10::make_intrusive<CedarTrieAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis