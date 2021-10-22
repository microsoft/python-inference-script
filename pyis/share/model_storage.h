// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>

#include "str_utils.h"

namespace pyis {

class ModelStorage {
  public:
    explicit ModelStorage(std::string root_dir, std::string prefix = "");
    virtual ~ModelStorage() = default;

    virtual std::shared_ptr<ModelStorage> clone() = 0;

    std::string root_dir();

    void set_file_prefix(const std::string& prefix);
    std::string get_file_prefix();

    virtual std::string uniq_file(const std::string& variant, const std::string& suffix) = 0;
    virtual std::string uniq_file(const std::string& preferred_filepath) = 0;

    virtual std::shared_ptr<std::ostream> open_ostream(             // NOLINT
        const std::string& file_path,                               // NOLINT
        std::ios_base::openmode mode = std::ios_base::out |         // NOLINT
                                       std::ios_base::binary) = 0;  // NOLINT

    virtual std::shared_ptr<std::istream> open_istream(             // NOLINT
        const std::string& file_path,                               // NOLINT
        std::ios_base::openmode mode = std::ios_base::in |          // NOLINT
                                       std::ios_base::binary) = 0;  // NOLINT

    virtual std::shared_ptr<FILE> open_file(const char* file_path, const char* mode) = 0;

    virtual void add_file(const std::string& source_file, const std::string& internal_path) = 0;

    // add src_file from src_storage to dst_storage as dst_file
    static void copy_file(ModelStorage& src_storage, const std::string& src_file, ModelStorage& dst_storage,
                          const std::string& dst_file);

  private:
    std::string root_dir_;
    std::string file_prefix_;
};

}  // namespace pyis