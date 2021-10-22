// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <future>

#include "ort_globals.h"

namespace pyis {
namespace ops {
struct BatchContext {
    BatchContext(size_t input_size, size_t output_size) {
        concat_inputs_.reserve(input_size);
        concat_outputs_.reserve(output_size);
    }

    std::vector<std::shared_ptr<Ort::Value>> concat_inputs_;
    std::vector<std::shared_ptr<Ort::Value>> concat_outputs_;
    std::vector<std::shared_ptr<std::promise<std::string>>> error_message_promises_;

    size_t BatchSize() const { return error_message_promises_.size(); }
    size_t BatchTileCount() const {
        if (BatchSize() > 0) {
            // get the tile count
            return concat_inputs_[0]->GetTensorTypeAndShapeInfo().GetShape()[0];
        }
        return 0;
    }
};
}  // namespace ops
}  // namespace pyis