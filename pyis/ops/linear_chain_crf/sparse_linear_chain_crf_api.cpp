// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "sparse_linear_chain_crf_api.h"

#include <iostream>
#include <memory>

#include "include/CRFFactory.h"
#include "include/CommandFactory.h"
#include "include/ConsoleUtils.h"
#include "include/Decode.h"
#include "include/ICommand.h"
#include "include/Learn.h"
#include "pyis/share/exception.h"
#include "src/ILinearChainCRF.h"
#include "src/SparseLinearModel.h"
#include "src/StreamDataManager.h"
#include "src/StringUtils.h"
#include "src/VanillaCRF.h"

using std::shared_ptr;
using namespace SparseLinearChainCRFConsole;

vector<uint16_t>* VectorCreate() {
    vector<uint16_t>* ret = new vector<uint16_t>();
    return ret;
}

void VectorDelete(vector<uint16_t>* vec) { delete vec; }

void* SparseLinearChainCRFCreate() {
    VanillaCRF* crf = new VanillaCRF();
    return crf;
}

void SparseLinearChainCRFDelete(void* crf) { delete crf; }

void SparseLinearChainCRFTrain(const char* model_file, const char* data_file, const char* tmp_dir, const char* alg,
                               int max_iter) {
    CommandFactory cmd_factory;
    std::shared_ptr<ICommand> cmd(nullptr);

    std::string cmd_str("learn");

    // LuLCCRF commands
    cmd = cmd_factory.Create("learn");
    if (cmd == nullptr) {
        PYIS_THROW("failed to create cmd factory");
    }

    std::map<std::string, std::string> options;
    if (std::strcmp(alg, "l1sgd") == 0) {
        options = {{"verbose", "1"},
                   {"force", "1"},
                   {"model", model_file},
                   {"train", data_file},
                   {"algo", alg},
                   {"stream.temp", tmp_dir},
                   {"iter.max", std::to_string(max_iter)},
                   {"sparsity", "0.1"},
                   {"iter.warm", "10"},
                   {"online.decay", "exp"},
                   {"iter.save", "1"}};
    } else if (std::strcmp(alg, "perceptron") == 0) {
        options = {{"verbose", "1"},
                   {"force", "1"},
                   {"model", model_file},
                   {"train", data_file},
                   {"algo", alg},
                   {"stream.temp", tmp_dir},
                   {"iter.max", std::to_string(max_iter)}};
    } else {
        PYIS_THROW("unknown training algorithm for lccrf");
    }
    cmd->Run(options);
}

void SparseLinearChainCRFLoad(void* crf, const char* model_file) {
    VanillaCRF* obj = reinterpret_cast<VanillaCRF*>(crf);
    shared_ptr<SparseLinearModel> model(new SparseLinearModel(model_file));
    obj->Initialize(model);
}

void SparseLinearChainCRFLoad(void* crf, istream& model_stream) {
    VanillaCRF* obj = reinterpret_cast<VanillaCRF*>(crf);
    shared_ptr<SparseLinearModel> model(new SparseLinearModel(model_stream));
    obj->Initialize(model);
}

void SparseLinearChainCRFSave(void* crf, ostream& model_stream) {
    VanillaCRF* obj = reinterpret_cast<VanillaCRF*>(crf);
    obj->m_LinearModel->Serialize(model_stream);
}

void SparseLinearChainCRFDecode(void* crf, int word_cnt, int* word_feat_cnt, int* features, vector<int>* tags) {
    VanillaCRF* obj = reinterpret_cast<VanillaCRF*>(crf);
    IndexedSentence index_sentence;
    int start = 0;
    for (int t = 0; t < word_cnt; ++t) {
        Word<IndexedParameterType> index_word(0, 1.0, "");
        for (int feat_idx = start; feat_idx < start + word_feat_cnt[t]; ++feat_idx) {
            size_t index_id = obj->m_LinearModel->FindFeautureIdMap(features[2 * feat_idx], features[2 * feat_idx + 1]);
            if (index_id != INDEX_NOT_FOUND) {
                index_word.Append(std::make_pair(index_id, 1.0));
            }
        }
        start += word_feat_cnt[t];
        index_sentence.Append(index_word);
    }
    vector<uint16_t> uint16_tags;
    obj->Decode(index_sentence, uint16_tags);
    for (auto ite = uint16_tags.begin(); ite != uint16_tags.end(); ite++) {
        tags->push_back(*ite);
    }
    return;
}

std::vector<uint16_t> SparseLinearChainCRFDecode(void* crf, uint16_t len,
                                                 std::vector<std::tuple<uint16_t, uint32_t, double>>& features) {
    VanillaCRF* obj = reinterpret_cast<VanillaCRF*>(crf);
    IndexedSentence index_sentence;
    index_sentence.Resize(len, Word<IndexedParameterType>(0, 1.0, ""));
    for (auto& f : features) {
        Word<IndexedParameterType> index_word(0, 1.0, "");
        // Note: the first argument of FindFeautureIdMap is set to be 1.
        // It must be the same number when generatring lccrf data file.
        size_t index_id = obj->m_LinearModel->FindFeautureIdMap(1, std::get<1>(f));
        if (index_id != INDEX_NOT_FOUND) {
            index_sentence.GetWord(std::get<0>(f)).Append(std::make_pair(index_id, static_cast<float>(std::get<2>(f))));
        }
    }

    vector<uint16_t> tags;
    obj->Decode(index_sentence, tags);
    return tags;
}

int32_t GenerateHash(int tag_a, int tag_b) {
    char concat_bytes[8];
    memcpy(concat_bytes, &tag_a, sizeof(int));
    memcpy(concat_bytes + sizeof(int), &tag_b, sizeof(int));
    int32_t hash = 0;
    for (size_t i = 0; i < sizeof(concat_bytes); ++i) {
        hash += concat_bytes[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    if (hash < 0) {
        hash *= -1;
    }

    return hash;
}
