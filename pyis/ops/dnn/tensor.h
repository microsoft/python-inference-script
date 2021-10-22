// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include "pyis/share/exception.h"

namespace pyis {
namespace ops {
namespace dnn {

enum class TensorType {
    T_INT64 = 0,
    T_FLOAT32 = 1,
};

class Tensor {
  public:
    Tensor() = default;
    Tensor(TensorType type, std::vector<int64_t>& shape, size_t dimension, size_t size, void* data_ptr)
        : type_(type), ndim_(dimension), shape_(shape), size_(size) {
        // Allocating owning memory for each Tensor upon creation
        data_ptr_ = malloc(size_ * GetElementSize());
        if (data_ptr_ == nullptr) {
            PYIS_THROW("Constructing Tensor exception : insufficient memory");
        }
        memcpy(data_ptr_, data_ptr, size_ * GetElementSize());
    }

    ~Tensor() {
        if (data_ptr_ != nullptr) {
            free(data_ptr_);
            data_ptr_ = nullptr;
        }
    }

    TensorType GetType() { return type_; }

    void* DataMap() { return data_ptr_; }

    std::vector<int64_t>& GetShape() { return shape_; }

    size_t GetDimensions() const { return ndim_; }

    size_t GetSize() const { return size_; }

    size_t GetElementSize() {
        if (type_ == TensorType::T_FLOAT32) {
            return sizeof(float);
        }
        if (type_ == TensorType::T_INT64) {
            return sizeof(int64_t);
        }
        PYIS_THROW("Get Element Size Error : unsupported data type");
    }

    size_t GetTensorDataMemoryBytes() {
        if (type_ == TensorType::T_FLOAT32) {
            return sizeof(float) * size_;
        }
        if (type_ == TensorType::T_INT64) {
            return sizeof(int64_t) * size_;
        }
        PYIS_THROW("Get Element Size Error : unsupported data type");
    }

    std::shared_ptr<Tensor> Concat(std::shared_ptr<Tensor>& another, size_t concat_dim) {
        if (this->GetDimensions() != 2) {
            PYIS_THROW("Unsupported Exception for Tensor concat, currently only for 2 dimensional tensor");
        }
        if (concat_dim > 1 || concat_dim < 0) {
            PYIS_THROW("Invalid dimension for Tensor concat : only support concat by dim 0 or 1");
        }
        if (this->GetType() != another->GetType()) {
            PYIS_THROW("Concat Exception : Unmatched type");
        }

        if (concat_dim == 0) {
            if (this->GetShape()[1] != another->GetShape()[1]) {
                PYIS_THROW("Invalid shape for Tensor concat : second dimension must be equal");
            }
            std::vector<int64_t> new_shape{GetShape()[0] + another->GetShape()[0], this->GetShape()[1]};
            auto new_size = this->size_ + another->GetSize();
            auto new_ndim = this->ndim_;
            void* new_data_ptr = malloc(new_size * GetElementSize());
            if (new_data_ptr == nullptr) PYIS_THROW("Concat error : insufficient memory!");

            memcpy(static_cast<char*>(new_data_ptr), static_cast<char*>(data_ptr_), this->GetTensorDataMemoryBytes());
            memcpy((static_cast<char*>(new_data_ptr) + this->GetSize()), another->DataMap(),
                   another->GetTensorDataMemoryBytes());
            auto concated_tensor = std::make_shared<Tensor>(type_, new_shape, new_ndim, new_size, new_data_ptr);
            free(new_data_ptr);
            new_data_ptr = nullptr;
            return concated_tensor;
        }
    }

    std::shared_ptr<Tensor> Slice(size_t slice_indx) {
        std::vector<int64_t> new_shape{1, shape_[1]};
        auto new_size = static_cast<size_t>(size_ / shape_[0]);
        void* new_data_ptr = malloc(GetElementSize() * new_size);
        if (new_data_ptr == nullptr) PYIS_THROW("Slice error : insufficient memory!");

        memcpy(new_data_ptr, static_cast<char*>(data_ptr_) + shape_[1] * slice_indx, new_size * this->GetElementSize());
        auto sliced_tensor = std::make_shared<Tensor>(type_, new_shape, ndim_, size_, new_data_ptr);
        free(new_data_ptr);
        new_data_ptr = nullptr;
        return sliced_tensor;
    }

  private:
    void* data_ptr_;
    TensorType type_;
    std::vector<int64_t> shape_;
    size_t ndim_;
    size_t size_;
};
}  // namespace dnn
}  // namespace ops
}  // namespace pyis