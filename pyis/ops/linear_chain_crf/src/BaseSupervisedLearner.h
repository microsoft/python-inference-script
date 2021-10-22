// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <climits>
#include <cstring>
#include <map>
#include <memory>

#include "Common.h"
#include "ILinearChainCRF.h"
#include "SparseLinearModel.h"
#include "StreamDataManager.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// SupervisedLearner Base Class
// ------------------------------------------------
class BaseSupervisedLearner {
  public:
    BaseSupervisedLearner();
    virtual ~BaseSupervisedLearner() {}

    virtual void Initialize(std::shared_ptr<ILinearChainCRF> CRF, std::shared_ptr<SparseLinearModel> linearModel,
                            std::map<std::string, std::string>& options);

    virtual void Learn(std::shared_ptr<StreamDataManager> trainData, std::shared_ptr<StreamDataManager> devData);

    std::map<std::string, std::string>& GetProgramOption() { return m_OptionDesc; }

  protected:
    virtual double SinglePassTraining(std::shared_ptr<StreamDataManager> trainData,
                                      std::shared_ptr<StreamDataManager> devData, float learningRate) = 0;

    std::shared_ptr<ILinearChainCRF> m_CRF;
    std::shared_ptr<SparseLinearModel> m_LinearModel;
    std::map<std::string, std::string> m_Options;
    std::map<std::string, std::string> m_OptionDesc;

    bool m_Verbose;
    int m_RandomSeed;
};
}  // namespace SparseLinearChainCRF