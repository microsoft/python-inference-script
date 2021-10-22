// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "FrenchVanillaCRF.h"

#include <numeric>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "VectorUtils.h"

namespace SparseLinearChainCRF {
using namespace std;

void FrenchVanillaCRF::Initialize(shared_ptr<SparseLinearModel> model) {
    m_LinearModel = model;
    uint16_t N = m_LinearModel->MaxLabel();

    int count = 0;
    for (uint16_t i = 0; i < N; ++i) {
        vector<uint16_t> emptyVector;
        m_SparseTransition.push_back(emptyVector);
        size_t id = m_LinearModel->FindFeautureIdMap(EDGE_FEATURE_SET, i);
        if (id != INDEX_NOT_FOUND) {
            for (const auto& iter : m_LinearModel->FindWeightIds(id)) {
                m_SparseTransition[i].push_back(iter.first);
                count += 1;
            }
        }
    }
    cout << count / (double)N << " " << N << endl;
}

double FrenchVanillaCRF::Infer(const IndexedSentence& sentence, float* probNode, float* probEdge) const {
    uint16_t T = (uint16_t)sentence.Size();
    uint16_t N = m_LinearModel->MaxLabel();
    size_t obsMatrixSize = T * N;
    size_t transMatrixSize = N * N;

    std::vector<float> linearFunctionCache(obsMatrixSize, 0.0f);
    CreateLinearFunctionCache(sentence, linearFunctionCache.data());
    std::vector<double> expNode(obsMatrixSize, 0.0f);

    CopyFloatToDoubleVector(linearFunctionCache.data(), expNode.data(), obsMatrixSize);
    ExponentializeVector(expNode.data(), obsMatrixSize);

    const float* transitionCache = m_LinearModel->GetTransitionCache();
    std::vector<double> expEdge(transMatrixSize, 0.0f);

    CopyFloatToDoubleVector(transitionCache, expEdge.data(), transMatrixSize);
    ExponentializeVector(expEdge.data(), transMatrixSize);
    std::vector<double> alpha(obsMatrixSize, 0.0f);
    std::vector<double> scaler(T, 0.0f);
    Forward(expNode.data(), expEdge.data(), T, N, alpha.data(), scaler.data());
    double logNorm = -VectorSumLog(scaler.data(), T);

    std::vector<double> beta(obsMatrixSize, 0.0f);
    Backward(expNode.data(), expEdge.data(), T, N, scaler.data(), beta.data());

    memset(probNode, 0, sizeof(float) * obsMatrixSize);
    memset(probEdge, 0, sizeof(float) * transMatrixSize);

    for (uint16_t t = 0; t < T; ++t) {
        const double* curAlpha = &alpha[t * N];
        const double* curBeta = &beta[t * N];
        float* node = &probNode[t * N];
        for (uint16_t i = 0; i < N; ++i) {
            node[i] = (float)(curAlpha[i] * curBeta[i] / scaler[t]);
        }
    }

    std::vector<double> column(N, 0.0f);
    for (uint16_t t = 0; t < T - 1; ++t) {
        const double* fwd = &alpha[t * N];
        const double* bwd = &beta[(t + 1) * N];
        const double* obs = &expNode[(t + 1) * N];
        VectorCopy(column.data(), bwd, N);
        VectorMultiply(column.data(), obs, N);

        for (uint16_t outgoing = 0; outgoing < N; ++outgoing) {
            const double* trans = &expEdge[outgoing * N];
            float* edge = &probEdge[outgoing * N];
            for (const auto& incoming : m_SparseTransition[outgoing]) {
                edge[incoming] += (float)(fwd[outgoing] * trans[incoming] * column[incoming]);
            }
        }
    }

    double loglikeli = 0.0;
    uint16_t prevTrueLabel = 0;
    for (uint16_t t = 0; t < T; ++t) {
        // likelihood
        const float* obs = &linearFunctionCache[t * N];
        uint16_t trueLabel = sentence.GetWord(t).GetLabel();
        float tranScore = (t > 0) ? transitionCache[prevTrueLabel * N + trueLabel] : 0.0f;
        loglikeli += obs[trueLabel] + tranScore;
        prevTrueLabel = trueLabel;
    }

    return loglikeli - logNorm;
}
void FrenchVanillaCRF::Forward(const double* nodeMatrix, const double* edgeMatrix, uint16_t T, uint16_t N,
                               double* alpha, double* scales) const {
    VectorCopy(alpha, nodeMatrix, N);
    double sum = VectorSum(alpha, N);
    scales[0] = (sum != 0.0) ? 1.0 / sum : 1.0;
    VectorScale(alpha, scales[0], N);

    // recursion
    for (uint16_t t = 1; t < T; ++t) {
        double* prev = &alpha[(t - 1) * N];
        double* cur = &alpha[t * N];
        const double* obs = &nodeMatrix[t * N];
        memset(cur, 0, sizeof(double) * N);

        for (uint16_t outgoing = 0; outgoing < N; ++outgoing) {
            const double* trans = &edgeMatrix[outgoing * N];
            SparseVectorAdd(cur, trans, prev[outgoing], m_SparseTransition[outgoing]);
        }
        SparseVectorMultiply(cur, obs, N);

        sum = VectorSum(cur, N);
        scales[t] = (sum != 0.0) ? 1.0 / sum : 1.0;
        VectorScale(cur, scales[t], N);
    }
}

void FrenchVanillaCRF::Backward(const double* nodeMatrix, const double* edgeMatrix, uint16_t T, uint16_t N,
                                const double* scales, double* beta) const {
    VectorSet(&beta[(T - 1) * N], scales[T - 1], N);
    std::vector<double> column(N, 0.0f);

    // recursion
    for (int t = T - 2; t >= 0; --t) {
        double* cur = &beta[t * N];
        const double* next = &beta[(t + 1) * N];
        const double* obs = &nodeMatrix[(t + 1) * N];

        VectorCopy(column.data(), next, N);
        VectorMultiply(column.data(), obs, N);
        double constant = VectorSum(column.data(), N);

        for (uint16_t outgoing = 0; outgoing < N; ++outgoing) {
            const double* trans = &edgeMatrix[outgoing * N];
            cur[outgoing] = SparseVectorDotProduct(trans, column.data(), m_SparseTransition[outgoing]);
            cur[outgoing] += constant;
        }
        VectorScale(cur, scales[t], N);
    }
}
}  // namespace SparseLinearChainCRF
