// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "LBFGSLearner.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "LBFGSSolver.h"
#include "VectorUtils.h"

namespace SparseLinearChainCRF {
using namespace std;

namespace {
void UpdateNodeGradient(const Word<IndexedParameterType>& word, const float* prob,
                        shared_ptr<SparseLinearModel> linearModel, vector<float>& gradient) {
    LogAssert(linearModel != nullptr, "Model is null");
    for (const IndexedParameterType& feature : word.Features()) {
        size_t id = feature.first;
        float val = feature.second;
        for (const auto& parameterId : linearModel->FindParameterVector(id)) {
            uint16_t y = parameterId.first;
            size_t fid = parameterId.second;
            float delta = (y == word.GetLabel()) ? (prob[y] - 1.0f) : prob[y];
            gradient[fid] += delta * val;
        }
    }
}

void UpdateEdgeGradientOverSentence(const float* empiricalCount, const float* prob,
                                    shared_ptr<SparseLinearModel> linearModel, vector<float>& gradient) {
    LogAssert(linearModel != nullptr, "Model is null");
    uint16_t N = linearModel->MaxLabel();
    for (uint16_t outgoing = 0; outgoing < N; ++outgoing) {
        const float* expectedProb = &prob[outgoing * N];
        const float* empiricalProb = &empiricalCount[outgoing * N];
        size_t id = linearModel->FindFeautureIdMap(EDGE_FEATURE_SET, outgoing);
        if (id != INDEX_NOT_FOUND) {
            for (const auto& parameterId : linearModel->FindParameterVector(id)) {
                uint16_t y = parameterId.first;
                size_t fid = parameterId.second;
                float delta = expectedProb[y] - empiricalProb[y];
                gradient[fid] += delta;
            }
        }
    }
}
}  // namespace

LBFGSLearner::LBFGSLearner() {
    m_OptionDesc["l1"] = "0.0";  // "float, L1 regularizer parameter"
    m_OptionDesc["l2"] = "0.0";  // "float, L2 regularizer parameter"
}

void LBFGSLearner::Initialize(shared_ptr<ILinearChainCRF> crf, shared_ptr<SparseLinearModel> model,
                              std::map<std::string, std::string>& options) {
    BaseSupervisedLearner::Initialize(crf, model, options);
    m_L1Penalty = std::stof(m_Options["l1"]);
    m_L2Penalty = std::stof(m_Options["l2"]);
}

double LBFGSLearner::SinglePassTraining(shared_ptr<StreamDataManager> trainData, shared_ptr<StreamDataManager> devData,
                                        float learningRate) {
    // UNREFERENCED_PARAMETER(devData);
    // UNREFERENCED_PARAMETER(learningRate); // no need for batch algorithm

    LBFGS lbfgsSolver;
    vector<float> gradientVector(m_LinearModel->Size());

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
            ontheflyLL -= logli;

            std::vector<float> empiricalCountOfTransition(N * N, 0.0f);
            memset(empiricalCountOfTransition.data(), 0, sizeof(float) * N * N);
            uint16_t prevLabel = 0;
            for (int t = 0; t < T; ++t) {
                const auto& word = sentence.GetWord(t);
                uint16_t label = word.GetLabel();
                UpdateNodeGradient(word, &probNode[t * N], m_LinearModel, gradientVector);

                if (t > 0) {
                    empiricalCountOfTransition[prevLabel * N + label] += 1.0f;
                }
                prevLabel = label;
            }
            UpdateEdgeGradientOverSentence(empiricalCountOfTransition.data(), probEdge.data(), m_LinearModel,
                                           gradientVector);
        }
    }

    float* weight = m_LinearModel->WeightVector().data();
    if (m_L1Penalty != 0.0f) {
        double weightNorm = 0.0;
        for (const auto& w : m_LinearModel->WeightVector()) {
            weightNorm += std::abs(w);
        }
        weightNorm /= m_L1Penalty;
        ontheflyLL += weightNorm;
    } else if (m_L2Penalty != 0.0f) {
        for (size_t i = 0; i < m_LinearModel->Size(); ++i) {
            gradientVector[i] += weight[i] / m_L2Penalty;
            ontheflyLL += std::pow(weight[i], 2) / (2.0 * m_L2Penalty);
        }
    }

    // LBFGS solver
    int returnCode =
        lbfgsSolver.optimize(m_LinearModel->Size(), weight, (float)ontheflyLL, gradientVector.data(),
                             (m_L1Penalty == 0.0f ? false : true), (m_L1Penalty == 0.0f ? m_L2Penalty : m_L1Penalty));
    LogAssert(returnCode >= 0, "");

    // NOTE: make sure that transition cache is re-computed by the following method.
    m_LinearModel->RefreshCache();

    return ontheflyLL;
}
}  // namespace SparseLinearChainCRF