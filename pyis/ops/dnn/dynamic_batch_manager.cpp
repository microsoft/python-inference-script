// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <functional>
#include <iostream>

#include "ort_dym_batch_mgr.h"
#include "pyis/share/exception.h"

namespace pyis {
namespace ops {
namespace dnn {

DynamicBatchManager::DynamicBatchManager(int max_batch_size) : max_batch_size_(max_batch_size) {
    worker_thread_ = std::make_unique<std::thread>(&DynamicBatchManager::WorkerLoop, this);
    worker_thread_->detach();
}

void DynamicBatchManager::Execute(const std::vector<std::string>& input_names,
                                  const std::vector<std::shared_ptr<Tensor>>& inputs,
                                  const std::vector<std::string>& output_names,
                                  std::vector<std::shared_ptr<Tensor>>& outputs) {
    bool first_query(false);
    auto error_message_promise = std::make_shared<std::promise<std::string>>();

    std::unique_lock<std::mutex> lock(queue_mutex_);

    if (batch_queue_.empty()) {
        first_query = true;
    }

    bool first_in_batch = false;

    if (first_query || batch_queue_.back()->BatchSize() == max_batch_size_) {
        auto context = std::make_shared<BatchContext>(inputs.size(), outputs.size());
        batch_queue_.emplace(context);
        first_in_batch = true;
    }

    auto batch_context = batch_queue_.back();
    if (first_in_batch) {
        // if the first query in a new batch, set the input and ouput names
        batch_context->batch_input_names_ = input_names;
        batch_context->batch_output_names_ = output_names;
    } else {
        bool is_input_output_consistent = batch_context->CheckInputOutputNames(input_names, output_names);
        if (!is_input_output_consistent) {
            // try {
            auto single_output_values = session_impl_->Execute(input_names, inputs, output_names);
            outputs = single_output_values;
            //} catch (std::runtime_error& e) {
            // log runtime error e.what()

            //} catch (...) {
            // log unknown error in model execute
            //}

            lock.unlock();
            return;
        }
    }
    auto output_index = batch_context->BatchSize();
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
        SliceOutputs(batch_context, outputs, output_index);
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

        // Take first BatchContext out of quueue
        auto batch_context = batch_queue_.front();
        batch_queue_.pop();
        queue_mutex_.unlock();

        // try {
        const auto begin = std::chrono::high_resolution_clock::now();
        ModelExecute(batch_context);

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        NotifyResults(batch_context, std::string());
        //} catch (const std::exception& e) {
        //    NotifyResults(batch_context, e.what());
        //} catch (...) {
        NotifyResults(batch_context, std::string("Dynamic Batch Unknown Error"));
        //}

        // Lock the queue. In next iteration if m_contextQueue.size() == 0, m_cv will release the lock and wait.
        queue_mutex_.lock();
    }
}

void DynamicBatchManager::ModelExecute(std::shared_ptr<BatchContext>& batch_context) {
    std::vector<std::shared_ptr<Tensor>> input_tensors(batch_context->concat_inputs_.size());
    std::vector<std::shared_ptr<Tensor>> output_tensors(batch_context->concat_outputs_.size());

    for (int i = 0; i < batch_context->concat_inputs_.size(); i++) {
        input_tensors[i] = batch_context->concat_inputs_[i];
    }

    auto output_values =
        session_impl_->Execute(batch_context->batch_input_names_, input_tensors, batch_context->batch_output_names_);

    batch_context->concat_outputs_ = output_values;
}

void DynamicBatchManager::ConcatInputs(const std::shared_ptr<BatchContext>& batch_context,
                                       const std::vector<std::shared_ptr<Tensor>>& inputs) {
    for (size_t i = 0; i < inputs.size(); i++) {
        auto source = inputs[i];
        // No need to concat input if there's only one request
        if (batch_context->BatchSize() == 1) {
            batch_context->concat_inputs_[i] = source;
        } else {
            batch_context->concat_inputs_[i] = batch_context->concat_inputs_[i]->Concat(source, 0);
        }
    }
}

void DynamicBatchManager::SliceOutputs(const std::shared_ptr<BatchContext>& batch_context,
                                       std::vector<std::shared_ptr<Tensor>>& targets, size_t index) {
    targets.reserve(batch_context->concat_outputs_.size());
    for (size_t i = 0; i < batch_context->concat_outputs_.size(); i++) {
        if (batch_context->BatchSize() == 1) {
            targets.emplace_back(batch_context->concat_outputs_[i]);
        } else {
            targets.emplace_back(batch_context->concat_outputs_[i]->Slice(index));
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
}  // namespace dnn
}  // namespace ops
}  // namespace pyis