// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "Common.h"
#include "Sentence.h"
#include "SparseLinearModel.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// DataFormatter
// ------------------------------------------------
class DataFormatter {
  public:
    static std::vector<MLGFeatureSentence> Read(std::ifstream& fin, int size);

    static std::vector<IndexedSentence> IndexData(std::shared_ptr<SparseLinearModel> model,
                                                  const std::vector<MLGFeatureSentence>& data);
};
}  // namespace SparseLinearChainCRF