// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ort_tensor_utils.h"

namespace pyis {
namespace ops {

bool CheckTensorDimensionMatch(std::vector<int64_t>& shape_src, std::vector<int64_t>& shape_tgt) {
    if (shape_src.size() != shape_tgt.size()) return false;
    for (int i = 1; i < shape_src.size(); i++) {
        if (shape_src[i] != shape_tgt[i]) return false;
    }
    return true;
}

template <typename T>
std::shared_ptr<Ort::Value> CreateConcatOrtTensor(const std::vector<std::shared_ptr<Ort::Value>>& tensors,
                                                  std::vector<int64_t>& shape, size_t dim) {
    auto& allocator = OrtGlobals::Allocator;
    size_t offset = 0;
    auto concat_tensor = std::make_shared<Ort::Value>(Ort::Value::CreateTensor<T>(*allocator, shape.data(), dim));
    void* data_map = static_cast<void*>(concat_tensor->template GetTensorMutableData<T>());
    for (const auto& tensor_ptr : tensors) {
        size_t tensor_numel = tensor_ptr->GetTensorTypeAndShapeInfo().GetElementCount();
        memcpy(static_cast<char*>(data_map) + offset, tensor_ptr->template GetTensorData<T>(),
               tensor_numel * sizeof(T));
        offset += tensor_numel * sizeof(T);
    }
    return concat_tensor;
}

template <typename T>
std::shared_ptr<Ort::Value> CreateConcatOrtTensor(const std::shared_ptr<Ort::Value>& src_tensor,
                                                  const std::shared_ptr<Ort::Value>& tgt_tensor,
                                                  const std::vector<int64_t>& shape, size_t dim) {
    auto& allocator = OrtGlobals::Allocator;
    size_t offset = 0;
    auto concat_tensor = std::make_shared<Ort::Value>(Ort::Value::CreateTensor<T>(*allocator, shape.data(), dim));
    void* data_map = static_cast<void*>(concat_tensor->template GetTensorMutableData<T>());

    auto src_numel = src_tensor->GetTensorTypeAndShapeInfo().GetElementCount();
    auto tgt_numel = tgt_tensor->GetTensorTypeAndShapeInfo().GetElementCount();
    memcpy(data_map, src_tensor->GetTensorData<T>(), src_numel * sizeof(T));
    memcpy(static_cast<char*>(data_map) + src_numel * sizeof(T), tgt_tensor->GetTensorData<T>(), tgt_numel * sizeof(T));

    return concat_tensor;
}

template <typename T>
std::shared_ptr<Ort::Value> CreateSliceOrtTensor(void* data_ptr, size_t numel, std::vector<int64_t>& shape,
                                                 size_t dim) {
    auto& allocator = OrtGlobals::Allocator;
    auto tensor_ptr = std::make_shared<Ort::Value>(Ort::Value::CreateTensor<T>(*allocator, shape.data(), dim));
    T* data_map = tensor_ptr->template GetTensorMutableData<T>();
    memcpy(static_cast<void*>(data_map), data_ptr, numel * sizeof(T));

    return tensor_ptr;
}

size_t GetTensorElementBytes(Ort::Value& tensor) {
    switch (tensor.GetTensorTypeAndShapeInfo().GetElementType()) {
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            return sizeof(float);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
            return sizeof(double);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            return sizeof(int);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            return sizeof(int64_t);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
            return sizeof(bool);
        default:
            PYIS_THROW("GetTensorDatamap : data type not supported");
    }
}

void* GetTensorDatamap(Ort::Value& tensor) {
    switch (tensor.GetTensorTypeAndShapeInfo().GetElementType()) {
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            return static_cast<void*>(tensor.GetTensorMutableData<float>());
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
            return static_cast<void*>(tensor.GetTensorMutableData<double>());
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            return static_cast<void*>(tensor.GetTensorMutableData<int>());
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            return static_cast<void*>(tensor.GetTensorMutableData<int64_t>());
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
            return static_cast<void*>(tensor.GetTensorMutableData<bool>());
        default:
            PYIS_THROW("GetTensorDatamap : data type not supported");
    }
}

std::shared_ptr<Ort::Value> ConcatTensors(std::vector<std::shared_ptr<Ort::Value>> tensors) {
    if (tensors.empty()) PYIS_THROW("ConcatTensors : empty tensor");
    if (tensors.size() == 1) return tensors[0];

    size_t base_dim = tensors[0]->GetTensorTypeAndShapeInfo().GetShape().size();
    if (base_dim < 1) PYIS_THROW("ConcatTensors : Invalid tensor dimension");
    std::vector<int64_t> base_shape = tensors[0]->GetTensorTypeAndShapeInfo().GetShape();
    size_t new_first_dim = 0;
    auto base_elem_type = tensors[0]->GetTensorTypeAndShapeInfo().GetElementType();

    for (auto& tensor : tensors) {
        auto tensor_shape = tensor->GetTensorTypeAndShapeInfo().GetShape();
        auto tensor_dim = tensor->GetTensorTypeAndShapeInfo().GetShape().size();
        auto tensor_elem_type = tensor->GetTensorTypeAndShapeInfo().GetElementType();
        if (base_dim != tensor_dim || !CheckTensorDimensionMatch(tensor_shape, base_shape) ||
            tensor_elem_type != base_elem_type)
            PYIS_THROW("ConcatTensors : dimension or shape or type mismatch");
        new_first_dim += tensor_shape[0];
    }

    base_shape[0] = new_first_dim;
    size_t base_numel =
        static_cast<size_t>(std::accumulate(base_shape.begin(), base_shape.end(), 1, std::multiplies<>()));

    auto& allocator = OrtGlobals::Allocator;

    switch (base_elem_type) {
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            return CreateConcatOrtTensor<float>(tensors, base_shape, base_dim);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
            return CreateConcatOrtTensor<double>(tensors, base_shape, base_dim);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            return CreateConcatOrtTensor<int>(tensors, base_shape, base_dim);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            return CreateConcatOrtTensor<int64_t>(tensors, base_shape, base_dim);
        case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
            return CreateConcatOrtTensor<bool>(tensors, base_shape, base_dim);
        default:
            PYIS_THROW("ConcatTensor : data type not supported");
    }
}

std::shared_ptr<Ort::Value> ConcatTensors(const std::shared_ptr<Ort::Value>& src_tensor,
                                          const std::shared_ptr<Ort::Value>& tgt_tensor) {
    if (src_tensor->GetTensorTypeAndShapeInfo().GetShape().size() !=
        tgt_tensor->GetTensorTypeAndShapeInfo().GetShape().size()) {
        PYIS_THROW("ConcatTensors : tensors dimension mismatch");
    }

    auto source_shape = src_tensor->GetTensorTypeAndShapeInfo().GetShape();
    auto target_shape = tgt_tensor->GetTensorTypeAndShapeInfo().GetShape();
    if (!CheckTensorDimensionMatch(source_shape, target_shape)) {
        PYIS_THROW("ConcatTensors : tensors shape mismatch");
    }

    auto& allocator = OrtGlobals::Allocator;

    auto base_elem_type = src_tensor->GetTensorTypeAndShapeInfo().GetElementType();
    auto base_dim = src_tensor->GetTensorTypeAndShapeInfo().GetDimensionsCount();
    std::vector<int64_t> base_shape = src_tensor->GetTensorTypeAndShapeInfo().GetShape();
    base_shape[0] =
        src_tensor->GetTensorTypeAndShapeInfo().GetShape()[0] + tgt_tensor->GetTensorTypeAndShapeInfo().GetShape()[0];
    auto base_numel = src_tensor->GetTensorTypeAndShapeInfo().GetElementCount() +
                      tgt_tensor->GetTensorTypeAndShapeInfo().GetElementCount();

    switch (base_elem_type) {
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            return CreateConcatOrtTensor<float>(src_tensor, tgt_tensor, base_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
            return CreateConcatOrtTensor<double>(src_tensor, tgt_tensor, base_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            return CreateConcatOrtTensor<int>(src_tensor, tgt_tensor, base_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            return CreateConcatOrtTensor<int64_t>(src_tensor, tgt_tensor, base_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
            return CreateConcatOrtTensor<bool>(src_tensor, tgt_tensor, base_shape, base_dim);
        default:
            PYIS_THROW("ConcatTensor : data type not supported");
    }
}

std::shared_ptr<Ort::Value> GetTensorSliceByIndexSpan(const std::shared_ptr<Ort::Value>& tensor,
                                                      std::pair<size_t, size_t>& slice_span) {
    auto tensor_shape = tensor->GetTensorTypeAndShapeInfo().GetShape();
    auto base_elem_type = tensor->GetTensorTypeAndShapeInfo().GetElementType();
    auto base_dim = tensor->GetTensorTypeAndShapeInfo().GetDimensionsCount();
    if (tensor_shape.empty()) PYIS_THROW("GetTensorSliceByIndexSpan : empty tensor");
    if (slice_span.first > slice_span.second || slice_span.second > tensor_shape[0])
        PYIS_THROW("GetTensorSliceByIndexSpan : invalid slice indexes");

    size_t elem_per_slice = 1;
    for (int i = 1; i < tensor_shape.size(); i++) {
        elem_per_slice *= static_cast<size_t>(tensor_shape[i]);
    }

    size_t new_numel = (slice_span.second - slice_span.first) * elem_per_slice;
    std::shared_ptr<Ort::Value> ret_tensor_ptr;
    auto& allocator = OrtGlobals::Allocator;
    size_t start_offset = slice_span.first * elem_per_slice;
    void* src_data_ptr = nullptr;

    tensor_shape[0] = (slice_span.second - slice_span.first);

    switch (base_elem_type) {
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            src_data_ptr = static_cast<char*>(tensor->GetTensorMutableData<void>()) + start_offset * sizeof(float);
            return CreateSliceOrtTensor<float>(src_data_ptr, new_numel, tensor_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
            src_data_ptr = static_cast<char*>(tensor->GetTensorMutableData<void>()) + start_offset * sizeof(double);
            return CreateSliceOrtTensor<double>(src_data_ptr, new_numel, tensor_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            src_data_ptr = static_cast<char*>(tensor->GetTensorMutableData<void>()) + start_offset * sizeof(int);
            return CreateSliceOrtTensor<int>(src_data_ptr, new_numel, tensor_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            src_data_ptr = static_cast<char*>(tensor->GetTensorMutableData<void>()) + start_offset * sizeof(int64_t);
            return CreateSliceOrtTensor<int64_t>(src_data_ptr, new_numel, tensor_shape, base_dim);
        case ::ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
            src_data_ptr = static_cast<char*>(tensor->GetTensorMutableData<void>()) + start_offset * sizeof(bool);
            return CreateSliceOrtTensor<bool>(src_data_ptr, new_numel, tensor_shape, base_dim);
        default:
            PYIS_THROW("ConcatTensor : data type not supported");
    }
}

Ort::Value CopyTensor(Ort::Value& src_tensor) {
    auto tensor_copy =
        Ort::Value::CreateTensor(*OrtGlobals::Allocator, src_tensor.GetTensorTypeAndShapeInfo().GetShape().data(),
                                 src_tensor.GetTensorTypeAndShapeInfo().GetDimensionsCount(),
                                 src_tensor.GetTensorTypeAndShapeInfo().GetElementType());

    memcpy(tensor_copy.GetTensorMutableData<void>(), src_tensor.GetTensorData<void>(),
           src_tensor.GetTensorTypeAndShapeInfo().GetElementCount() * GetTensorElementBytes(src_tensor));

    return tensor_copy;
}

Ort::Value ShallowCopyTensor(Ort::Value& src_tensor) {
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    auto tensor_copy =
        Ort::Value::CreateTensor(memory_info, const_cast<void*>(src_tensor.GetTensorData<void>()),
                                 static_cast<size_t>(src_tensor.GetTensorTypeAndShapeInfo().GetElementCount() *
                                                     GetTensorElementBytes(src_tensor)),
                                 src_tensor.GetTensorTypeAndShapeInfo().GetShape().data(),
                                 src_tensor.GetTensorTypeAndShapeInfo().GetDimensionsCount(),
                                 src_tensor.GetTensorTypeAndShapeInfo().GetElementType());

    return tensor_copy;
}

}  // namespace ops
}  // namespace pyis