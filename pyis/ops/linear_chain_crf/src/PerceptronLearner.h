// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>
#include <vector>

#include "BaseSupervisedLearner.h"
#include "Common.h"
#include "ILinearChainCRF.h"
#include "SparseLinearModel.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// PerceptronLearner
// ------------------------------------------------
class PerceptronLearner : public BaseSupervisedLearner {
  public:
    PerceptronLearner();

  private:
    virtual double SinglePassTraining(std::shared_ptr<StreamDataManager> trainData,
                                      std::shared_ptr<StreamDataManager> devData, float learningRate);

    std::vector<size_t> m_TrainSetShuffleIndex;
};
}  // namespace SparseLinearChainCRF