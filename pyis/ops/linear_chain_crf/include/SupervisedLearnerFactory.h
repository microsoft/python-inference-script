// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "AbstractFactory.h"
#include "src/BaseSupervisedLearner.h"
#include "src/CumulativeL1SGDLearner.h"
#include "src/LBFGSLearner.h"
#include "src/PerceptronLearner.h"

namespace SparseLinearChainCRFConsole {
using namespace SparseLinearChainCRF;

// ------------------------------------------------
// SupervisedLearnerFactory
// ------------------------------------------------
class SupervisedLearnerFactory : public AbstractFactory<BaseSupervisedLearner> {
  public:
    SupervisedLearnerFactory() {
        Register(CreateInstance<PerceptronLearner, BaseSupervisedLearner>, "perceptron", "Structured Perceptron");
        Register(CreateInstance<PerceptronLearner, BaseSupervisedLearner>, "perc", "Structured Perceptron");
        Register(CreateInstance<CumulativeL1SGDLearner, BaseSupervisedLearner>, "l1sgd",
                 "Tsuruoka-Tsujii-Ananiadou algorithm for SGD-L1");
        Register(CreateInstance<CumulativeL1SGDLearner, BaseSupervisedLearner>, "tta",
                 "Tsuruoka-Tsujii-Ananiadou algorithm for SGD-L1");
        Register(CreateInstance<LBFGSLearner, BaseSupervisedLearner>, "lbfgs", "LBFGS Quasi-Newton Algorithm");
    }
};
}  // namespace SparseLinearChainCRFConsole