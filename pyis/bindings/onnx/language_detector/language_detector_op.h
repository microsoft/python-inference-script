// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "pyis/bindings/onnx/onnx_base_kernel.h"
#include "pyis/ops/example_ort/language_detector.h"

namespace pyis {
namespace onnx {

using pyis::ops::NaiveLanguageDetector;

class KernelNaiveLanguageDetector : public BaseKernel {
  public:
    explicit KernelNaiveLanguageDetector(OrtApi api);
    void Compute(OrtKernelContext* context);

  private:
    NaiveLanguageDetector detector_;
};

class OrtOpNaiveLanguageDetector : public Ort::CustomOpBase<OrtOpNaiveLanguageDetector, KernelNaiveLanguageDetector> {
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
