// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)

#include "pyis/ops/linear_chain_crf/linear_chain_crf.h"
#include "pyis/share/model_context.h"
#include "pyis/share/scope_guard.h"

namespace pyis {
namespace torchscript {

using pyis::ops::LinearChainCRF;

class LinearChainCRFAdaptor : public ::torch::CustomClassHolder {
  public:
    explicit LinearChainCRFAdaptor(const std::string& path) { obj_ = std::make_shared<LinearChainCRF>(path); }
    explicit LinearChainCRFAdaptor(std::shared_ptr<LinearChainCRF>& obj) { obj_ = obj; }

    std::vector<int64_t> Predict(int64_t len, const std::vector<std::tuple<int64_t, int64_t, double>>& features) {
        std::vector<std::tuple<uint16_t, uint32_t, double>> inputs(features.size());
        for (auto i = 0; i < features.size(); i++) {
            inputs[i] = std::tuple<uint16_t, uint32_t, double>(static_cast<uint16_t>(std::get<0>(features[i])),
                                                               static_cast<uint32_t>(std::get<1>(features[i])),
                                                               std::get<2>(features[i]));
        }
        auto labels = obj_->Predict(len, inputs);
        std::vector<int64_t> res(labels.size());
        for (auto i = 0; i < labels.size(); i++) {
            res[i] = labels[i];
        }
        return res;
    }

    static void Train(const std::string& data_file, const std::string& model_file, const std::string& alg,
                      int64_t max_iter) {
        LinearChainCRF::Train(data_file, model_file, alg, max_iter);
    }

    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<LinearChainCRF> obj_;
};

void init_linear_chain_crf(::torch::Library& m) {
    m.class_<LinearChainCRFAdaptor>("LinearChainCRF")
        .def(::torch::init<std::string>(), "", {torch::arg("model_file")})
        .def("predict", &LinearChainCRFAdaptor::Predict, "", {torch::arg("len"), torch::arg("features")})
        .def_static("train", &LinearChainCRFAdaptor::Train)
        .def_pickle(
            [](const c10::intrusive_ptr<LinearChainCRFAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<LinearChainCRFAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<LinearChainCRF>(state);
                return c10::make_intrusive<LinearChainCRFAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis