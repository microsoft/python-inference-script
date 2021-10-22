// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "linear_chain_crf.h"

#include <cmath>

#include "pyis/share/exception.h"
#include "pyis/share/file_system.h"
#include "pyis/share/json_persist_helper.h"

namespace pyis {
namespace ops {

LinearChainCRF::LinearChainCRF() {
    crf_ = reinterpret_cast<SparseLinearChainCRF::VanillaCRF*>(SparseLinearChainCRFCreate());
}

LinearChainCRF::LinearChainCRF(const std::string& model_file) : LinearChainCRF() {
    SparseLinearChainCRFLoad(crf_, model_file.c_str());
}

LinearChainCRF::~LinearChainCRF() { SparseLinearChainCRFDelete(crf_); }

void LinearChainCRF::Train(const std::string& data_file, const std::string& model_file, const std::string& alg,
                           int max_iter) {
    std::string tmp_dir = FileSystem::dirname(model_file);
    SparseLinearChainCRFTrain(model_file.c_str(), data_file.c_str(), tmp_dir.c_str(), alg.c_str(), max_iter);
}

std::vector<uint16_t> LinearChainCRF::Predict(uint16_t len,
                                              std::vector<std::tuple<uint16_t, uint32_t, double>>& features) {
    return SparseLinearChainCRFDecode(crf_, len, features);
}

void LinearChainCRF::SaveModel(const std::string& model_file, ModelStorage& storage) {
    auto fp = storage.open_ostream(model_file);
    SparseLinearChainCRFSave(crf_, *(fp.get()));
}

void LinearChainCRF::LoadModel(const std::string& model_file, ModelStorage& storage) {
    auto ifs = storage.open_istream(model_file);
    SparseLinearChainCRFLoad(crf_, *(ifs.get()));
}

std::string LinearChainCRF::Serialize(ModelStorage& storage) {
    std::string model_file = storage.uniq_file("lccrf", ".model.bin");
    SaveModel(model_file, storage);

    JsonPersistHelper jph(1);
    jph.add_file("model_file", model_file);

    return jph.serialize();
}

void LinearChainCRF::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state);
    int version = jph.version();

    if (1 == version) {
        std::string model_file = jph.get_file("model_file");
        LoadModel(model_file, storage);
    } else {
        PYIS_THROW("LinearChainCRF v%d is incompatible with the runtime", version);
    }
}

}  // namespace ops
}  // namespace pyis