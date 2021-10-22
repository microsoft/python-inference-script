// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "BaseSupervisedLearner.h"

#include <algorithm>
#include <cfloat>
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

#include "VectorUtils.h"

namespace SparseLinearChainCRF {
using namespace std;

namespace {
void AddParameter(float* lhs, const float* rhs, size_t size) {
    for (int i = 0; i < size; ++i) {
        lhs[i] += rhs[i];
    }
}

void DivideParameter(float* vec, float val, size_t size) {
    for (int i = 0; i < size; ++i) {
        vec[i] /= val;
    }
}

void SubstractModel(float* lhs, const float* rhs, size_t size) {
    for (int i = 0; i < size; ++i) {
        lhs[i] -= rhs[i];
    }
}

float NextLearningRate(const string& method, float curRate, float initValue, size_t iter, size_t numData) {
    LogAssert(numData > 0, "No data used for training.");
    float nextRate = curRate;

    if (method == "exp") {
        nextRate *= pow(initValue, 1.0f * static_cast<float>(iter) / numData);
    } else if (method == "linear") {
        nextRate /= (1.0f + static_cast<float>(iter) / numData);
    } else if (method == "abs") {
        nextRate *= initValue;
    }

    return nextRate;
}
}  // namespace

BaseSupervisedLearner::BaseSupervisedLearner() {
    m_OptionDesc["iter.max"] = "100";           // "int, max iteration"
    m_OptionDesc["iter.earlystop"] = "0.0001";  // "float, early stopping diff: 0 indicates no creteria"
    m_OptionDesc["iter.save"] = "0";            // "bool, save interim models during iterations"
    m_OptionDesc["iter.avg"] = "0";             // "bool, use parameter averaging"
    m_OptionDesc["iter.warm"] = "0";            // "int, warm start period not to stop training early"

    m_OptionDesc["online.stepsize"] = "0.1";   // "int, step-size of update"
    m_OptionDesc["online.decay"] = "exp";      // "string, decaying method of learning rate (abs, linear, exp)"
    m_OptionDesc["online.decayinit"] = "0.7";  // "float, initial value of decaying"
    m_OptionDesc["random.seed"] = "0";         // "int, random generator seed"
}

void BaseSupervisedLearner::Initialize(shared_ptr<ILinearChainCRF> CRF, shared_ptr<SparseLinearModel> linearModel,
                                       std::map<std::string, std::string>& options) {
    m_CRF = CRF;
    m_LinearModel = linearModel;
    m_Options = options;

    m_Verbose = std::stoi(m_Options["verbose"]) > 0 ? true : false;
    m_RandomSeed = std::stoi(m_Options["random.seed"]);
}

void BaseSupervisedLearner::Learn(shared_ptr<StreamDataManager> trainData, shared_ptr<StreamDataManager> devData) {
    // Options
    float stepsize = std::stof(m_Options["online.stepsize"]);
    string decayMethod = m_Options["online.decay"];
    float initialDecayValue = std::stof(m_Options["online.decayinit"]);
    bool useParameterAveraging = std::stoi(m_Options["iter.avg"]) > 0 ? true : false;
    int maxIter = std::stoi(m_Options["iter.max"]);
    int warmIter = std::stoi(m_Options["iter.warm"]);
    float ealryStopThreshold = std::stof(m_Options["iter.earlystop"]);

    size_t M = trainData->Size();
    vector<float> aggregatedModel(m_LinearModel->Size());
    memcpy(aggregatedModel.data(), m_LinearModel->WeightVector().data(), sizeof(float) * m_LinearModel->Size());

    vector<float> rollbackModel(m_LinearModel->Size());
    vector<float> highestModel(m_LinearModel->Size());
    double highestLoss = -DBL_MAX;
    int highestIteration = 0;

    int iter = 1;
    float learningRate = stepsize;
    double prevLoss = 0.0;
    int nConvergency = 0;

    for (; iter <= maxIter; ++iter) {
        chrono::time_point<chrono::system_clock> stopwatch;
        stopwatch = chrono::system_clock::now();

        learningRate = NextLearningRate(decayMethod, learningRate, initialDecayValue, iter - 1, M);
        trainData->Flush();
        double ontheflyLoss = SinglePassTraining(trainData, devData, learningRate);

        if (useParameterAveraging && warmIter < iter) {
            AddParameter(aggregatedModel.data(), m_LinearModel->WeightVector().data(), m_LinearModel->Size());
        }

        float lossDiff = static_cast<float>((prevLoss - ontheflyLoss) / ontheflyLoss);
        if (warmIter < iter && (lossDiff < ealryStopThreshold || std::isnan(lossDiff) || !std::isfinite(lossDiff))) {
            nConvergency++;
            AddParameter(rollbackModel.data(), m_LinearModel->WeightVector().data(), m_LinearModel->Size());
        } else {
            nConvergency = 0;
            memset(rollbackModel.data(), 0, sizeof(float) * m_LinearModel->Size());
            memcpy(highestModel.data(), m_LinearModel->WeightVector().data(), sizeof(float) * m_LinearModel->Size());
            highestLoss = ontheflyLoss;
            highestIteration = iter;
        }
        prevLoss = ontheflyLoss;

        if (m_Verbose) {
            chrono::duration<double> elapsedTime = chrono::system_clock::now() - stopwatch;
            cout << "Iteration = " << std::setw(3) << std::left << iter << " | ";
            cout << "time = " << std::setw(10) << elapsedTime.count() << " | ";
            cout << "loss = " << std::setw(8) << ontheflyLoss << " | ";
            cout << "delta = " << std::setw(12) << lossDiff << " | ";
            cout << "learning rate = " << learningRate;
            cout << endl;
        }

        // early stop
        if (nConvergency > 2) {
            cout << endl;
            cout << "delta = " << std::setw(12) << lossDiff << " | ";
            cout << "early stop threshold = " << ealryStopThreshold;
            cout << endl;
            cout << "Training algorithm terminated with early stopping" << endl;
            cout << "Rolling back to the latest improvement point at iteration " << highestIteration << endl;
            break;
        }
    }

    if (useParameterAveraging) {
        if (highestIteration != iter) {
            SubstractModel(aggregatedModel.data(), rollbackModel.data(), m_LinearModel->Size());
        }
        DivideParameter(aggregatedModel.data(), static_cast<float>(highestIteration - warmIter), m_LinearModel->Size());
        m_LinearModel->Reset(aggregatedModel);
    } else if (highestIteration != iter) {
        m_LinearModel->Reset(highestModel);
    } else {
        LogAssert(true, "Null model parameter");
    }
}

}  // namespace SparseLinearChainCRF