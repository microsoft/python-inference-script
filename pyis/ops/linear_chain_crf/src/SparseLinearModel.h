// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <climits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "Sentence.h"

namespace SparseLinearChainCRF {
#define EDGE_FEATURE_SET 0
#define INDEX_NOT_FOUND UINT_MAX

// ------------------------------------------------
// SparseLinearModel class.
// ------------------------------------------------
class SparseLinearModel {
  public:
    SparseLinearModel();
    SparseLinearModel(const std::string& filename);
    SparseLinearModel(istream& stream);
    ~SparseLinearModel() {}

    bool Serialize(const std::string& txtModelFile) const;
    bool Serialize(ostream& stream) const;
    bool Deserialize(const std::string& modelFile);
    bool Deserialize(istream& stream);

    size_t Shrink(float truncation = 0.0f);
    size_t Expand(const std::vector<MLGFeatureSentence>& data);

    size_t FindFeautureIdMap(uint32_t setId, uint32_t featId);
    const std::unordered_map<uint16_t, size_t>& FindWeightIds(size_t uid) { return m_WeightIndex[uid]; }
    const std::vector<std::pair<uint16_t, size_t>>& FindParameterVector(size_t uid) {
        return m_ParameterVectorIndex[uid];
    }

    size_t Size() const { return m_WeightVector.size(); }
    uint16_t MaxLabel() const { return m_MaxLabelState; }
    std::vector<float>& WeightVector() { return m_WeightVector; }

    void Reset();
    void Reset(const std::vector<float>& paramVector);
    float* GetTransitionCache() const { return m_TrainsitionCache.get(); }
    const std::unordered_map<uint32_t, size_t>& FindAllFeatures(uint32_t setId) { return m_FeatureToUid[setId]; }
    void BackPropagateTransitionWeight();
    void RefreshCache();

  private:
    void InsertParameter(uint32_t setId, uint32_t featId, uint16_t labId);
    void CreateParameterVectorIndex();

    std::vector<std::unordered_map<uint32_t, size_t>> m_FeatureToUid;
    std::vector<std::unordered_map<uint16_t, size_t>> m_WeightIndex;
    std::vector<std::vector<std::pair<uint16_t, size_t>>> m_ParameterVectorIndex;
    std::vector<float> m_WeightVector;
    uint16_t m_MaxLabelState;
    std::unique_ptr<float[]> m_TrainsitionCache;
};
}  // namespace SparseLinearChainCRF