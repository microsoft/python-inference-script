// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "DataFormatter.h"

#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "Common.h"
#include "StringUtils.h"

namespace SparseLinearChainCRF {
using namespace std;
namespace {
vector<uint16_t> ParseTagConstraints(const string& dataline) {
    vector<uint16_t> activeTagset;
    vector<string> tokens = StringUtils::Split(dataline, " \t");
    for (const auto& tok : tokens) {
        if (tok == "@") continue;

        uint16_t tag = static_cast<uint16_t>(std::stoi(tok));
        activeTagset.push_back(tag);
    }
    return activeTagset;
}
}  // namespace

vector<MLGFeatureSentence> DataFormatter::Read(ifstream& fin, int size) {
    vector<MLGFeatureSentence> data;
    string line;
    MLGFeatureSentence sentence;
    int nRead = 0;

    while (nRead < size && getline(fin, line)) {
        if (line.empty()) {
            if (sentence.Size() > 0) {
                data.push_back(sentence);
                sentence.Clear();
                nRead++;
            }
        } else {
            if (line[0] == '#') {
                continue;
            } else if (line[0] == '@') {
                for (const auto& tag : ParseTagConstraints(line)) {
                    sentence.AddActiveTag(tag);
                }
                continue;
            }

            vector<string> tokens = StringUtils::Split(line, " ");
            // LogAssert(tokens.size() > 3);

            uint16_t labelId = static_cast<uint16_t>(std::stoi(tokens[0]));
            float importance = static_cast<float>(std::stof(tokens[1]));
            string text = tokens[2];

            Word<MLGFeatureType> word(labelId, importance, text);

            uint8_t featureSetId = UINT8_MAX;
            for (int i = 3; i < tokens.size(); ++i) {
                if (tokens[i].size() > 0 && tokens[i][0] == '|') {  // FeatureSet Id (i.e. Namespace in VW)
                    string featureSetStr(tokens[i].c_str() + 1);
                    featureSetId = (uint8_t)std::stoi(featureSetStr);
                } else if (featureSetId != UINT8_MAX) {  // Feature Id
                    vector<string> comps = StringUtils::Split(tokens[i], ":");
                    LogAssert(comps.size() > 0, "comps.size() > 0");

                    uint32_t featureId = std::stoi(comps[0]);
                    float featureWeight = 1.0f;
                    if (comps.size() > 1) {
                        featureWeight = std::stof(comps[1]);
                    }

                    word.Append(make_tuple(featureSetId, featureId, featureWeight));
                }
            }

            sentence.Append(word);
        }
    }

    return data;
}

vector<IndexedSentence> DataFormatter::IndexData(shared_ptr<SparseLinearModel> model,
                                                 const vector<MLGFeatureSentence>& mlgData) {
    vector<IndexedSentence> indexData;

    for (const MLGFeatureSentence& mlgSentence : mlgData) {
        IndexedSentence indexSentence;
        for (int t = 0; t < mlgSentence.Size(); ++t) {
            const auto& mlgWord = mlgSentence.GetWord(t);
            Word<IndexedParameterType> indexWord(mlgWord.GetLabel(), mlgWord.GetImportance(), mlgWord.GetText());

            for (int featIdx = 0; featIdx < mlgWord.Size(); ++featIdx) {
                const auto& feature = mlgWord.GetFeature(featIdx);
                size_t indexId = model->FindFeautureIdMap(std::get<0>(feature), std::get<1>(feature));
                if (indexId != INDEX_NOT_FOUND) {
                    indexWord.Append(make_pair(indexId, std::get<2>(feature)));
                }
            }
            indexSentence.Append(indexWord);
        }

        indexData.push_back(indexSentence);
    }

    // Validate and add tagset constraints to example
    for (size_t i = 0; i < mlgData.size(); ++i) {
        const auto& mlgSentence = mlgData[i];
        auto& indexSentence = indexData[i];

        uint16_t N = model->MaxLabel();
        set<uint16_t> activeTagset;
        for (const auto& tag : mlgSentence.GetActiveTagset()) {
            if (tag < N && tag >= 0)  // check the constraint is valid
                activeTagset.insert(tag);
        }
        if (activeTagset.size() > 0) {
            for (const auto& tag : activeTagset) indexSentence.AddActiveTag(tag);
        } else {
            for (uint16_t tag = 0; tag < N; ++tag) indexSentence.AddActiveTag(tag);
        }
    }

    return indexData;
}
}  // namespace SparseLinearChainCRF