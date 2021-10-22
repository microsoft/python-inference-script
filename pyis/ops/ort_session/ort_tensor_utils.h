// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <cassert>
#include <numeric>

#include "ort_globals.h"
#include "pyis/share/exception.h"

namespace pyis {
namespace ops {

bool CheckTensorDimensionMatch(std::vector<int64_t>& shape_src, std::vector<int64_t>& shape_tgt);

template <typename T>
std::shared_ptr<Ort::Value> CreateConcatOrtTensor(const std::vector<std::shared_ptr<Ort::Value>>& tensors,
                                                  std::vector<int64_t>& shape, size_t dim);

template <typename T>
std::shared_ptr<Ort::Value> CreateConcatOrtTensor(const std::shared_ptr<Ort::Value>& src_tensor,
                                                  const std::shared_ptr<Ort::Value>& tgt_tensor,
                                                  const std::vector<int64_t>& shape, size_t dim);

template <typename T>
std::shared_ptr<Ort::Value> CreateSliceOrtTensor(void* data_ptr, size_t numel, std::vector<int64_t>& shape, size_t dim);

size_t GetTensorElementBytes(Ort::Value& tensor);

void* GetTensorDatamap(Ort::Value& tensor);

std::shared_ptr<Ort::Value> ConcatTensors(std::vector<std::shared_ptr<Ort::Value>> tensors);

std::shared_ptr<Ort::Value> ConcatTensors(const std::shared_ptr<Ort::Value>& src_tensor,
                                          const std::shared_ptr<Ort::Value>& tgt_tensor);

std::shared_ptr<Ort::Value> GetTensorSliceByIndexSpan(const std::shared_ptr<Ort::Value>& tensor,
                                                      std::pair<size_t, size_t>& slice_span);

Ort::Value CopyTensor(Ort::Value& src_tensor);

Ort::Value ShallowCopyTensor(Ort::Value& src_tensor);

}  // namespace ops
}  // namespace pyis