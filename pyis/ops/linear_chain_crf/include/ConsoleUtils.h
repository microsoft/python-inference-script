// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <map>
#include <string>

namespace SparseLinearChainCRFConsole {
// ------------------------------------------------
// ConsoleUtils
// ------------------------------------------------
class ConsoleUtils {
  public:
    static std::map<std::string, std::string> ParseArgs(std::map<std::string, std::string>& options,
                                                        std::map<std::string, std::string>& optionDesc,
                                                        bool allowUnregistered = false);
};
}  // namespace SparseLinearChainCRFConsole