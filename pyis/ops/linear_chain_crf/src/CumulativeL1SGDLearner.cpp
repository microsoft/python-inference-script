// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "CumulativeL1SGDLearner.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

namespace SparseLinearChainCRF {
using namespace std;

namespace {
void AddVec(vector<float>& vec_a, const vector<float>& vec_b) {
    LogAssert(vec_a.size() == vec_b.size(), "vec_a.size() == vec_b.size()");

    for (int i = 0; i < vec_a.size(); ++i) {
        vec_a[i] += vec_b[i];
    }
}

inline void ApplyPenalty(float* weight, float* penalty, size_t fid, float u) {
    // clipping + lazy update
    float z = weight[fid];
    if (z > 0) {
        weight[fid] = max(0.0f, weight[fid] - (u + penalty[fid]));
    } else {
        weight[fid] = min(0.0f, weight[fid] + (u - penalty[fid]));
    }
    penalty[fid] += weight[fid] - z;
}

void UpdateNodeWeight(const Word<IndexedParameterType>& word, const float* prob, float l, float u,
                      shared_ptr<SparseLinearModel> linearModel, vector<float>& penalty) {
    LogAssert(linearModel != nullptr, "Model is null");
    float* weight = linearModel->WeightVector().data();
    for (const IndexedParameterType& feature : word.Features()) {
        for (const auto& parameterId : linearModel->FindParameterVector(feature.first)) {
            uint16_t y = parameterId.first;
            size_t fid = parameterId.second;
            float delta = (y == word.GetLabel()) ? (1.0f - prob[y]) : (-1.0f * prob[y]);
            weight[fid] += l * delta * feature.second;
            ApplyPenalty(weight, penalty.data(), fid, u);
        }
    }
}

void UpdateEdgeWeightOverSentence(const float* empiricalCount, const float* prob, float l, float u,
                                  shared_ptr<SparseLinearModel> linearModel, vector<float>& penalty) {
    LogAssert(linearModel != nullptr, "Model is null");
    uint16_t N = linearModel->MaxLabel();
    float* transitionCache = linearModel->GetTransitionCache();
    for (uint16_t outgoing = 0; outgoing < N; ++outgoing) {
        float* trans = &transitionCache[outgoing * N];
        const float* expectedProb = &prob[outgoing * N];
        const float* empiricalProb = &empiricalCount[outgoing * N];
        for (uint16_t incoming = 0; incoming < N; ++incoming) {
            float delta = empiricalProb[incoming] - expectedProb[incoming];
            trans[incoming] += l * delta;
            size_t fid = outgoing * N + incoming;
            float z = trans[incoming];
            if (z > 0) {
                trans[incoming] = max(0.0f, trans[incoming] - (u + penalty[fid]));
            } else {
                trans[incoming] = min(0.0f, trans[incoming] + (u - penalty[fid]));
            }
            penalty[fid] += trans[incoming] - z;
        }
    }
}
}  // namespace

CumulativeL1SGDLearner::CumulativeL1SGDLearner() {
    m_OptionDesc["sparsity"] = "0.1";  // "float, Sparsity parameter (i.e. L1 penality parameter)"
}

void CumulativeL1SGDLearner::Initialize(shared_ptr<ILinearChainCRF> crf, shared_ptr<SparseLinearModel> model,
                                        std::map<std::string, std::string>& options) {
    BaseSupervisedLearner::Initialize(crf, model, options);
    m_L1Penalty = std::stof(options["sparsity"]);

    m_CumulativeL1PenalizedWeight.resize(m_LinearModel->Size());
    memset(m_CumulativeL1PenalizedWeight.data(), 0, sizeof(float) * m_LinearModel->Size());
    m_CumulativeL1Penalty = 0.0f;

    size_t N = m_LinearModel->MaxLabel();
    m_CumulativeL1PenalizedTransitionWeight.resize(N * N);
    memset(m_CumulativeL1PenalizedTransitionWeight.data(), 0, sizeof(float) * N * N);
}

double CumulativeL1SGDLearner::SinglePassTraining(shared_ptr<StreamDataManager> trainData,
                                                  shared_ptr<StreamDataManager> devData, float learningRate) {
    // UNREFERENCED_PARAMETER(devData);
    m_CumulativeL1Penalty += learningRate * m_L1Penalty;

    uint16_t N = m_LinearModel->MaxLabel();
    double ontheflyLL = 0.0;

    while (!trainData->Empty()) {
        const auto& trainSet = trainData->Next();
        for (const auto& sentence : trainSet) {
            uint16_t T = (uint16_t)sentence.Size();

            std::vector<float> probNode(T * N, 0.0f);
            std::vector<float> probEdge(N * N, 0.0f);

            double logli = m_CRF->Infer(sentence, probNode.data(), probEdge.data());
            LogAssert(!std::isnan(logli), "Log-likelihood is NaN");
            LogAssert(std::isfinite(logli), "Log-likelihood is infinite");
            ontheflyLL += logli;

            std::vector<float> empiricalCountOfTransition(N * N, 0.0f);
            memset(empiricalCountOfTransition.data(), 0, sizeof(float) * N * N);
            uint16_t prevLabel = 0;
            for (int t = 0; t < T; ++t) {
                const auto& word = sentence.GetWord(t);
                uint16_t label = word.GetLabel();

                // Update node parameters
                UpdateNodeWeight(word, &probNode[t * N], learningRate, m_CumulativeL1Penalty, m_LinearModel,
                                 m_CumulativeL1PenalizedWeight);

                // Update edge parameters
                if (t > 0) {
                    empiricalCountOfTransition[prevLabel * N + label] += 1.0;
                }
                prevLabel = label;
            }
            UpdateEdgeWeightOverSentence(empiricalCountOfTransition.data(), probEdge.data(), learningRate,
                                         m_CumulativeL1Penalty, m_LinearModel, m_CumulativeL1PenalizedTransitionWeight);
        }
    }
    m_LinearModel->BackPropagateTransitionWeight();

    double weightNorm = 0.0;
    for (const auto& w : m_LinearModel->WeightVector()) {
        weightNorm += std::abs(w);
    }
    weightNorm *= m_L1Penalty;
    ontheflyLL += weightNorm * m_L1Penalty;

    return -ontheflyLL;
}
}  // namespace SparseLinearChainCRF