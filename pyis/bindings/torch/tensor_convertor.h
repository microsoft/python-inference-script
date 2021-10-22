// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <onnxruntime_cxx_api.h>
#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)

#include <numeric>

#include "pyis/ops/ort_session/ort_globals.h"
#include "pyis/share/exception.h"

namespace pyis {
namespace torchscript {

template <typename T>
Ort::Value CreateOrtTensor(torch::Tensor tensor) {
    auto* data_ptr = tensor.data_ptr<T>();
    auto dim = tensor.dim();
    auto sizes = tensor.sizes();
    const auto* shape = sizes.data();
    auto size = tensor.numel();
    auto ort_value = Ort::Value::CreateTensor<T>(*OrtGlobals::Allocator, shape, dim);
    void* data_map = static_cast<void*>(ort_value.template GetTensorMutableData<T>());
    memcpy(static_cast<char*>(data_map), data_ptr, sizeof(T) * size);
    return ort_value;
}

inline torch::Tensor ToTorchTensor(Ort::Value& onnx_tensor) {
    auto data_type = onnx_tensor.GetTensorTypeAndShapeInfo().GetElementType();
    auto shape = onnx_tensor.GetTensorTypeAndShapeInfo().GetShape();
    if (data_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
        auto options = torch::TensorOptions().dtype(torch::kInt64);
        auto tensor =
            torch::from_blob(onnx_tensor.GetTensorMutableData<int64_t>(), c10::IntArrayRef(shape), options).clone();
        return tensor;
    }
    if (data_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
        // [TODO] float is default to float 32 or float 64?
        auto options = torch::TensorOptions().dtype(torch::kFloat32).device(torch::kCPU);
        auto tensor =
            torch::from_blob(onnx_tensor.GetTensorMutableData<float>(), c10::IntArrayRef(shape), options).clone();
        return tensor;
    }
    if (data_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL) {
        auto options = torch::TensorOptions().dtype(torch::kBool).device(torch::kCPU);
        auto tensor =
            torch::from_blob(onnx_tensor.GetTensorMutableData<bool>(), c10::IntArrayRef(shape), options).clone();
        return tensor;
    }
    return torch::Tensor();
}

inline Ort::Value ToOrtTensor(const torch::Tensor& torch_tensor) {
    if (torch_tensor.dtype() == torch::kFloat) {
        return CreateOrtTensor<float>(torch_tensor);
    }
    if (torch_tensor.dtype() == torch::kInt64) {
        return CreateOrtTensor<int64_t>(torch_tensor);
    }
    if (torch_tensor.dtype() == torch::kBool) {
        return CreateOrtTensor<bool>(torch_tensor);
    }
    PYIS_THROW("PyIS currently only supports tensor conversion for data types : float, int64, bool");
}

}  // namespace torchscript
}  // namespace pyis
