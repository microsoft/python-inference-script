// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <torch/custom_class.h>
#include <torch/script.h>

#include "pyis/share/model_context.h"

namespace pyis {
namespace torchscript {
class ModelContextAdaptor : public torch::CustomClassHolder, public ModelContext {
  public:
    using ModelContext::ModelContext;

    void SetFilePrefix(const std::string& prefix) { ModelContext::SetFilePrefix(prefix); }

    static void Activate(const c10::intrusive_ptr<ModelContextAdaptor>& ctx) { ModelContext::Activate(ctx.get()); }

    static void Deactivate(const c10::intrusive_ptr<ModelContextAdaptor>& ctx) { ModelContext::Deactivate(ctx.get()); }
};

// bind model context
void init_model_context(::torch::Library& m) {
    m.class_<ModelContextAdaptor>("ModelContext")
        .def(::torch::init<std::string, std::string>(), "", {torch::arg("model_path"), torch::arg("data_archive") = ""})
        .def("set_file_prefix", &ModelContextAdaptor::SetFilePrefix, "", {torch::arg("prefix")})
        .def_static("activate", &ModelContextAdaptor::Activate)
        .def_static("deactivate", &ModelContextAdaptor::Deactivate);
}
}  // namespace torchscript
}  // namespace pyis