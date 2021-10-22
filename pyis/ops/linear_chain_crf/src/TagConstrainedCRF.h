// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include "ILinearChainCRF.h"
#include "SparseLinearModel.h"
#include "VanillaCRF.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// TagConstrainedCRF: CRF that contrains on subset of state spaces
// ------------------------------------------------
class TagConstrainedCRF : public VanillaCRF {
  public:
    virtual void Decode(const IndexedSentence& sentence, std::vector<uint16_t>& tags) const;
    virtual double Infer(const IndexedSentence& sentence, float* probNode, float* probEdge) const;

  protected:
    void CreateConstrainedLinearFunctionCache(const IndexedSentence& sentence,
                                              const std::unordered_set<uint16_t>& activeTagset,
                                              float* linearFunctionCache) const;
    float ConstrainedViterbi1Best(const float* linearFunctionCache, int wordCount,
                                  const std::vector<uint16_t>& activeTagset, uint16_t* bestPathIds) const;
    void ConstrainedForward(const double* node, const double* edge, uint16_t wordCount, uint16_t labelCount,
                            const std::vector<uint16_t>& activeTagset, double* matrix, double* scales) const;
    void ConstrainedBackward(const double* node, const double* edge, uint16_t wordCount, uint16_t labelCount,
                             const std::vector<uint16_t>& activeTagset, const double* scales, double* matrix) const;
};
}  // namespace SparseLinearChainCRF