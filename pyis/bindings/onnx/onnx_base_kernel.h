// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>

namespace pyis {
namespace onnx {

class BaseKernel {
  public:
    explicit BaseKernel(OrtApi api) : api_(api), info_(nullptr), ort_(api_) {}

    BaseKernel(OrtApi api, const OrtKernelInfo* info) : api_(api), info_(info), ort_(api_) {}

    void GetTensorAsString(OrtKernelContext* context, const OrtValue* value, std::vector<std::string>& output);

    bool HasAttribute(const char* name) const;
    bool TryGetAttribute(const char* name, std::string& value);
    bool TryGetAttribute(const char* name, int64_t& value);
    bool TryGetAttribute(const char* name, float& value);

  protected:
    OrtErrorCode GetErrorCodeAndRelease(OrtStatusPtr status) const;
    OrtApi api_;  // keep a copy of the struct, whose ref is used in the ort_
    Ort::CustomOpApi ort_;
    const OrtKernelInfo* info_;
};
}  // namespace onnx
}  // namespace pyis