// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "linear_svm.h"

#include <cmath>

#include "liblinear_api.h"
#include "pyis/share/exception.h"
#include "pyis/share/json_persist_helper.h"

namespace pyis {
namespace ops {

LinearSVM::LinearSVM(const std::string& model_file) {
    model_ = load_model(model_file.c_str());
    if (model_ == nullptr) {
        PYIS_THROW("failed to load liblinear model from file %s", model_file.c_str());
    }
}

LinearSVM::~LinearSVM() {
    destroy_param(&model_->param);
    free_and_destroy_model(&model_);
}

void LinearSVM::Train(const std::string& libsvm_data_file, const std::string& model_file, int solver_type, double eps,
                      double c, double p, double bias) {
    bool succeed = liblinear_train(libsvm_data_file.c_str(), model_file.c_str(), solver_type, eps, c, p, bias);
    if (!succeed) {
        PYIS_THROW("failed to train liblinear model");
    }
}

std::vector<double> LinearSVM::Predict(std::vector<std::tuple<int, double>>& features) {
    std::vector<feature_node> feature_nodes(features.size() + 1);
    int idx = 0;
    for (auto& feature : features) {
        feature_nodes[idx].index = static_cast<int>(std::get<0>(feature));
        feature_nodes[idx].value = std::get<1>(feature);
        idx++;
    }
    // insert node with index=-1 to indicate termination.
    feature_nodes[idx].index = -1;
    std::vector<double> values(model_->nr_class);
    predict_values(model_, feature_nodes.data(), values.data());

    return values;
}

void LinearSVM::SaveModel(const std::string& model_file, ModelStorage& storage) {
    auto fp = storage.open_file(model_file.c_str(), "w");
    bool succeed = liblinear_save_model(model_, fp.get());

    if (!succeed) {
        PYIS_THROW("failed to save model to %s", model_file.c_str());
    }
}

void LinearSVM::LoadModel(const std::string& model_file, ModelStorage& storage) {
    auto fp = storage.open_file(model_file.c_str(), "r");
    model_ = liblinear_load_model(fp.get());

    if (model_ == nullptr) {
        PYIS_THROW("failed to load model from %s", model_file.c_str());
    }
}

std::string LinearSVM::Serialize(ModelStorage& storage) {
    std::string model_file = storage.uniq_file("svm", ".model.bin");
    SaveModel(model_file, storage);

    JsonPersistHelper jph(1);
    jph.add("model_file", model_file);

    return jph.serialize();
}

void LinearSVM::Deserialize(const std::string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state);
    int version = jph.version();

    if (1 == version) {
        std::string model_file = jph.get_file("model_file");
        LoadModel(model_file, storage);
    } else {
        PYIS_THROW("LinearSVM v%d is incompatible with the runtime", version);
    }
}

}  // namespace ops
}  // namespace pyis