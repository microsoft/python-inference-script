// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "Common.h"
#include "Sentence.h"
#include "SparseLinearModel.h"

namespace SparseLinearChainCRF {
// Predefined AutoBuffer Counts
const size_t TOKEN_BUFFER_COUNT = 50;
const size_t LABEL_BUFFER_COUNT = 40;
const size_t NBEST_BUFFER_COUNT = 5;

// ------------------------------------------------
// LinearChainCRF Interface
// ------------------------------------------------
class ILinearChainCRF {
  public:
    virtual ~ILinearChainCRF() {}

    virtual void Initialize(std::shared_ptr<SparseLinearModel> model) = 0;

    virtual void Decode(const IndexedSentence& sentence, std::vector<uint16_t>& tags) const = 0;

    virtual void Decode(const IndexedSentence& sentence, std::vector<std::vector<uint16_t>>& nbestTags) const {
        // UNREFERENCED_PARAMETER(sentence);
        // UNREFERENCED_PARAMETER(nbestTags);
        LogAssert(false, "Nbest Decoding not found for this class.");
    }

    virtual double Infer(const IndexedSentence& sentence, float* probNode, float* probEdge) const = 0;
};
}  // namespace SparseLinearChainCRF