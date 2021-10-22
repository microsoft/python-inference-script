// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Common.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// StringUtils
// ------------------------------------------------
class StringUtils {
  public:
    static std::vector<std::string> Split(const std::string& str, const std::string& delim,
                                          bool removeEmptyEntries = true);

    static bool ToBool(const std::string& str);
};
}  // namespace SparseLinearChainCRF
