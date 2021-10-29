// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <algorithm>
#include <cstdlib>

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)

#include "pyis/bindings/torch/tensor_convertor.h"
#include "pyis/ops/ort_session/ort_session.h"

namespace pyis {
namespace torchscript {

using pyis::ops::OrtSession;

class OrtSessionAdaptor : public ::torch::CustomClassHolder {
  public:
    OrtSessionAdaptor(const std::string& model_path, const std::vector<std::string>& input_names,
                      const std::vector<std::string>& output_names, int64_t inter_op_thread_num,
                      int64_t intra_op_thread_num, bool dynamic_batching, int64_t batch_size) {
        obj_ = std::make_shared<OrtSession>(model_path, input_names, output_names, inter_op_thread_num,
                                            intra_op_thread_num, dynamic_batching, batch_size);
    }

    explicit OrtSessionAdaptor(std::shared_ptr<OrtSession>& obj) { obj_ = obj; }

    std::vector<::torch::Tensor> run(const std::vector<::torch::Tensor>& inputs) {
        std::vector<std::shared_ptr<Ort::Value>> input_values;
        input_values.resize(inputs.size());
        for (size_t i = 0; i < inputs.size(); i++) {
            input_values[i] = (std::make_shared<Ort::Value>(ToOrtTensor(inputs[i])));
        }
        auto outputs = obj_->Run(input_values);
        std::vector<::torch::Tensor> ret;
        ret.reserve(outputs.size());
        for (const auto& output : outputs) {
            ret.emplace_back(ToTorchTensor(*output));
        }
        return ret;
    }

    static void InitializeOrt(const std::string& ort_dll_file) { OrtSession::InitializeOrt(ort_dll_file); }

    std::string Serialize(ModelStorage& m) { return obj_->Serialize(m); }

  private:
    std::shared_ptr<OrtSession> obj_;
};

void init_ort_session(::torch::Library& m) {
    m.class_<OrtSessionAdaptor>("OrtSession")
        .def(::torch::init<const std::string&, const std::vector<std::string>&, const std::vector<std::string>&,
                           int64_t, int64_t, bool, int64_t>(),
             "",
             {torch::arg("model_path"), torch::arg("input_names"), torch::arg("output_names"),
              torch::arg("inter_op_thread_num") = 1, torch::arg("intra_op_thread_num") = 0,
              torch::arg("dynamic_batching") = false, torch::arg("batch_size") = 1})
        .def("run", &OrtSessionAdaptor::run, "", {torch::arg("inputs")})
        .def_static("initialize_ort", &OrtSessionAdaptor::InitializeOrt, "")
        .def_pickle(
            [](const c10::intrusive_ptr<OrtSessionAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<OrtSessionAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<OrtSession>(state);
                return c10::make_intrusive<OrtSessionAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis