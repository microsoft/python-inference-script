// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)
#include "pyis/ops/example_op/word_dict.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace torchscript {

using pyis::ops::WordDict;

class WordDictAdaptor : public ::torch::CustomClassHolder {
  public:
    explicit WordDictAdaptor(const std::string& data_file) { obj_ = std::make_shared<WordDict>(data_file); }
    explicit WordDictAdaptor(std::shared_ptr<WordDict>& obj) { obj_ = obj; }
    std::vector<std::string> Translate(const std::vector<std::string>& tokens) { return obj_->Translate(tokens); }
    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<WordDict> obj_;
};

void init_word_dict(::torch::Library& m) {
    m.class_<WordDictAdaptor>("WordDict")
        .def(::torch::init<std::string>(), "", {torch::arg("data_file")})
        .def("translate", &WordDictAdaptor::Translate, "", {torch::arg("tokens")})

        .def_pickle(
            [](const c10::intrusive_ptr<WordDictAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<WordDictAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state);
                return c10::make_intrusive<WordDictAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis