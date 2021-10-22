// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <string>
#include <vector>

#include "pyis/ops/text/text_feature.h"
#include "pyis/share/cached_object.h"
#include "pyis/share/model_storage.h"
#include "sparse_linear_chain_crf_api.h"

namespace SparseLinearChainCRF {
class VanillaCRF;
}

namespace pyis {
namespace ops {

class LinearChainCRF : public CachedObject<LinearChainCRF> {
  public:
    explicit LinearChainCRF(const std::string& model_file);
    ~LinearChainCRF();
    LinearChainCRF(LinearChainCRF&& o) = default;
    // default constructor used for deserilization only.
    LinearChainCRF();

    // disable the following copy semantics
    LinearChainCRF(const LinearChainCRF& o) = delete;
    LinearChainCRF& operator=(const LinearChainCRF& o) = delete;

    static void Train(const std::string& data_file, const std::string& model_file, const std::string& alg,
                      int max_iter);
    std::vector<uint16_t> Predict(uint16_t len, std::vector<std::tuple<uint16_t, uint32_t, double>>& features);

    std::string Serialize(ModelStorage& storage);
    void Deserialize(const std::string& state, ModelStorage& storage);

  private:
    void SaveModel(const std::string& model_file, ModelStorage& storage);
    void LoadModel(const std::string& model_file, ModelStorage& storage);

    SparseLinearChainCRF::VanillaCRF* crf_;
};

}  // namespace ops
}  // namespace pyis