// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>
#include <vector>

#include "ILinearChainCRF.h"
#include "SparseLinearModel.h"

namespace SparseLinearChainCRF {

// ------------------------------------------------
// VanillaCRF: a standard and basic CRF inference
// algorithm implementation.
// ------------------------------------------------
class VanillaCRF : public ILinearChainCRF {
  public:
    virtual void Initialize(std::shared_ptr<SparseLinearModel> model);
    virtual void Decode(const IndexedSentence& sentence, std::vector<uint16_t>& tags) const;
    virtual double Infer(const IndexedSentence& sentence, float* probNode, float* probEdge) const;

  protected:
    virtual void CreateLinearFunctionCache(const IndexedSentence& sentence, float* linearFunctionCache) const;
    virtual float Viterbi1Best(const float* linearFunctionCache, int wordCount, uint16_t* bestPathIds) const;
    virtual void Forward(const double* node, const double* edge, uint16_t wordCount, uint16_t labelCount,
                         double* matrix, double* scales) const;
    virtual void Backward(const double* node, const double* edge, uint16_t wordCount, uint16_t labelCount,
                          const double* scales, double* matrix) const;

  public:
    std::shared_ptr<SparseLinearModel> m_LinearModel;
};
}  // namespace SparseLinearChainCRF