// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <functional>

class ScopeGuard {
  public:
    template <class Callable>
    explicit ScopeGuard(const Callable& undo_func) : f_(undo_func) {}

    ScopeGuard(ScopeGuard&& other) noexcept : f_(std::move(other.f_)) { other.f_ = nullptr; }

    ~ScopeGuard() {
        if (f_) {
            f_();
        }  // must not throw
    }

    void dismiss() noexcept { f_ = nullptr; }

    ScopeGuard(const ScopeGuard&) = delete;
    void operator=(const ScopeGuard&) = delete;

  private:
    std::function<void()> f_;
};