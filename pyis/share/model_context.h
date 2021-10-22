// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "cached_object.h"
#include "json_persist_helper.h"
#include "logging.h"

namespace pyis {

class ModelStorage;

class ModelContext {
  public:
    explicit ModelContext(const std::string& path, const std::string& data_archive = "");
    virtual ~ModelContext() = default;

    std::string Path() { return path_; }
    ModelStorage& Storage() { return *storage_; }

    // Add a common prefix to filenames when saving a model
    std::string SetFilePrefix(const std::string& prefix);

    template <typename T, typename std::enable_if<std::is_base_of<CachedObject<T>, T>::value, T>::type* = nullptr>
    std::shared_ptr<T> GetOrCreateObject(const std::string& state);

    template <typename T, typename std::enable_if<std::is_base_of<CachedObject<T>, T>::value, T>::type* = nullptr>
    void ClearCache();

    static void Activate(ModelContext* ctx);
    static void Deactivate(ModelContext* ctx);
    static ModelContext* GetActive();

  private:
    std::string path_;
    std::string data_dir_;
    std::string data_archive_;
    std::shared_ptr<ModelStorage> storage_;

    std::string file_prefix_;

    static ModelContext* active_model_context;
};

template <typename T, typename std::enable_if<std::is_base_of<CachedObject<T>, T>::value, T>::type*>
std::shared_ptr<T> ModelContext::GetOrCreateObject(const std::string& state) {
    JsonPersistHelper jph(state);
    std::string key = jph.sign(&Storage());

    std::shared_ptr<T> res = CachedObject<T>::Get(key);
    if (res) {
        LOG_INFO("load object from cache. signature:%s, state:%s", key.c_str(), state.c_str());
        return res;
    }

    res = std::make_shared<T>();
    res->Deserialize(state, Storage());
    CachedObject<T>::Add(key, res);

    return res;
}

template <typename T, typename std::enable_if<std::is_base_of<CachedObject<T>, T>::value, T>::type*>
void ModelContext::ClearCache() {
    CachedObject<T>::Clear();
}

}  // namespace pyis