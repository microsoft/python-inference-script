// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <onnxruntime_cxx_api.h>

#include <cstdint>
#include <vector>

namespace pyis {
namespace onnx {

class OrtTensorDimensions {
  public:
    OrtTensorDimensions() = default;
    explicit OrtTensorDimensions(std::vector<int64_t> dim);
    OrtTensorDimensions(Ort::CustomOpApi& ort, const OrtValue* value);

    int64_t size();
    size_t count();
    int64_t* data();
    int64_t operator[](size_t n);
    void push_back(int64_t& n);

  private:
    std::vector<int64_t> dim_;
};
}  // namespace onnx
}  // namespace pyis