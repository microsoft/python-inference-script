// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "PerceptronLearner.h"

#include <vector>

namespace SparseLinearChainCRF {
using namespace std;

namespace {
int Diff(const IndexedSentence& sentence, const vector<uint16_t>& tags) {
    LogAssert(sentence.Size() == tags.size(), "ref tag size != predicted tag size");

    int diff = 0;
    for (int t = 0; t < tags.size(); t++) {
        if (sentence.GetWord(t).GetLabel() != tags[t]) diff++;
    }

    return diff;
}

void UpdateNodeWeight(const Word<IndexedParameterType>& word, uint16_t label, float delta,
                      shared_ptr<SparseLinearModel> linearModel) {
    auto& weight = linearModel->WeightVector();
    for (const IndexedParameterType& feature : word.Features()) {
        size_t id = feature.first;
        float val = feature.second;
        const auto& paramIndex = linearModel->FindWeightIds(id);
        const auto& iter = paramIndex.find(label);
        if (iter != paramIndex.end()) {
            weight[iter->second] += delta * val;
        }
    }
}

void UpdateEdgeWeight(uint16_t label, uint16_t prevLabel, float delta, shared_ptr<SparseLinearModel> linearModel) {
    float* transitionCache = linearModel->GetTransitionCache();
    transitionCache[prevLabel * linearModel->MaxLabel() + label] += delta;
}
}  // namespace

PerceptronLearner::PerceptronLearner() {}

double PerceptronLearner::SinglePassTraining(shared_ptr<StreamDataManager> trainData,
                                             shared_ptr<StreamDataManager> devData, float learningRate) {
    // UNREFERENCED_PARAMETER(devData);

    double loss = 0.0;

    while (!trainData->Empty()) {
        const auto& trainSet = trainData->Next();
        for (const auto& sentence : trainSet) {
            if (sentence.Size() <= 0) {
                continue;
            }

            vector<uint16_t> predictedTags(sentence.Size());
            m_CRF->Decode(sentence, predictedTags);

            int diff = Diff(sentence, predictedTags);
            if (diff > 0) {
                uint16_t prevLabel = 0;
                for (int t = 0; t < sentence.Size(); t++) {
                    const auto& word = sentence.GetWord(t);
                    uint16_t label = word.GetLabel();
                    UpdateNodeWeight(word, label, learningRate, m_LinearModel);
                    UpdateNodeWeight(word, predictedTags[t], -learningRate, m_LinearModel);
                    if (t > 0) {
                        UpdateEdgeWeight(label, prevLabel, learningRate, m_LinearModel);
                        UpdateEdgeWeight(predictedTags[t], predictedTags[t - 1], -learningRate, m_LinearModel);
                    }
                    prevLabel = label;
                }

                loss += (double)diff / sentence.Size();
            }
        }
    }

    m_LinearModel->BackPropagateTransitionWeight();

    return loss;
}
}  // namespace SparseLinearChainCRF