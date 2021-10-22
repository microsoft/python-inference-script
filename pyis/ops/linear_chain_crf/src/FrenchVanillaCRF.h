// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "SparseLinearModel.h"
#include "VanillaCRF.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// FrenchVanillaCRF: a CRF that runs on sparse transition matrix.
// Implements Method2 described in (Efficient inference of CRFs for
// large-scale natural language data. Minwoo Jeong et al. ACL 2009).
// ------------------------------------------------
class FrenchVanillaCRF : public VanillaCRF {
  public:
    virtual void Initialize(std::shared_ptr<SparseLinearModel> model);
    virtual double Infer(const IndexedSentence& sentence, float* probNode, float* probEdge) const;

  protected:
    virtual void Forward(const double* node, const double* edge, uint16_t wordCount, uint16_t labelCount,
                         double* matrix, double* scales) const;
    virtual void Backward(const double* node, const double* edge, uint16_t wordCount, uint16_t labelCount,
                          const double* scales, double* matrix) const;

  private:
    std::vector<std::vector<uint16_t>> m_SparseTransition;
};
}  // namespace SparseLinearChainCRF