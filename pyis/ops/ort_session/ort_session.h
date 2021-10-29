// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <codecvt>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#include "ort_dym_batch_mgr.h"
#include "ort_globals.h"
#include "ort_tensor_utils.h"
#include "pyis/share/exception.h"
#include "pyis/share/model_context.h"
#include "pyis/share/model_storage_local.h"

namespace pyis {
namespace ops {

class OrtSession : public CachedObject<OrtSession> {
  public:
    OrtSession(std::string model_file, std::vector<std::string> input_names, std::vector<std::string> output_names,
               int inter_op_thread_num, int intra_op_thread_num, bool dynamic_batching = false, int batch_size = 1);

    // For deserialization
    OrtSession() = default;

    std::string Serialize(ModelStorage& storage);
    void Deserialize(const std::string& state, ModelStorage& storage);

    std::vector<std::shared_ptr<Ort::Value>> Run(const std::vector<std::shared_ptr<Ort::Value>>& inputs);

    /// <summary>
    /// Initialize onnxruntime dynamically by loading ort dll
    /// </summary>
    /// <param name="ort_dll_file">ort dll file name</param>
    static void InitializeOrt(const std::string& ort_dll_file);

  private:
    void BuildSession();

    static std::mutex ort_initialization_mutex;
    static bool ort_initialized;
    std::shared_ptr<Ort::SessionOptions> session_options_;
    std::shared_ptr<Ort::Session> session_;

    int inter_op_thread_num_;
    int intra_op_thread_num_;
    bool dynamic_batching_;
    int batch_size_;

    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
    std::vector<std::string> input_names_str_repr_;
    std::vector<std::string> output_names_str_repr_;

    std::unique_ptr<DynamicBatchManager> batch_mgr_;

    // keep a pointer to file and storage from which the onnx model is loaded from
    std::shared_ptr<ModelStorage> src_model_storage_;
    std::string src_model_file_;
};

}  // namespace ops
}  // namespace pyis