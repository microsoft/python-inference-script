// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <stdexcept>

#include "exception.h"

namespace pyis {

template <class T>
class Expected {
  public:
    explicit Expected(const T& t) : val_(t), ok_(true) {}

    explicit Expected(T&& t) : val_(std::move(t)), ok_(true) {}

    Expected(Expected&& rhs) noexcept : ok_(rhs.ok_) {
        if (ok_) {
            new (&val_) T(std::move(rhs.val_));
        } else {
            new (&err_) std::runtime_error(std::move(rhs.err_));
        }
    }

    explicit Expected(std::runtime_error e) : err_(std::move(e)), ok_(false) {}  // NOLINT

    ~Expected() {
        if (ok_) {
            val_.~T();
        } else {
            err_.~runtime_error();
        }
    }

    T& operator*() {
        if (!ok_) {
            PYIS_THROW(err_.what());
        }
        return val_;
    }

    const T& operator*() const {
        if (!ok_) {
            PYIS_THROW(err_.what());
        }
        return val_;
    }

    explicit operator bool() const noexcept { return ok_; }

    bool has_value() const noexcept { return ok_; }
    const T& value() const& {
        if (!ok_) {
            PYIS_THROW(err_.what());
        }
        return val_;
    }

    bool has_error() const noexcept { return !ok_; }
    std::runtime_error error() const& {
        if (ok_) {
            PYIS_THROW("no error occurred");
        }
        return err_;
    }

  private:
    bool ok_;
    union {
        T val_;
        std::runtime_error err_;
    };
};

template <>
class Expected<void> {
  public:
    Expected() : ok_(true) {}

    Expected(Expected<void>&& rhs) noexcept : ok_(rhs.ok_) {
        if (!ok_) {
            new (&err_) std::runtime_error(std::move(rhs.err_));
        }
    }

    explicit Expected(std::runtime_error e) : err_(std::move(e)), ok_(false) {}  // NOLINT

    ~Expected() {
        if (!ok_) {
            err_.~runtime_error();
        }
    }

    explicit operator bool() const noexcept { return ok_; }

    bool has_value() const noexcept { return ok_; }

    bool has_error() const noexcept { return !ok_; }
    std::runtime_error error() const& {
        if (ok_) {
            PYIS_THROW("no error occurred");
        }
        return err_;
    }

  private:
    bool ok_;
    union {
        std::runtime_error err_;
    };
};

}  // namespace pyis