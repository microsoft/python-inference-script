// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <climits>
#include <cstring>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

void* SparseLinearChainCRFCreate();
void SparseLinearChainCRFDelete(void* crf);

void SparseLinearChainCRFTrain(const char* model_file, const char* data_file, const char* tmp_dir, const char* alg,
                               int max_iter);

void SparseLinearChainCRFLoad(void* crf, const char* model_file);
void SparseLinearChainCRFLoad(void* crf, std::istream& model_stream);
void SparseLinearChainCRFSave(void* crf, std::ostream& model_stream);

void SparseLinearChainCRFDecode(void* crf, int word_cnt, int* word_feat_cnt, int* features, std::vector<int>* tags);

std::vector<uint16_t> SparseLinearChainCRFDecode(void* crf, uint16_t len,
                                                 std::vector<std::tuple<uint16_t, uint32_t, double>>& features);

int32_t GenerateHash(int tag_a, int tag_b);
