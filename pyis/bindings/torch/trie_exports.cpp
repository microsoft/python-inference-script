// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)
#include "pyis/ops/text/trie.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace torchscript {

using pyis::ops::Trie;
class TrieAdaptor : public ::torch::CustomClassHolder {
  public:
    TrieAdaptor() { obj_ = std::make_shared<Trie>(); }
    explicit TrieAdaptor(std::shared_ptr<Trie>& obj) { obj_ = obj; }
    void Insert(const std::string& key, int64_t value) { obj_->Insert(key, value); }
    void Erase(const std::string& key) { obj_->Erase(key); }
    int64_t Lookup(const std::string& key) {
        auto result = obj_->Lookup(key);
        if (result.has_error()) {
            throw result.error();
        }
        return result.value();
    }
    bool Contains(const std::string& key) { return obj_->Contains(key); }
    void Freeze() { obj_->Freeze(); }
    void Save(const std::string& path) { obj_->Save(path); }
    void Load(const std::string& path) { obj_->Load(path); }
    std::vector<std::tuple<std::string, int64_t>> Items() {
        std::vector<std::tuple<std::string, int64_t>> result;
        for (const auto& ite : obj_->Items()) {
            result.emplace_back(std::make_tuple(std::get<0>(ite), static_cast<int64_t>(std::get<1>(ite))));
        }
        return result;
    }
    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<Trie> obj_;
};

void init_trie(::torch::Library& m) {
    m.class_<TrieAdaptor>("Trie")
        .def(::torch::init<>())
        .def("lookup", &TrieAdaptor::Lookup, "", {torch::arg("key")})
        .def("items", &TrieAdaptor::Items)
        .def("erase", &TrieAdaptor::Erase, "", {torch::arg("key")})
        .def("insert", &TrieAdaptor::Insert, "", {torch::arg("key"), torch::arg("value") = 0})
        .def("contains", &TrieAdaptor::Contains)
        .def("freeze", &TrieAdaptor::Freeze)
        .def("load", &TrieAdaptor::Load, "", {torch::arg("path")})
        .def("save", &TrieAdaptor::Save, "", {torch::arg("path")})
        .def_pickle(
            [](const c10::intrusive_ptr<TrieAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<TrieAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<Trie>(state);
                return c10::make_intrusive<TrieAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis