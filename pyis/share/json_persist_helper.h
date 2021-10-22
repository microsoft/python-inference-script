// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <set>
#include <string>
#include <vector>

#include "model_storage.h"
#include "third_party/md5/md5.hpp"

#pragma warning(push, 0)
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#pragma warning(pop)

namespace pyis {

class JsonPersistHelper {
  public:
    explicit JsonPersistHelper(int version);
    explicit JsonPersistHelper(const std::string& state);
    JsonPersistHelper(const std::string& state, ModelStorage& storage);

    template <typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
    JsonPersistHelper& add(const std::string& key, const T& value, bool configurable = false);
    JsonPersistHelper& add(const std::string& key, const std::string& value, bool configurable = false);

    template <typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
    JsonPersistHelper& add(const std::string& key, const std::vector<T>& value, bool configurable = false);
    JsonPersistHelper& add(const std::string& key, const std::vector<std::string>& value, bool configurable = false);

    template <typename T, typename std::enable_if<std::is_same<T, bool>::value, T>::type* = nullptr>
    T get(const std::string& key);
    template <typename T, typename std::enable_if<std::is_same<T, int>::value, T>::type* = nullptr>
    T get(const std::string& key);
    template <typename T, typename std::enable_if<std::is_same<T, int64_t>::value, T>::type* = nullptr>
    T get(const std::string& key);
    template <typename T, typename std::enable_if<std::is_same<T, unsigned int>::value, T>::type* = nullptr>
    T get(const std::string& key);
    template <typename T, typename std::enable_if<std::is_same<T, uint64_t>::value, T>::type* = nullptr>
    T get(const std::string& key);
    template <typename T, typename std::enable_if<std::is_same<T, float>::value, T>::type* = nullptr>
    T get(const std::string& key);
    template <typename T, typename std::enable_if<std::is_same<T, double>::value, T>::type* = nullptr>
    T get(const std::string& key);
    template <typename T, typename std::enable_if<std::is_same<T, std::vector<std::string>>::value, T>::type* = nullptr>
    T get(const std::string& key);
    std::string get(const std::string& key);

    JsonPersistHelper& add_file(const std::string& key, const std::string& file, bool configurable = false);
    std::string get_file(const std::string& key);

    std::string serialize();
    std::string serialize(const std::string& config_file, ModelStorage& storage);
    void deserialize(const std::string& state);
    void deserialize(const std::string& state, ModelStorage& storage);

    int version();

    std::string sign(ModelStorage* storage = nullptr);

  private:
    JsonPersistHelper();
    void sign(rapidjson::Value* obj, boost::uuids::detail::md5& md5, ModelStorage* storage = nullptr);
    void sign_file(const std::string& file_path, ModelStorage& storage, boost::uuids::detail::md5& md5);

    std::set<std::string> configurables_;
    rapidjson::Document doc_;
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator_;
};

template <typename T, typename std::enable_if<std::is_integral<T>::value, T>::type*>
JsonPersistHelper& JsonPersistHelper::add(const std::string& key, const T& value, bool configurable) {
    rapidjson::Value k(key.c_str(), allocator_);
    doc_.AddMember(k, value, allocator_);

    if (configurable) {
        configurables_.insert(key);
    }

    return *this;
}

template <typename T, typename std::enable_if<std::is_integral<T>::value, T>::type*>
JsonPersistHelper& JsonPersistHelper::add(const std::string& key, const std::vector<T>& value, bool configurable) {
    rapidjson::Value arr(rapidjson::kArrayType);

    for (const auto& i : value) {
        arr.PushBack(i, allocator_);
    }

    rapidjson::Value k(key.c_str(), allocator_);
    doc_.AddMember(k, arr, allocator_);

    if (configurable) {
        configurables_.insert(key);
    }

    return *this;
}

template <typename T, typename std::enable_if<std::is_same<T, bool>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    return value.GetBool();
}

template <typename T, typename std::enable_if<std::is_same<T, int>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    return value.GetInt();
}

template <typename T, typename std::enable_if<std::is_same<T, unsigned int>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    return value.GetUint();
}

template <typename T, typename std::enable_if<std::is_same<T, int64_t>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    return value.GetInt64();
}

template <typename T, typename std::enable_if<std::is_same<T, uint64_t>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    return value.GetUint64();
}

template <typename T, typename std::enable_if<std::is_same<T, float>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    return value.GetFloat();
}

template <typename T, typename std::enable_if<std::is_same<T, double>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    return value.GetDouble();
}

template <typename T, typename std::enable_if<std::is_same<T, std::vector<std::string>>::value, T>::type*>
T JsonPersistHelper::get(const std::string& key) {
    const rapidjson::Value& value = doc_[key.c_str()];
    std::vector<std::string> result;
    auto arr = value.GetArray();
    result.reserve(arr.Size());
    for (const auto& elem : arr) {
        result.emplace_back(elem.GetString());
    }
    return result;
}

}  // namespace pyis