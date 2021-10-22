// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <fstream>
#include <map>
#include <string>

namespace SparseLinearChainCRFConsole {
class ICommand {
  public:
    virtual ~ICommand() {}

    virtual void Run(std::map<std::string, std::string>& options) = 0;
};

inline bool CheckFileExists(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    return true;
}

}  // namespace SparseLinearChainCRFConsole