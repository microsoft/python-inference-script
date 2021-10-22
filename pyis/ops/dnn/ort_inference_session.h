// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <memory>
#include <vector>

#include "inference_session.h"
#include "ort_cpu_session_impl.h"
#include "pyis/share/exception.h"
#include "session_impl_base.h"

namespace pyis {
namespace ops {
namespace dnn {
class OrtSession : public InferenceSession {
  public:
    explicit OrtSession(std::string& model_path, std::string& device, size_t inter_op_thread_num,
                        size_t intra_op_thread_num, bool enable_batch, int max_batch_size)
        : InferenceSession(model_path, enable_batch, max_batch_size) {
        if (device == "CPU") {
            session_impl_ = std::static_pointer_cast<SessionImplBase>(
                std::make_shared<OrtCPUSessionImpl>(model_path, intra_op_thread_num, inter_op_thread_num));
            this->dynamic_batch_manager_->SetSessionImpl(session_impl_);
        } else if (device == "GPU") {
            PYIS_THROW("Ort Session Exception : only support CPU for now");
        } else {
            PYIS_THROW("Ort Session Exception : unsupported device");
        }
    }
};
}  // namespace dnn
}  // namespace ops
}  // namespace pyis
