// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "ICommand.h"

namespace SparseLinearChainCRFConsole {
// ------------------------------------------------
// Decode command
// ------------------------------------------------
class Decode : public ICommand {
  public:
    Decode();

    virtual void Run(std::map<std::string, std::string>& options);

    virtual std::map<std::string, std::string> GetProgramOption() const { return m_OptionDesc; }

  private:
    std::map<std::string, std::string> m_OptionDesc;
};
}  // namespace SparseLinearChainCRFConsole