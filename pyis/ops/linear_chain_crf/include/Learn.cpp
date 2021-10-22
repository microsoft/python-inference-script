// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "Learn.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "CRFFactory.h"
#include "ConsoleUtils.h"
#include "SupervisedLearnerFactory.h"
#include "src/SparseLinearModel.h"
#include "src/StreamDataManager.h"
#include "src/StringUtils.h"

namespace SparseLinearChainCRFConsole {
using namespace std;

namespace {

void CheckModelFileArgs(std::map<std::string, std::string>& vm, string& modelFile) {
    modelFile = vm["model"];
    bool force_overwrite = std::stoi(vm["force"]) > 0 ? true : false;
    if (CheckFileExists(modelFile) && !force_overwrite) {
        LogAssert(false, "Model file exists. If you want force overwritting, please use -f option.");
    }
}

void CheckPremodelArgs(std::map<std::string, std::string>& vm, string& premodelFile, bool& resetPremodel,
                       bool& premodel_expand) {
    if (vm["premodel.file"].length() != 0) {
        premodelFile = vm["premodel.file"];
        LogAssert(CheckFileExists(premodelFile), "Premodel file does not exist.");

        // below options should be combined with premodel.file
        resetPremodel = std::stoi(vm["premodel.reset"]) > 0 ? true : false;
        premodel_expand = std::stoi(vm["premodel.expand"]) > 0 ? true : false;
    }
}
}  // namespace

Learn::Learn() {
    m_OptionDesc["algo"] = "perc";           // "string, learning algorithm"
    m_OptionDesc["crf"] = "vanilla";         // "string, crf algorithm"
    m_OptionDesc["model"] = "";              // "string, model file"
    m_OptionDesc["train"] = "";              // "list[string], training data file(s)"
    m_OptionDesc["dev"] = "";                // "list[string], development data file(s)"
    m_OptionDesc["stream.size"] = "200000";  // "int, stream size (#data) for a chunk"
    m_OptionDesc["stream.temp"] = "tmp";     // "string, working temporary folder for binary dump files)"
    m_OptionDesc["shuffle"] = "1";           // "bool, shuffle data at each iteration"
    m_OptionDesc["force"] = "0";             // "string, force to overwrite model"
    m_OptionDesc["verbose"] = "0";           // "string, show verbose"
    m_OptionDesc["help"] = "0";              // "string, show help message"
    m_OptionDesc["premodel.file"] = "";      // "string, premodel file"
    m_OptionDesc["premodel.reset"] = "0";    // "bool, enable to reset all the values in premodel"
    m_OptionDesc["premodel.expand"] = "1";   // "bool, enable to expand parameter index/vector by data"
}

void Learn::Run(std::map<std::string, std::string>& options) {
    chrono::time_point<chrono::system_clock> stopwatch, stopwatchForEntireTraining;
    stopwatch = chrono::system_clock::now();
    stopwatchForEntireTraining = chrono::system_clock::now();

    std::map<std::string, std::string> vm = ConsoleUtils::ParseArgs(options, m_OptionDesc, true);

    string modelFile, premodelFile;
    bool resetPremodel, expandParameterSpace = true;

    CheckModelFileArgs(vm, modelFile);
    CheckPremodelArgs(vm, premodelFile, resetPremodel, expandParameterSpace);

    // Create CRF algo and Learner
    shared_ptr<SparseLinearModel> model(new SparseLinearModel());
    CRFFactory crfFactory;
    shared_ptr<ILinearChainCRF> crf;
    crf = crfFactory.Create(vm["crf"].c_str());

    // Load premodel
    if (premodelFile != "") {
        model->Deserialize(premodelFile);
        cout << "# Premodel parameter size: " << model->Size() << endl;
        if (resetPremodel) {
            model->Reset();
        }
    }
    chrono::duration<double> preparationTime = chrono::system_clock::now() - stopwatch;
    stopwatch = chrono::system_clock::now();

    shared_ptr<StreamDataManager> trainData(new StreamDataManager(vm["train"], model, vm["stream.temp"],
                                                                  std::stoi(vm["stream.size"]), expandParameterSpace,
                                                                  StringUtils::ToBool(vm["shuffle"])));
    cout << "# Training data size: " << trainData->Size() << endl;
    shared_ptr<StreamDataManager> devData = nullptr;
    if (vm["dev"].length() != 0) {
        devData = shared_ptr<StreamDataManager>(
            new StreamDataManager(vm["dev"], model, vm["stream.temp"], std::stoi(vm["stream.size"]), false));
        cout << "# Development data size: " << devData->Size() << endl;
    }

    chrono::duration<double> loadingDataTime = chrono::system_clock::now() - stopwatch;
    stopwatch = chrono::system_clock::now();

    cout << "# Parameter size: " << model->Size() << endl;

    SupervisedLearnerFactory learnerFactory;
    shared_ptr<BaseSupervisedLearner> learner;
    learner = learnerFactory.Create(vm["algo"].c_str());
    learner->GetProgramOption().insert(m_OptionDesc.begin(), m_OptionDesc.end());
    std::map<std::string, std::string> algoOptions = ConsoleUtils::ParseArgs(options, learner->GetProgramOption());
    crf->Initialize(model);
    learner->Initialize(crf, model, algoOptions);

    cout << "# Learning algorithm: " << vm["algo"] << endl;

    learner->Learn(trainData, devData);
    chrono::duration<double> learningTime = chrono::system_clock::now() - stopwatch;
    stopwatch = chrono::system_clock::now();

    // Finialize/Serialize
    model->Shrink();

    std::ofstream modelStream(modelFile, std::ios::binary);
    model->Serialize(modelStream);
    chrono::duration<double> serializingTime = chrono::system_clock::now() - stopwatch;
    chrono::duration<double> trainingTime = chrono::system_clock::now() - stopwatchForEntireTraining;

    cout << endl;
    cout << "# Training time (sec): " << trainingTime.count() << endl;
    cout << "    Model initialization time (sec): " << preparationTime.count() << endl;
    cout << "    Data loading time (sec): " << loadingDataTime.count() << endl;
    cout << "    Model learning time (sec): " << learningTime.count() << endl;
    cout << "    Model serialization time (sec): " << serializingTime.count() << endl;
    cout << "# Number of queries used for training: " << trainData->Size() << endl;
    cout << "# Number of parameter saved: " << model->Size() << endl;
}
}  // namespace SparseLinearChainCRFConsole
