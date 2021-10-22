// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "AbstractFactory.h"
#include "Decode.h"
#include "ICommand.h"
#include "Learn.h"

namespace SparseLinearChainCRFConsole {
// ------------------------------------------------
// CommandFactory
// ------------------------------------------------
class CommandFactory : public AbstractFactory<ICommand> {
  public:
    CommandFactory() {
        Register(CreateInstance<Decode, ICommand>, "decode", "Decodes a CRF model");
        Register(CreateInstance<Learn, ICommand>, "learn", "Learns a CRF model with supervised learning");
    }
};
}  // namespace SparseLinearChainCRFConsole