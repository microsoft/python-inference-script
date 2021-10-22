// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "Decode.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "CRFFactory.h"
#include "ConsoleUtils.h"
#include "src/ILinearChainCRF.h"
#include "src/SparseLinearModel.h"
#include "src/StreamDataManager.h"
#include "src/StringUtils.h"

namespace SparseLinearChainCRFConsole {
using namespace SparseLinearChainCRF;
using namespace std;

namespace {

void CheckModelFilePresence(std::map<std::string, std::string>& vm, string& modelFile) {
    modelFile = vm["model"];
    LogAssert(CheckFileExists(modelFile), "Model file does not exist");
}

void CheckInputAndOutput(std::map<std::string, std::string>& vm, string& inputFile, ofstream& ofs) {
    inputFile = vm["input"];
    LogAssert(CheckFileExists(inputFile), "Input file does not exist");

    if (vm["output"].length() != 0) {
        string output_file = vm["output"];
        ofs.open(output_file.c_str(), std::ios_base::out);
        bool force_overwrite = std::stoi(vm["force"]) > 0 ? true : false;
        if (CheckFileExists(output_file) && !force_overwrite) {
            LogAssert(false, "Output file exists. If you want force overwritting, please use -f option.");
        }
    }
}

void CheckReferenceArgs(std::map<std::string, std::string>& vm, unordered_map<uint16_t, string>& labelMap,
                        bool& showReference) {
    showReference = std::stoi(vm["reference"]) > 0 ? true : false;

    labelMap.clear();
    if (vm["label"].length() != 0) {
        ifstream in(vm["label"]);
        string line;
        while (getline(in, line)) {
            vector<string> tokens = StringUtils::Split(line, " ");
            LogAssert(tokens.size() >= 2, "Invalid line in label mapping file: %s", line.c_str());
            labelMap.insert(std::make_pair(static_cast<uint16_t>(std::stoi(tokens[0])), tokens[1]));
        }
        in.close();
    }
}

string LabelIdToString(uint16_t id, const std::unordered_map<uint16_t, string>& labelMap) {
    if (labelMap.size() > 0) {
        const auto& iter = labelMap.find(id);
        if (iter != labelMap.end()) {
            return iter->second;

        } else {
            return std::to_string(id);
        }
    } else {
        return std::to_string(id);
    }
}

void BatchDecode(const shared_ptr<ILinearChainCRF> crf, shared_ptr<StreamDataManager> data,
                 unordered_map<uint16_t, string>& labelMap, bool showReference, ostream& os) {
    data->Flush();
    while (!data->Empty()) {
        auto const& set = data->Next();
        for (auto const& sentence : set) {
            vector<uint16_t> predicted_tags;
            crf->Decode(sentence, predicted_tags);

            for (int t = 0; t < predicted_tags.size(); ++t) {
                if (showReference) {
                    os << LabelIdToString(sentence.GetWord(t).GetLabel(), labelMap) << "\t";
                }
                os << LabelIdToString(predicted_tags[t], labelMap) << endl;
            }
            os << endl;
        }
    }
}

}  // namespace

Decode::Decode() {
    m_OptionDesc["crf"] = "vanilla";  // "string, crf algorithm"
    m_OptionDesc["model"] = "";       // "string, model file"
    m_OptionDesc["input"] = "";       // "string, input file"
    m_OptionDesc["output"] = "";      // "string, output file (optional; default is to output stdout)"
    m_OptionDesc["label"] =
        "";  // "string, "label mapping file (optional; given this option, decoder outputs string-valued labels)"
    m_OptionDesc["reference"] = "0";         // "bool, show labels in data (optional)"
    m_OptionDesc["stream.size"] = "200000";  // "int, stream size (#data) for a chunk"
    m_OptionDesc["stream.temp"] = "tmp";     // "string, working temporary folder for binary dump files)"
    m_OptionDesc["force"] = "0";             // "bool, force to overwrite output file"
}

void Decode::Run(std::map<std::string, std::string>& options) {
    chrono::time_point<chrono::system_clock> stopwatch;
    stopwatch = chrono::system_clock::now();

    std::map<std::string, std::string> vm = ConsoleUtils::ParseArgs(options, m_OptionDesc, true);

    string modelFile, inputFile;
    ofstream ofs;
    bool showReference;
    unordered_map<uint16_t, string> labelMap;

    CheckModelFilePresence(vm, modelFile);
    CheckInputAndOutput(vm, inputFile, ofs);
    CheckReferenceArgs(vm, labelMap, showReference);

    // Initialize/Deserialize model
    shared_ptr<SparseLinearModel> model(new SparseLinearModel(modelFile));

    // Intialize:
    CRFFactory crfFactory;
    shared_ptr<ILinearChainCRF> crf;
    crf = crfFactory.Create(vm["crf"].c_str());
    crf->Initialize(model);
    chrono::duration<double> loadingModelTime = chrono::system_clock::now() - stopwatch;
    stopwatch = chrono::system_clock::now();

    shared_ptr<StreamDataManager> data(
        new StreamDataManager(inputFile, model, vm["stream.temp"], std::stoi(vm["stream.size"]), false));

    chrono::duration<double> loadingDataTime = chrono::system_clock::now() - stopwatch;
    stopwatch = chrono::system_clock::now();

    // Decode
    std::ostream& os = ofs.is_open() ? ofs : cout;
    BatchDecode(crf, data, labelMap, showReference, os);

    // Finalize
    ofs.close();
    chrono::duration<double> decodingTime = chrono::system_clock::now() - stopwatch;

    cout << endl;
    cout << "# Model loading time (sec): " << loadingModelTime.count() << endl;
    cout << "# Data loading time (sec): " << loadingDataTime.count() << endl;
    cout << "# Decoding time (sec): " << decodingTime.count() << endl;
    cout << "# Number of queries decoded: " << data->Size() << endl;
}

}  // namespace SparseLinearChainCRFConsole
