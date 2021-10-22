// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "SparseLinearModel.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "Sentence.h"

namespace SparseLinearChainCRF {
using namespace std;

namespace {
std::unordered_map<uint16_t, size_t> defaultAttribute;

void PushIndex(unordered_map<uint32_t, size_t>& featureToUid, vector<unordered_map<uint16_t, size_t>>& weightIndex,
               uint32_t featId, uint16_t labId, size_t paramId) {
    if (featureToUid.find(featId) == featureToUid.end()) {
        unordered_map<uint16_t, size_t> newLabelIndex;
        weightIndex.push_back(newLabelIndex);
        featureToUid.insert(make_pair(featId, weightIndex.size() - 1));
    }
    const auto& featureIndex = featureToUid.find(featId);
    size_t uid = featureIndex->second;

    const auto& labelIndex = weightIndex[uid].find(labId);
    if (labelIndex == weightIndex[uid].end()) {
        weightIndex[uid].insert(make_pair(labId, paramId));
    }
}

const int HEADER_SIZE = 4096;
const char* V1_MAGIC_WORD = "__LCCRF__v1";
}  // namespace

SparseLinearModel::SparseLinearModel() : m_MaxLabelState(0) {}

SparseLinearModel::SparseLinearModel(const string& filename) {
    ifstream stream(filename, ios::in | ios::binary);
    Deserialize(stream);
    stream.close();
}

SparseLinearModel::SparseLinearModel(istream& stream) { Deserialize(stream); }

void SparseLinearModel::Reset() { memset(m_WeightVector.data(), 0, sizeof(float) * m_WeightVector.size()); }

void SparseLinearModel::Reset(const std::vector<float>& vec) {
    LogAssert(vec.size() == m_WeightVector.size(), "Not able to copy vector that has different size");
    memcpy(m_WeightVector.data(), vec.data(), sizeof(float) * vec.size());
    RefreshCache();
}

bool SparseLinearModel::Serialize(ostream& stream) const {
    size_t bytesToWrite = (m_FeatureToUid.size() + 1) * sizeof(uint32_t) * 2 +
                          m_WeightVector.size() * (sizeof(uint16_t) + sizeof(float)) + HEADER_SIZE;

    for (const auto& featureIndex : m_FeatureToUid) {
        bytesToWrite += featureIndex.size() * (sizeof(uint32_t) + sizeof(uint16_t));
    }

    stream.write((char*)&bytesToWrite, sizeof(uint32_t));

    char header[HEADER_SIZE];
    memset(header, 0, HEADER_SIZE);
    strcpy(header, V1_MAGIC_WORD);
    stream.write(header, HEADER_SIZE);

    uint32_t featureSetSize = (uint32_t)m_FeatureToUid.size();

    uint32_t m_MaxLabelState32 = (uint32_t)m_MaxLabelState;
    stream.write((char*)&m_MaxLabelState32, sizeof(uint32_t));
    stream.write((char*)&featureSetSize, sizeof(uint32_t));

    size_t paramsWritten = 0;
    for (uint32_t featureSetIter = 0; featureSetIter < featureSetSize; ++featureSetIter) {
        const auto& featureIndex = m_FeatureToUid[featureSetIter];
        uint32_t numFeatures = (uint32_t)featureIndex.size();
        stream.write((char*)&featureSetIter, sizeof(uint32_t));
        stream.write((char*)&numFeatures, sizeof(uint32_t));

        map<uint32_t, size_t> orderedFeatureIndex(featureIndex.begin(), featureIndex.end());
        for (const auto& featureIndexIter : orderedFeatureIndex) {
            uint32_t featureId = featureIndexIter.first;
            const auto& attributeList = m_WeightIndex[featureIndexIter.second];

            uint16_t numAttribute = (uint16_t)attributeList.size();
            stream.write((char*)&featureId, sizeof(uint32_t));
            stream.write((char*)&numAttribute, sizeof(uint16_t));

            map<uint16_t, size_t> orderedAttribute(attributeList.begin(), attributeList.end());
            for (const auto& attribute : orderedAttribute) {
                uint16_t labelId = attribute.first;
                float weight = m_WeightVector[attribute.second];

                stream.write((char*)&labelId, sizeof(uint16_t));
                stream.write((char*)&weight, sizeof(float));

                paramsWritten++;
            }
        }
    }

    LogAssert(paramsWritten == m_WeightVector.size(), "Something wrong in serializing the model.");
    LogAssert(bytesToWrite + sizeof(uint32_t) == (size_t)stream.tellp(),
              "Model file was not correctly written. Some parameters are missing.");
    return true;
}

bool SparseLinearModel::Serialize(const std::string& txtModelFile) const {
    size_t bytesToWrite = (m_FeatureToUid.size() + 1) * sizeof(uint32_t) * 2 +
                          m_WeightVector.size() * (sizeof(uint16_t) + sizeof(float)) + HEADER_SIZE;

    for (const auto& featureIndex : m_FeatureToUid) {
        bytesToWrite += featureIndex.size() * (sizeof(uint32_t) + sizeof(uint16_t));
    }

    std::ofstream txtModel(txtModelFile.c_str(), std::ofstream::out);

    char header[HEADER_SIZE];
    memset(header, 0, HEADER_SIZE);
    strcpy(header, V1_MAGIC_WORD);

    uint32_t featureSetSize = (uint32_t)m_FeatureToUid.size();

    uint32_t m_MaxLabelState32 = (uint32_t)m_MaxLabelState;

    txtModel << "MaxLabelState:" << m_MaxLabelState << " "
             << "FeatureSetSize:" << featureSetSize << std::endl;
    txtModel << std::endl;

    size_t paramsWritten = 0;
    for (uint32_t featureSetIter = 0; featureSetIter < featureSetSize; ++featureSetIter) {
        const auto& featureIndex = m_FeatureToUid[featureSetIter];
        uint32_t numFeatures = (uint32_t)featureIndex.size();

        txtModel << "FeatureSetIter:" << featureSetIter << " "
                 << "NumFeatures:" << numFeatures << std::endl;
        txtModel << std::endl;

        map<uint32_t, size_t> orderedFeatureIndex(featureIndex.begin(), featureIndex.end());
        for (const auto& featureIndexIter : orderedFeatureIndex) {
            uint32_t featureId = featureIndexIter.first;
            const auto& attributeList = m_WeightIndex[featureIndexIter.second];

            uint16_t numAttribute = (uint16_t)attributeList.size();

            txtModel << "featureId:" << featureId << " "
                     << "numAttribute:" << numAttribute << std::endl;

            map<uint16_t, size_t> orderedAttribute(attributeList.begin(), attributeList.end());
            for (const auto& attribute : orderedAttribute) {
                uint16_t labelId = attribute.first;
                float weight = m_WeightVector[attribute.second];

                txtModel << labelId << " " << weight << std::endl;
                paramsWritten++;
            }
            txtModel << std::endl;
        }
    }

    LogAssert(paramsWritten == m_WeightVector.size(), "Something wrong in serializing the model.");
    txtModel.close();
    return true;
}

bool SparseLinearModel::Deserialize(istream& stream) {
    m_WeightVector.clear();
    m_FeatureToUid.clear();
    m_WeightIndex.clear();

    uint32_t bytesToRead = 0;
    stream.read((char*)&bytesToRead, sizeof(uint32_t));

    char header[HEADER_SIZE];
    stream.read(header, HEADER_SIZE);
    if (strcmp(header, V1_MAGIC_WORD) != 0) {
        LogAssert(false, "Model file doesn't match with LCCRF V1 format.");
    }

    stream.read((char*)&m_MaxLabelState, sizeof(uint32_t));

    uint32_t nFeatureSets = 0;
    stream.read((char*)&nFeatureSets, sizeof(uint32_t));
    m_FeatureToUid.reserve(nFeatureSets);

    for (uint32_t featureSetIter = 0; featureSetIter < nFeatureSets; ++featureSetIter) {
        unordered_map<uint32_t, size_t> mlgFeatureIdToUid;

        uint32_t featureSetId = UINT32_MAX;
        uint32_t numFeatures = 0;

        stream.read((char*)&featureSetId, sizeof(uint32_t));
        stream.read((char*)&numFeatures, sizeof(uint32_t));
        mlgFeatureIdToUid.reserve(numFeatures);

        LogAssert(featureSetId == featureSetIter, "Model doesn't have all lists of feature sets. Please verify it.");

        for (uint32_t featureIter = 0; featureIter < numFeatures; ++featureIter) {
            uint32_t featureId = UINT32_MAX;
            uint16_t numAttribute = 0;
            stream.read((char*)&featureId, sizeof(uint32_t));
            stream.read((char*)&numAttribute, sizeof(uint16_t));

            unordered_map<uint16_t, size_t> attributes;
            attributes.reserve(numAttribute);

            for (uint16_t attributeIter = 0; attributeIter < numAttribute; ++attributeIter) {
                uint16_t labelId = UINT16_MAX;
                float weight = 0.0f;

                stream.read((char*)&labelId, sizeof(uint16_t));
                stream.read((char*)&weight, sizeof(float));

                LogAssert(labelId != UINT16_MAX, "");
                LogAssert(weight != 0.0f && !std::isnan(weight), "");

                size_t paramId = m_WeightVector.size();
                m_WeightVector.push_back(weight);

                auto ret = attributes.insert(make_pair(labelId, paramId));
                LogAssert(ret.second, "");
            }

            size_t uid = m_WeightIndex.size();
            m_WeightIndex.push_back(std::move(attributes));
            auto ret = mlgFeatureIdToUid.insert(make_pair(featureId, uid));
            LogAssert(ret.second, "Found duplicate in feature Id. Maybe due to corrupted model file.");
        }
        m_FeatureToUid.push_back(std::move(mlgFeatureIdToUid));
    }

    uint32_t bytesRead = (uint32_t)stream.tellg();
    LogAssert(bytesRead - sizeof(uint32_t) == bytesToRead, "Invalid model file.");

    RefreshCache();
    CreateParameterVectorIndex();

    return true;
}

bool SparseLinearModel::Deserialize(const std::string& modelFile) {
    ifstream stream(modelFile, ios::in | ios::binary);
    return Deserialize(stream);
}

void SparseLinearModel::RefreshCache() {
    // Create the state-state transition cache
    m_TrainsitionCache.reset(new float[m_MaxLabelState * m_MaxLabelState]);
    memset(m_TrainsitionCache.get(), 0, m_MaxLabelState * m_MaxLabelState * sizeof(float));

    for (int i = 0; i < m_MaxLabelState; ++i) {
        size_t id = FindFeautureIdMap(EDGE_FEATURE_SET, i);
        if (id != INDEX_NOT_FOUND) {
            for (const auto& iter : FindWeightIds(id)) {
                m_TrainsitionCache[i * m_MaxLabelState + iter.first] = m_WeightVector[iter.second];
            }
        }
    }
}

size_t SparseLinearModel::Shrink(float truncation) {
    vector<unordered_map<uint32_t, size_t>> featureToUid;
    vector<unordered_map<uint16_t, size_t>> weightIndex;
    vector<float> weightVector;

    featureToUid.reserve(m_FeatureToUid.size());
    for (int idx = 0; idx < m_FeatureToUid.size(); ++idx) {
        unordered_map<uint32_t, size_t> paramMap;
        featureToUid.push_back(std::move(paramMap));
    }

    for (uint32_t setId = 0; setId < m_FeatureToUid.size(); ++setId) {
        for (const auto& featureIndex : m_FeatureToUid[setId]) {
            uint32_t featId = featureIndex.first;
            size_t uid = featureIndex.second;
            for (const auto& labelIndex : m_WeightIndex[uid]) {
                uint16_t labId = labelIndex.first;
                LogAssert(labelIndex.second < m_WeightVector.size(), "labelIndex.second < m_WeightVector.size()");
                if (abs(m_WeightVector[labelIndex.second]) > truncation) {  // active parameter
                    size_t newParamId = weightVector.size();
                    weightVector.push_back(m_WeightVector[labelIndex.second]);
                    PushIndex(featureToUid[setId], weightIndex, featId, labId, newParamId);
                }
            }
        }
    }

    m_WeightVector = std::move(weightVector);
    m_FeatureToUid = std::move(featureToUid);
    m_WeightIndex = std::move(weightIndex);

    return m_WeightVector.size();
}

void SparseLinearModel::InsertParameter(uint32_t setId, uint32_t featId, uint16_t labId) {
    bool weightExists = false;
    size_t uid = FindFeautureIdMap(setId, featId);
    if (uid != INDEX_NOT_FOUND) {
        const auto& iter = m_WeightIndex[uid].find(labId);
        if (iter != m_WeightIndex[uid].end()) {
            weightExists = true;
        }
    }

    if (m_FeatureToUid.size() <= setId) {
        for (size_t i = m_FeatureToUid.size(); i <= setId; ++i) {
            unordered_map<uint32_t, size_t> newFeatureIndex;
            m_FeatureToUid.push_back(std::move(newFeatureIndex));
        }
    }

    if (!weightExists) {
        size_t newParamId = m_WeightVector.size();
        m_WeightVector.push_back(0.0f);
        PushIndex(m_FeatureToUid[setId], m_WeightIndex, featId, labId, newParamId);
    }
}

size_t SparseLinearModel::Expand(const std::vector<MLGFeatureSentence>& data) {
    // Note: m_MaxLabelState is defined by max(label ids) + 1
    uint16_t maxLabelId = m_MaxLabelState;

    for (const MLGFeatureSentence& sentence : data) {
        uint16_t prevLabId = 0;
        for (int timeIdx = 0; timeIdx < sentence.Size(); ++timeIdx) {
            const auto& word = sentence.GetWord(timeIdx);
            uint16_t labId = word.GetLabel();

            for (int featIdx = 0; featIdx < word.Size(); ++featIdx) {
                const auto setId = std::get<0>(word.GetFeature(featIdx));
                const auto featId = std::get<1>(word.GetFeature(featIdx));
                InsertParameter(setId, featId, labId);
            }

            // state
            if (timeIdx > 0) {
                InsertParameter(EDGE_FEATURE_SET, prevLabId, labId);
            }
            prevLabId = labId;

            if (maxLabelId < labId) {
                maxLabelId = labId;
            }
        }
    }

    m_MaxLabelState = maxLabelId + 1;
    RefreshCache();
    CreateParameterVectorIndex();

    return m_WeightVector.size();
}

void SparseLinearModel::CreateParameterVectorIndex() {
    m_ParameterVectorIndex.clear();
    for (const auto& paramIndex : m_WeightIndex) {
        vector<pair<uint16_t, size_t>> index;
        for (const auto& param : paramIndex) {
            index.push_back(param);
        }
        m_ParameterVectorIndex.push_back(index);
    }
}

size_t SparseLinearModel::FindFeautureIdMap(uint32_t setId, uint32_t featId) {
    if (m_FeatureToUid.size() > setId) {
        const auto& featureMap = m_FeatureToUid[setId];
        const auto& iter = featureMap.find(featId);
        if (iter != featureMap.end()) {
            return iter->second;
        }
    }
    return INDEX_NOT_FOUND;
}

void SparseLinearModel::BackPropagateTransitionWeight() {
    size_t N = MaxLabel();
    for (uint16_t outgoing = 0; outgoing < N; ++outgoing) {
        float* trans = &m_TrainsitionCache[outgoing * N];
        size_t uid = FindFeautureIdMap(EDGE_FEATURE_SET, outgoing);
        if (uid == INDEX_NOT_FOUND) continue;
        for (const auto& iter : FindWeightIds(uid)) {
            m_WeightVector[iter.second] = trans[iter.first];
        }
    }
}

}  // namespace SparseLinearChainCRF