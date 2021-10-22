// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "logging.h"
#include "str_utils.h"

#ifndef PYIS_NO_EXCEPTIONS
#define PYIS_THROW(...) throw std::runtime_error(::pyis::fmt_str(__VA_ARGS__))
#else
#define PYIS_THROW(...)         \
    do {                        \
        LOG_ERROR(__VA_ARGS__); \
        abort();                \
    } while (false)
#endif

namespace pyis {}