// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "pyis/bindings/onnx/onnx_base_kernel.h"
#include "pyis/ops/tokenizer/bert_tokenizer.h"

namespace pyis {
namespace onnx {

using pyis::ops::BertTokenizer;

class KernelBertTokenizer : public BaseKernel {
  public:
    explicit KernelBertTokenizer(OrtApi api, const OrtKernelInfo* info);
    void Compute(OrtKernelContext* context);

  private:
    std::shared_ptr<BertTokenizer> tokenizer_;
};

class OrtOpBertTokenizer : public Ort::CustomOpBase<OrtOpBertTokenizer, KernelBertTokenizer> {
  public:
    static void* CreateKernel(OrtApi api, const OrtKernelInfo* info);
    static const char* GetName();
    static size_t GetInputTypeCount();
    static ONNXTensorElementDataType GetInputType(size_t index);
    static size_t GetOutputTypeCount();
    static ONNXTensorElementDataType GetOutputType(size_t index);
};

}  // namespace onnx
}  // namespace pyis
