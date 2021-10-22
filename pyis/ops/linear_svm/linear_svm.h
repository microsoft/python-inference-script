// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <string>
#include <vector>

#include "pyis/share/cached_object.h"
#include "pyis/share/model_storage.h"

struct model;

namespace pyis {
namespace ops {

class LinearSVM : public CachedObject<LinearSVM> {
  public:
    explicit LinearSVM(const std::string& model_file);
    ~LinearSVM();
    LinearSVM(LinearSVM&& o) = default;
    // default constructor used for deserilization only.
    LinearSVM() = default;

    // disable the following copy semantics
    LinearSVM(const LinearSVM& o) = delete;
    LinearSVM& operator=(const LinearSVM& o) = delete;

    static void Train(const std::string& libsvm_data_file, const std::string& model_file, int solver_type, double eps,
                      double c, double p, double bias);
    std::vector<double> Predict(std::vector<std::tuple<int, double>>& features);

    std::string Serialize(ModelStorage& storage);
    void Deserialize(const std::string& state, ModelStorage& storage);

  private:
    void SaveModel(const std::string& model_file, ModelStorage& storage);
    void LoadModel(const std::string& model_file, ModelStorage& storage);

    model* model_;
};

}  // namespace ops
}  // namespace pyis