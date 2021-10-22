// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>
#include <vector>

#include "BaseSupervisedLearner.h"
#include "Common.h"
#include "ILinearChainCRF.h"
#include "SparseLinearModel.h"
#include "StreamDataManager.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// LBFGSLearner
// L-BFGS Batch Learner
// ------------------------------------------------
class LBFGSLearner : public BaseSupervisedLearner {
  public:
    LBFGSLearner();

    void Initialize(std::shared_ptr<ILinearChainCRF> CRF, std::shared_ptr<SparseLinearModel> linearModel,
                    std::map<std::string, std::string>& options);

  private:
    virtual double SinglePassTraining(std::shared_ptr<StreamDataManager> trainData,
                                      std::shared_ptr<StreamDataManager> devData, float learningRate);

    float m_L1Penalty;
    float m_L2Penalty;
};
}  // namespace SparseLinearChainCRF