// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "model_context.h"

#include <iostream>

#include "exception.h"
#include "file_system.h"
#include "logging.h"
#include "model_storage_local.h"

namespace pyis {

ModelContext* ModelContext::active_model_context = nullptr;
std::mutex model_context_mutex;

ModelContext::ModelContext(const std::string& path, const std::string& data_archive) {
    path_ = path;
    data_dir_ = FileSystem::dirname(path_);
    data_archive_ = data_archive;
    if (!data_archive_.empty()) {
        PYIS_THROW("external data archive is not supported yet");
    }
    storage_ = std::make_shared<ModelStorageLocal>(data_dir_);
}

std::string ModelContext::SetFilePrefix(const std::string& prefix) {
    file_prefix_ = prefix;
    storage_->set_file_prefix(prefix);
    return file_prefix_;
}

void ModelContext::Activate(ModelContext* ctx) {
    model_context_mutex.lock();
    active_model_context = ctx;
}

void ModelContext::Deactivate(ModelContext* ctx) {
    if (ctx != active_model_context) {
        PYIS_THROW("unable to deactive model context. it is not the currently activated one");
    }
    active_model_context = nullptr;
    model_context_mutex.unlock();
}

ModelContext* ModelContext::GetActive() { return active_model_context; }

}  // namespace pyis