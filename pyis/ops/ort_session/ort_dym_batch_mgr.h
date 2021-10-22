// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <functional>
#include <memory>
#include <queue>

#include "ort_batch_context.h"

namespace pyis {
namespace ops {

class DynamicBatchManager final {
  public:
    explicit DynamicBatchManager(int max_batch_size, std::shared_ptr<Ort::Session> ort_session,
                                 std::vector<const char*>& input_names, std::vector<const char*>& output_names);

    void Execute(const std::vector<std::shared_ptr<Ort::Value>>& inputs,
                 std::vector<std::shared_ptr<Ort::Value>>& outputs);

  private:
    void WorkerLoop();
    void ModelExecute(std::shared_ptr<BatchContext>& batch_context);
    void ConcatInputs(const std::shared_ptr<BatchContext>& batch_context,
                      const std::vector<std::shared_ptr<Ort::Value>>& inputs);

    void SliceOutputs(const std::shared_ptr<BatchContext>& batch_context,
                      std::vector<std::shared_ptr<Ort::Value>>& targets, std::pair<size_t, size_t>& index_span);
    void NotifyResults(const std::shared_ptr<BatchContext>& batch_context, const std::string& message);

    std::unique_ptr<std::thread> worker_thread_;
    std::shared_ptr<Ort::Session> ort_session_;
    int max_batch_size_;

    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;

    std::queue<std::shared_ptr<BatchContext>> batch_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
};

}  // namespace ops
}  // namespace pyis