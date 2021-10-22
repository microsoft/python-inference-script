// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ort_session.h"

#include <algorithm> /* std::transform */
#include <cassert>   /* assert */

#include "pyis/share/file_system.h"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/str_utils.h"

namespace pyis {
namespace ops {

std::once_flag OrtSession::ort_initialization_flag;

OrtSession::OrtSession(std::string model_file, std::vector<std::string> input_names,
                       std::vector<std::string> output_names, int inter_op_thread_num, int intra_op_thread_num,
                       bool dynamic_batching, int batch_size, const std::string& ort_dll_file)
    : src_model_file_(std::move(model_file)),
      dynamic_batching_(dynamic_batching),
      batch_size_(batch_size),
      inter_op_thread_num_(inter_op_thread_num),
      intra_op_thread_num_(intra_op_thread_num),
      input_names_str_repr_(std::move(input_names)),
      output_names_str_repr_(std::move(output_names)) {
    src_model_storage_ = std::make_shared<ModelStorageLocal>();
    std::call_once(ort_initialization_flag, [&ort_dll_file]() { OrtGlobals::Initialize(ort_dll_file); });
    BuildSession();
}

std::vector<std::shared_ptr<Ort::Value>> OrtSession::Run(const std::vector<std::shared_ptr<Ort::Value>>& inputs) {
    if (dynamic_batching_) {
        // run batch manager
        std::vector<std::shared_ptr<Ort::Value>> outputs;
        outputs.reserve(output_names_.size());
        batch_mgr_->Execute(inputs, outputs);
        return outputs;
    }

    std::vector<Ort::Value> input_tensor_data;
    input_tensor_data.reserve(input_names_.size());
    std::vector<std::shared_ptr<Ort::Value>> outputs;
    outputs.reserve(output_names_.size());
    for (const auto& tensor_ptr : inputs) {
        input_tensor_data.emplace_back(ShallowCopyTensor(*tensor_ptr));
    }
    auto output_tensor_data = session_->Run(Ort::RunOptions(), input_names_.data(), input_tensor_data.data(),
                                            inputs.size(), output_names_.data(), output_names_.size());

    for (auto& output_tensor : output_tensor_data) {
        outputs.emplace_back(std::make_shared<Ort::Value>(std::move(output_tensor)));
    }
    return outputs;
}

std::string OrtSession::Serialize(ModelStorage& storage) {
    std::string onnx_model_file = storage.uniq_file("ort_session", ".model.onnx");
    ModelStorage::copy_file(*src_model_storage_, src_model_file_, storage, onnx_model_file);
    std::string config_file = storage.uniq_file("ort_session", ".config.json");

    JsonPersistHelper jph(1);
    jph.add_file("onnx_model_file", onnx_model_file);
    jph.add<int>("inter_thread_op_num", inter_op_thread_num_, true);
    jph.add<int>("intra_thread_op_num", intra_op_thread_num_, true);
    jph.add<bool>("dynamic_batching", dynamic_batching_);
    jph.add<int>("batch_size", batch_size_);
    jph.add("input_names", input_names_str_repr_);
    jph.add("output_names", output_names_str_repr_);
    std::string state = jph.serialize(config_file, storage);
    return state;
}

void OrtSession::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state, storage);
    int version = jph.version();
    if (1 == version) {
        src_model_file_ = jph.get_file("onnx_model_file");
        src_model_storage_ = storage.clone();
        dynamic_batching_ = jph.get<bool>("dynamic_batching");
        batch_size_ = jph.get<int>("batch_size");
        inter_op_thread_num_ = jph.get<int>("inter_thread_op_num");
        intra_op_thread_num_ = jph.get<int>("intra_thread_op_num");
        input_names_str_repr_ = jph.get<std::vector<std::string>>("input_names");
        output_names_str_repr_ = jph.get<std::vector<std::string>>("output_names");
        BuildSession();
    } else {
        PYIS_THROW(fmt_str("OrtSession v{} is incompatible with the runtime", version).c_str());
    }
}

void OrtSession::BuildSession() {
    // initialize Session options
    session_options_ = std::make_shared<Ort::SessionOptions>();

    session_options_->SetIntraOpNumThreads(intra_op_thread_num_);
    session_options_->SetInterOpNumThreads(inter_op_thread_num_);

    // convert input_names and output_names
    input_names_.clear();
    output_names_.clear();
    input_names_.resize(input_names_str_repr_.size());
    output_names_.resize(output_names_str_repr_.size());
    std::transform(input_names_str_repr_.begin(), input_names_str_repr_.end(), input_names_.begin(),
                   [](std::string& name) { return name.c_str(); });

    std::transform(output_names_str_repr_.begin(), output_names_str_repr_.end(), output_names_.begin(),
                   [](std::string& name) { return name.c_str(); });

    // load model from memory buffer. https://github.com/microsoft/onnxruntime/issues/6475
    auto model_stream = src_model_storage_->open_istream(src_model_file_);
    std::vector<char> model_buffer((std::istreambuf_iterator<char>(*model_stream)), std::istreambuf_iterator<char>());
    session_.reset(new Ort::Session(*OrtGlobals::Env, model_buffer.data(), model_buffer.size(), *session_options_));

    // use dynamic batching or not
    if (dynamic_batching_) {
        this->batch_mgr_ = std::make_unique<DynamicBatchManager>(batch_size_, session_, input_names_, output_names_);
    }
}

}  // namespace ops
}  // namespace pyis