// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ConsoleUtils.h"

#include <iostream>

#include "src/Common.h"

namespace SparseLinearChainCRFConsole {

std::map<std::string, std::string> ConsoleUtils::ParseArgs(std::map<std::string, std::string>& options,
                                                           std::map<std::string, std::string>& optionDesc,
                                                           bool allowUnregistered) {
    std::map<std::string, std::string> allOptions = optionDesc;
    for (auto& item : options) {
        if (optionDesc.count(item.first) == 0 && !allowUnregistered) {
            LogAssert(false, "Please use --help to see allowed option arguments");
        }
        allOptions[item.first] = item.second;
    }

    return allOptions;
}
}  // namespace SparseLinearChainCRFConsole