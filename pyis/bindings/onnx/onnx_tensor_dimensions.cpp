// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "onnx_tensor_dimensions.h"

#include <utility>

namespace pyis {
namespace onnx {

OrtTensorDimensions::OrtTensorDimensions(std::vector<int64_t> dim) : dim_(std::move(dim)) {}

OrtTensorDimensions::OrtTensorDimensions(Ort::CustomOpApi& ort, const OrtValue* value) {
    OrtTensorTypeAndShapeInfo* info = ort.GetTensorTypeAndShape(value);
    dim_ = ort.GetTensorShape(info);
    ort.ReleaseTensorTypeAndShapeInfo(info);
}

int64_t OrtTensorDimensions::size() {
    if (dim_.empty()) {
        return 0;
    }

    int64_t result = 1;
    for (auto i : dim_) {
        result *= i;
    }
    return result;
}

size_t OrtTensorDimensions::count() { return (dim_.size()); }

int64_t* OrtTensorDimensions::data() { return dim_.data(); }

int64_t OrtTensorDimensions::operator[](size_t n) { return dim_[n]; }

void OrtTensorDimensions::push_back(int64_t& n) { dim_.push_back(n); }
}  // namespace onnx
}  // namespace pyis