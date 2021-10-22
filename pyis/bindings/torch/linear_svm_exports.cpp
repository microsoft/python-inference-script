// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)

#include "pyis/ops/linear_svm/linear_svm.h"
#include "pyis/share/model_context.h"
#include "pyis/share/scope_guard.h"

namespace pyis {
namespace torchscript {

using pyis::ops::LinearSVM;

class LinearSVMAdaptor : public ::torch::CustomClassHolder {
  public:
    explicit LinearSVMAdaptor(const std::string& path) { obj_ = std::make_shared<LinearSVM>(path); }
    explicit LinearSVMAdaptor(std::shared_ptr<LinearSVM>& obj) { obj_ = obj; }

    std::vector<double> Predict(const std::vector<std::tuple<int64_t, double>>& features) {
        std::vector<std::tuple<int, double>> data;
        data.resize(features.size());
        for (size_t i = 0; i < features.size(); i++) {
            data[i] =
                std::make_tuple(static_cast<int>(static_cast<int>(std::get<0>(features[i]))), std::get<1>(features[i]));
        }

        auto scores = obj_->Predict(data);
        return scores;
    }

    static void Train(const std::string& libsvm_data_file, const std::string& model_file, int64_t solver_type,
                      double eps, double c, double p, double bias) {
        LinearSVM::Train(libsvm_data_file, model_file, solver_type, eps, c, p, bias);
    }

    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<LinearSVM> obj_;
};

void init_linear_svm(::torch::Library& m) {
    m.class_<LinearSVMAdaptor>("LinearSVM")
        .def(::torch::init<std::string>(), "", {torch::arg("model_file")})
        .def("predict", &LinearSVMAdaptor::Predict, "", {torch::arg("features")})
        .def_static("train", &LinearSVMAdaptor::Train)
        .def_pickle(
            [](const c10::intrusive_ptr<LinearSVMAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<LinearSVMAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<LinearSVM>(state);
                return c10::make_intrusive<LinearSVMAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis