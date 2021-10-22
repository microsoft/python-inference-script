// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "AbstractFactory.h"
#include "src/FrenchVanillaCRF.h"
#include "src/ILinearChainCRF.h"
#include "src/TagConstrainedCRF.h"
#include "src/VanillaCRF.h"

namespace SparseLinearChainCRFConsole {
using namespace SparseLinearChainCRF;

// ------------------------------------------------
// CRFFactory
// ------------------------------------------------
class CRFFactory : public AbstractFactory<ILinearChainCRF> {
  public:
    CRFFactory() {
        Register(CreateInstance<VanillaCRF, ILinearChainCRF>, "VanillaCRF", "Vanilla CRF");
        Register(CreateInstance<VanillaCRF, ILinearChainCRF>, "vanilla", "Vanilla CRF");
        Register(CreateInstance<FrenchVanillaCRF, ILinearChainCRF>, "FrenchVanillaCRF", "FrenchVanilla CRF");
        Register(CreateInstance<FrenchVanillaCRF, ILinearChainCRF>, "sparse", "FrenchVanilla CRF");
        Register(CreateInstance<TagConstrainedCRF, ILinearChainCRF>, "TagConstrainedCRF", "TagConstrained CRF");
        Register(CreateInstance<TagConstrainedCRF, ILinearChainCRF>, "constrained", "TagConstrained CRF");
    }
};
}  // namespace SparseLinearChainCRFConsole