// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ort_cpu_session_impl.h"

#include <cassert> /* assert */

#include "pyis/share/exception.h"
#include "pyis/share/str_utils.h"

namespace pyis {
namespace ops {
namespace dnn {

Ort::Env OrtCPUSessionImpl::env(ORT_LOGGING_LEVEL_WARNING, "pyis_ort_cpu_session_impl");
const std::string OrtCPUSessionImpl::DELIMITER = ";;;";

OrtCPUSessionImpl::OrtCPUSessionImpl(std::string model_file, int64_t intra_op_thread_num, int64_t inter_op_thread_num)
    : model_file_(std::move(model_file)),
      intra_op_thread_num_(intra_op_thread_num),
      inter_op_thread_num_(inter_op_thread_num),
      memory_info_(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault)) {
    ;

    session_options_.SetIntraOpNumThreads(intra_op_thread_num_);
    session_options_.SetInterOpNumThreads(inter_op_thread_num_);
    session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

#ifdef _WIN32
    session_ = std::make_shared<Ort::Session>(env, str_to_wstr(model_file_).c_str(), session_options_);
#else
    session_ = std::make_shared<Ort::Session>(env, model_file_.c_str(), session_options_);
#endif
}

std::vector<std::shared_ptr<Tensor>> OrtCPUSessionImpl::Execute(const std::vector<std::string>& input_names,
                                                                const std::vector<std::shared_ptr<Tensor>>& inputs,
                                                                const std::vector<std::string>& output_names) {
    // one thread execute the inference at a time
    std::lock_guard<std::mutex> runtime_lock(runtime_mutex_);

    size_t input_num = input_names.size();
    std::vector<char*> input_names_chars_repr(input_num);
    std::transform(input_names.begin(), input_names.end(), input_names_chars_repr.begin(),
                   [](const std::string& name) { return const_cast<char*>(name.c_str()); });

    size_t output_num = output_names.size();
    std::vector<char*> output_names_chars_repr(output_num);
    std::transform(output_names.begin(), output_names.end(), output_names_chars_repr.begin(),
                   [](const std::string& name) { return const_cast<char*>(name.c_str()); });

    // convert tensor to ort values
    std::vector<Ort::Value> inputs_as_ort_values;
    inputs_as_ort_values.reserve(inputs.size());

    for (const auto& tensor : inputs) {
        if (tensor->GetType() == TensorType::T_FLOAT32) {
            inputs_as_ort_values.emplace_back(ConvertToOrtValue<float>(tensor));
        } else if (tensor->GetType() == TensorType::T_INT64) {
            inputs_as_ort_values.emplace_back(ConvertToOrtValue<int64_t>(tensor));
        } else {
            PYIS_THROW("Convert to ort value exception, unsupported data type");
        }
    }

    auto ort_outputs = session_->Run(Ort::RunOptions(), input_names_chars_repr.data(), inputs_as_ort_values.data(),
                                     input_num, output_names_chars_repr.data(), output_num);

    std::vector<std::shared_ptr<Tensor>> outputs;
    outputs.reserve(output_num);
    for (auto& ort_output : ort_outputs) {
        auto output_tensor = ConvertToTensor(ort_output);
        outputs.emplace_back(output_tensor);
    }

    return outputs;
}

Ort::MemoryInfo& OrtCPUSessionImpl::GetMemoryInfo() { return memory_info_; }

std::string OrtCPUSessionImpl::Serialize() {
    return this->model_file_ + DELIMITER + std::to_string(this->intra_op_thread_num_) + DELIMITER +
           std::to_string(this->inter_op_thread_num_);
}

std::shared_ptr<OrtCPUSessionImpl> OrtCPUSessionImpl::Deserilize(std::string& state) {
    std::vector<std::string> segments;
    auto begin = 0U;
    auto end = state.find(DELIMITER);
    while (end != std::string::npos) {
        segments.emplace_back(state.substr(begin, end));
        begin = end + DELIMITER.length();
        end = state.find(DELIMITER, begin);
    }
    segments.emplace_back(state.substr(begin, end));

    assert(segments.size() == 3);

    return std::make_shared<OrtCPUSessionImpl>(segments[0], std::stol(segments[1]), std::stol(segments[2]));
}

}  // namespace dnn
}  // namespace ops
}  // namespace pyis