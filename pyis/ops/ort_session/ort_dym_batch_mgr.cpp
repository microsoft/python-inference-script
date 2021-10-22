// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ort_dym_batch_mgr.h"

#include <functional>

#include "ort_tensor_utils.h"
#include "pyis/share/str_utils.h"

namespace pyis {
namespace ops {

DynamicBatchManager::DynamicBatchManager(int max_batch_size, std::shared_ptr<Ort::Session> ort_session,
                                         std::vector<const char*>& input_names, std::vector<const char*>& output_names)
    : max_batch_size_(max_batch_size),
      ort_session_(std::move(ort_session)),
      input_names_(std::move(input_names)),
      output_names_(std::move(output_names)) {
    worker_thread_ = std::make_unique<std::thread>(&DynamicBatchManager::WorkerLoop, this);
    worker_thread_->detach();
}

void DynamicBatchManager::Execute(const std::vector<std::shared_ptr<Ort::Value>>& inputs,
                                  std::vector<std::shared_ptr<Ort::Value>>& outputs) {
    bool first_query(false);
    auto error_message_promise = std::make_shared<std::promise<std::string>>();

    std::unique_lock<std::mutex> lock(queue_mutex_);

    if (batch_queue_.empty()) {
        first_query = true;
    }

    if (first_query || batch_queue_.back()->BatchSize() == max_batch_size_) {
        auto context = std::make_shared<BatchContext>(inputs.size(), outputs.size());
        batch_queue_.emplace(context);
    }

    auto batch_context = batch_queue_.back();

    size_t output_index_start = batch_context->BatchTileCount();
    size_t output_index_end =
        batch_context->BatchTileCount() + static_cast<size_t>(inputs[0]->GetTensorTypeAndShapeInfo().GetShape()[0]);
    batch_context->error_message_promises_.push_back(error_message_promise);
    ConcatInputs(batch_context, inputs);

    lock.unlock();

    // Tell WorkerExecute there's a new query
    if (first_query) {
        cv_.notify_one();
    }

    auto future = error_message_promise->get_future();
    auto timeout_point = std::chrono::system_clock::now() + std::chrono::milliseconds(5000);

    if (std::future_status::ready == future.wait_until(timeout_point)) {
        auto error_message = future.get();
        auto index_span = std::make_pair(output_index_start, output_index_end);
        SliceOutputs(batch_context, outputs, index_span);

    } else {
        PYIS_THROW("Dynamic Batch Timeout");
    }
}

void DynamicBatchManager::WorkerLoop() {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    while (true) {
        while (batch_queue_.empty()) {
            // WorkerLoop will be notified when there's a new request
            cv_.wait(lock);
        }

        // Take first BatchContext out of queue
        auto batch_context = batch_queue_.front();
        batch_queue_.pop();
        queue_mutex_.unlock();

        const auto begin = std::chrono::high_resolution_clock::now();
        ModelExecute(batch_context);

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        NotifyResults(batch_context, std::string());

        // Lock the queue. In next iteration if m_contextQueue.size() == 0, m_cv will release the lock and wait.
        queue_mutex_.lock();
    }
}

void DynamicBatchManager::ModelExecute(std::shared_ptr<BatchContext>& batch_context) {
    std::vector<Ort::Value> input_tensor_data;
    input_tensor_data.reserve(batch_context->concat_inputs_.size());
    for (const auto& tensor_ptr : batch_context->concat_inputs_) {
        input_tensor_data.emplace_back(CopyTensor(*tensor_ptr));
    }

    auto outputs = ort_session_->Run(Ort::RunOptions(), input_names_.data(), input_tensor_data.data(),
                                     batch_context->concat_inputs_.size(), output_names_.data(), output_names_.size());

    for (auto& output_tensor : outputs) {
        batch_context->concat_outputs_.emplace_back(std::make_shared<Ort::Value>(CopyTensor(output_tensor)));
    }
}

void DynamicBatchManager::ConcatInputs(const std::shared_ptr<BatchContext>& batch_context,
                                       const std::vector<std::shared_ptr<Ort::Value>>& inputs) {
    if (batch_context->BatchSize() == 1) {
        batch_context->concat_inputs_ = inputs;
        return;
    }
    for (size_t i = 0; i < inputs.size(); i++) {
        batch_context->concat_inputs_[i] = ConcatTensors(batch_context->concat_inputs_[i], inputs[i]);
    }
}

void DynamicBatchManager::SliceOutputs(const std::shared_ptr<BatchContext>& batch_context,
                                       std::vector<std::shared_ptr<Ort::Value>>& targets,
                                       std::pair<size_t, size_t>& index_span) {
    targets.reserve(batch_context->concat_outputs_.size());
    for (size_t i = 0; i < batch_context->concat_outputs_.size(); i++) {
        if (batch_context->BatchSize() == 1) {
            targets.emplace_back(batch_context->concat_outputs_[i]);
        } else {
            targets.emplace_back(GetTensorSliceByIndexSpan(batch_context->concat_outputs_[i], index_span));
        }
    }
}

void DynamicBatchManager::NotifyResults(const std::shared_ptr<BatchContext>& batch_context,
                                        const std::string& message) {
    for (size_t i = 0; i < batch_context->BatchSize(); i++) {
        // Notify waiting thread when it's finished
        batch_context->error_message_promises_[i]->set_value(std::string(message));
    }
}
}  // namespace ops
}  // namespace pyis