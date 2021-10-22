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
// CumulativeL1SGDLearner
// Implementaiton of Cumulative-L1 regularization technique for SGD,
// introduced by Yoshimasa Tsuruoka, Jun'ichi Tsujii,  Sophia Ananiadou,
// ACL 2009. This algorithm updates the weight vector by SGD for a
// single example (aka Perceptron-style update) and then applies L1
// penalty for clipping.
// ------------------------------------------------
class CumulativeL1SGDLearner : public BaseSupervisedLearner {
  public:
    CumulativeL1SGDLearner();

    void Initialize(std::shared_ptr<ILinearChainCRF> CRF, std::shared_ptr<SparseLinearModel> linearModel,
                    std::map<std::string, std::string>& options);

  private:
    virtual double SinglePassTraining(std::shared_ptr<StreamDataManager> trainData,
                                      std::shared_ptr<StreamDataManager> devData, float learningRate);

    std::vector<float> m_CumulativeL1PenalizedWeight;
    std::vector<float> m_CumulativeL1PenalizedTransitionWeight;
    float m_CumulativeL1Penalty;
    float m_L1Penalty;
};
}  // namespace SparseLinearChainCRF