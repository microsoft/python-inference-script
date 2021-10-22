// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <onnxruntime_cxx_api.h>

#include <algorithm>
#include <codecvt>
#include <cstdlib>
#include <locale>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "session_impl_base.h"
#include "tensor.h"

namespace pyis {
namespace ops {
namespace dnn {
// Notice a few things:
// - On windows, there is an onnxruntime.dll in C:\Windows\system32\.
//   According to the dll search path order here, this dll will be loaded
//   when you are run the extension in Python.
//   https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order#search-order-for-desktop-applications
//   To avoid this, load your specific onnxruntime.dll by providing the library
//   path torch.classes.load_library("/path/to/onnxruntime.dll")
// - Do not set macro ORT_API_MANUAL_INIT. The OrtSession class require the
//   OrtApi to be initialized before any object instantiation
class OrtCPUSessionImpl : public SessionImplBase {
  public:
    OrtCPUSessionImpl(std::string model_file, int64_t intra_op_thread_num, int64_t inter_op_thread_num);

    std::vector<std::shared_ptr<Tensor>> Execute(const std::vector<std::string>& input_names,
                                                 const std::vector<std::shared_ptr<Tensor>>& inputs,
                                                 const std::vector<std::string>& output_names) override;

    Ort::MemoryInfo& GetMemoryInfo();

    std::string Serialize();
    static std::shared_ptr<OrtCPUSessionImpl> Deserilize(std::string& state);

  private:
    static Ort::Env env;

    Ort::SessionOptions session_options_;
    Ort::AllocatorWithDefaultOptions allocator_;
    Ort::MemoryInfo memory_info_;
    std::shared_ptr<Ort::Session> session_;

    std::string model_file_;
    int64_t intra_op_thread_num_;
    int64_t inter_op_thread_num_;

    // [TODO-haoji] keep it until we figure out an elegant serialization solution
    static const std::string DELIMITER;

    std::mutex runtime_mutex_;

    template <typename T>
    Ort::Value ConvertToOrtValue(const std::shared_ptr<Tensor>& tensor) {
        auto ort_value =
            Ort::Value::CreateTensor<T>(memory_info_, static_cast<T*>(tensor->DataMap()), tensor->GetSize(),
                                        tensor->GetShape().data(), tensor->GetDimensions());
        return ort_value;
    }

    std::shared_ptr<Tensor> ConvertToTensor(const Ort::Value& ort_value) {
        auto tensor_size = ort_value.GetTensorTypeAndShapeInfo().GetElementCount();
        auto tensor_shape = ort_value.GetTensorTypeAndShapeInfo().GetShape();
        auto tensor_ndim = ort_value.GetTensorTypeAndShapeInfo().GetDimensionsCount();
        TensorType tensor_datatype;

        if (ort_value.GetTensorTypeAndShapeInfo().GetElementType() ==
            ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            tensor_datatype = TensorType::T_FLOAT32;
        } else if (ort_value.GetTensorTypeAndShapeInfo().GetElementType() ==
                   ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
            tensor_datatype = TensorType::T_INT64;
        } else {
            PYIS_THROW("Convert Tensor error : Unsupported data type");
        }

        auto ret = std::make_shared<Tensor>(tensor_datatype, tensor_shape, tensor_ndim, tensor_size,
                                            const_cast<void*>(ort_value.GetTensorData<void>()));
        return ret;
    }
};
}  // namespace dnn
}  // namespace ops
}  // namespace pyis