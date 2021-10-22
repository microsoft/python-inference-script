// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "TagConstrainedCRF.h"

#include <cfloat>
#include <numeric>
#include <vector>

#include "Common.h"
#include "VectorUtils.h"

namespace SparseLinearChainCRF {
using namespace std;

namespace {
unordered_set<uint16_t> ConvertVectorToSet(const vector<uint16_t>& activeTagset) {
    unordered_set<uint16_t> set;
    for (const auto& tag : activeTagset) {
        set.insert(tag);
    }
    return set;
}
}  // namespace

void TagConstrainedCRF::CreateConstrainedLinearFunctionCache(const IndexedSentence& sentence,
                                                             const unordered_set<uint16_t>& activeTagsetMap,
                                                             float* linearFunctionCache) const {
    // Initialize
    const auto& weight = m_LinearModel->WeightVector();
    uint16_t N = m_LinearModel->MaxLabel();
    memset(linearFunctionCache, 0, sentence.Size() * N * sizeof(float));

    for (uint16_t timeIdx = 0; timeIdx < sentence.Size(); ++timeIdx) {
        float* column = &linearFunctionCache[timeIdx * N];
        const auto& word = sentence.GetWord(timeIdx);

        for (const auto& feature : word.Features()) {
            for (const auto& param : m_LinearModel->FindParameterVector(feature.first)) {
                if (activeTagsetMap.find(param.first) != activeTagsetMap.end()) {
                    column[param.first] += weight[param.second] * feature.second;
                }
            }
        }
    }
}

double TagConstrainedCRF::Infer(const IndexedSentence& sentence, float* probNode, float* probEdge) const {
    uint16_t T = (uint16_t)sentence.Size();
    uint16_t N = m_LinearModel->MaxLabel();
    size_t obsMatrixSize = T * N;
    size_t transMatrixSize = N * N;
    const auto& activeTagset = sentence.GetActiveTagset();
    // auto activeTagsetMap = ConvertVectorToSet(activeTagset);
    // cout << activeTagsetMap.size() << endl;

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
    ConstrainedForward(expNode.data(), expEdge.data(), T, N, activeTagset, alpha.data(), scaler.data());
    double logNorm = -VectorSumLog(scaler.data(), T);

    std::vector<double> beta(obsMatrixSize, 0.0f);
    ConstrainedBackward(expNode.data(), expEdge.data(), T, N, activeTagset, scaler.data(), beta.data());

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
            for (uint16_t incoming = 0; incoming < N; ++incoming) {
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

void TagConstrainedCRF::Decode(const IndexedSentence& sentence, vector<uint16_t>& tags) const {
    uint16_t T = (uint16_t)sentence.Size();
    uint16_t N = m_LinearModel->MaxLabel();
    const auto& activeTagset = sentence.GetActiveTagset();
    auto activeTagsetMap = ConvertVectorToSet(activeTagset);

    std::vector<float> linearFunctionCache(T * N, 0.0f);
    CreateConstrainedLinearFunctionCache(sentence, activeTagsetMap, linearFunctionCache.data());

    std::vector<uint16_t> bestpath(T, 0.0f);
    if (activeTagset.size() > 0 && activeTagset.size() < N) {
        ConstrainedViterbi1Best(linearFunctionCache.data(), T, activeTagset, bestpath.data());
    } else {
        Viterbi1Best(linearFunctionCache.data(), T, bestpath.data());
    }

    // output
    // TODO: tags should be switched to auto_buffer
    tags.clear();
    tags.reserve(T);
    for (int t = 0; t < T; ++t) {
        tags.push_back(bestpath[t]);
    }
}

float TagConstrainedCRF::ConstrainedViterbi1Best(const float* linearFunctionCache, int wordCount,
                                                 const std::vector<uint16_t>& activeTagset,
                                                 uint16_t* bestPathIds) const {
    int T = wordCount;
    uint16_t N = m_LinearModel->MaxLabel();
    const float* transitionCache = m_LinearModel->GetTransitionCache();

    // initialize
    std::vector<float> viterbiScore(T * N, 0.0f);
    memcpy(viterbiScore.data(), linearFunctionCache, N * sizeof(float));

    std::vector<uint16_t> backtracer(T * N, 0.0f);
    memset(backtracer.data(), UCHAR_MAX, N * sizeof(uint16_t));

    // dynamic programming
    for (uint16_t t = 1; t < T; ++t) {
        float* prevcolumn = &viterbiScore[(t - 1) * N];
        float* curScores = &viterbiScore[t * N];
        const float* curWeightedSum = &linearFunctionCache[t * N];
        uint16_t* curBackTrace = &backtracer[t * N];

        for (const auto& i : activeTagset) {
            float maxScore = -FLT_MAX;
            uint16_t backTracer = UINT16_MAX;

            for (const auto& j : activeTagset) {
                float score = prevcolumn[j] + transitionCache[j * N + i];
                if (maxScore < score) {
                    maxScore = score;
                    backTracer = j;
                }
            }

            curScores[i] = maxScore + curWeightedSum[i];
            curBackTrace[i] = backTracer;
        }
    }

    // backtrace
    float bestPathScore = -FLT_MAX;
    float* lastColumnScores = &viterbiScore[(T - 1) * N];
    for (const auto& i : activeTagset) {
        float score = lastColumnScores[i];
        if (bestPathScore < score) {
            bestPathScore = score;
            bestPathIds[T - 1] = i;
        }
    }

    for (int t = T - 2; t >= 0; --t) {
        LogAssert(bestPathIds[t + 1] < N, "bestPathIds[t+1] < N");
        bestPathIds[t] = backtracer[(t + 1) * N + bestPathIds[t + 1]];
    }

    return bestPathScore;
}

void TagConstrainedCRF::ConstrainedForward(const double* nodeMatrix, const double* edgeMatrix, uint16_t T, uint16_t N,
                                           const vector<uint16_t>& activeTagset, double* alpha, double* scales) const {
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

        for (const auto& outgoing : activeTagset) {
            const double* trans = &edgeMatrix[outgoing * N];
            SparseVectorAdd(cur, trans, prev[outgoing], activeTagset);
        }
        VectorMultiply(cur, obs, N);

        sum = VectorSum(cur, N);
        scales[t] = (sum != 0.0) ? 1.0 / sum : 1.0;
        VectorScale(cur, scales[t], N);
    }
}

void TagConstrainedCRF::ConstrainedBackward(const double* nodeMatrix, const double* edgeMatrix, uint16_t T, uint16_t N,
                                            const vector<uint16_t>& activeTagset, const double* scales,
                                            double* beta) const {
    VectorSet(&beta[(T - 1) * N], scales[T - 1], N);
    std::vector<double> column(N, 0.0f);

    // recursion
    for (int t = T - 2; t >= 0; --t) {
        double* cur = &beta[t * N];
        const double* next = &beta[(t + 1) * N];
        const double* obs = &nodeMatrix[(t + 1) * N];

        VectorCopy(column.data(), next, N);
        VectorMultiply(column.data(), obs, N);

        for (const auto& outgoing : activeTagset) {
            const double* trans = &edgeMatrix[outgoing * N];
            cur[outgoing] = SparseVectorDotProduct(column.data(), trans, activeTagset);
        }
        VectorScale(cur, scales[t], N);
    }
}
}  // namespace SparseLinearChainCRF
