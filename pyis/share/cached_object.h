// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <map>
#include <memory>

namespace pyis {

template <class T>
class CachedObject {
  public:
    CachedObject() = default;

    static void Add(const std::string& key, std::shared_ptr<T> obj) { cache[key] = obj; }
    static std::shared_ptr<T> Get(const std::string& key) {
        std::shared_ptr<T> res;
        if (cache.count(key) != 0) {
            res = cache[key];
        }

        return res;
    }

    static void Clear() { cache.clear(); }

  private:
    static std::map<std::string, std::shared_ptr<T>> cache;
};

template <class T>
std::map<std::string, std::shared_ptr<T>> CachedObject<T>::cache;

}  // namespace pyis