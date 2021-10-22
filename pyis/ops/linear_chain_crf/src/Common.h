// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <stdarg.h>
#include <stdint.h>

#include <cmath>
#include <iostream>
#include <string>

#include "pyis/share/exception.h"

using namespace std;

// Suppress a "contional expression is constant" warning by LogAssert
#pragma warning(disable : 4127)

namespace SparseLinearChainCRF {

#define LogAssert(exp, ...)          \
    do {                             \
        if (!(exp)) {                \
            PYIS_THROW(__VA_ARGS__); \
        }                            \
    } while (0)
}  // namespace SparseLinearChainCRF
