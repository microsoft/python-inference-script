// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "model_storage.h"

namespace pyis {

class ModelStorageLocal : public ModelStorage {
  public:
    explicit ModelStorageLocal(const std::string& root_dir = ".", const std::string& prefix = "");

    std::shared_ptr<ModelStorage> clone() override;

    std::string uniq_file(const std::string& variant, const std::string& suffix) override;
    std::string uniq_file(const std::string& preferred_filepath) override;

    std::shared_ptr<std::ostream> open_ostream(const std::string& file_path, std::ios_base::openmode mode) override;

    std::shared_ptr<std::istream> open_istream(const std::string& file_path, std::ios_base::openmode mode) override;

    std::shared_ptr<FILE> open_file(const char* file_path, const char* mode) override;

    void add_file(const std::string& source_path, const std::string& internal_path) override;

  private:
    std::string abs_path(const std::string& path);
    bool file_exists(const std::string& path);
};

}  // namespace pyis