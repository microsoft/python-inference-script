// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "model_storage.h"

#include "str_utils.h"

namespace pyis {

ModelStorage::ModelStorage(std::string root_dir, std::string prefix)
    : root_dir_(std::move(root_dir)), file_prefix_(std::move(prefix)) {
    if (root_dir_.empty()) {
        root_dir_ = ".";
    }
}

std::string ModelStorage::root_dir() { return root_dir_; }

std::string ModelStorage::get_file_prefix() { return file_prefix_; }

void ModelStorage::set_file_prefix(const std::string& prefix) { file_prefix_ = prefix; }

void ModelStorage::copy_file(ModelStorage& src_storage, const std::string& src_file, ModelStorage& dst_storage,
                             const std::string& dst_file) {
    auto src_stream = src_storage.open_istream(src_file);
    auto dst_stream = dst_storage.open_ostream(dst_file);

    std::copy(std::istreambuf_iterator<char>(*src_stream), std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(*dst_stream));
}

}  // namespace pyis