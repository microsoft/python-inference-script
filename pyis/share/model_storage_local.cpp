// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "model_storage_local.h"

#include <fstream>
#include <iostream>

#include "exception.h"
#include "file_system.h"

#if defined(_WIN32) || defined(__unix__)
#include <experimental/filesystem>
namespace efs = std::experimental::filesystem;
#endif

namespace pyis {

using fs = FileSystem;

ModelStorageLocal::ModelStorageLocal(const std::string& root_dir, const std::string& prefix)
    : ModelStorage(root_dir, prefix) {}

std::shared_ptr<ModelStorage> ModelStorageLocal::clone() { return std::make_shared<ModelStorageLocal>(*this); }

std::string ModelStorageLocal::abs_path(const std::string& path) {
    std::string abs_path = path;
    if (!fs::is_absolute(path)) {
        abs_path = fs::join_path({root_dir(), path});
    }
    return abs_path;
}

bool ModelStorageLocal::file_exists(const std::string& path) { return fs::file_exists(abs_path(path)); }

std::string ModelStorageLocal::uniq_file(const std::string& variant, const std::string& suffix) {
    if (variant.find_first_of("/\\") != std::string::npos) {
        PYIS_THROW("variant could only be a filename, without any folder structure. variant:%s", variant.c_str());
    }

    std::string res = get_file_prefix() + variant + suffix;
    if (!file_exists(res)) {
        return res;
    }

    int i = 1;
    do {
        res = get_file_prefix() + variant + std::to_string(i) + suffix;
        i += 1;
    } while (file_exists(res));

    return res;
}

std::string ModelStorageLocal::uniq_file(const std::string& preferred_filepath) {
    if (preferred_filepath.find_first_of("/\\") != std::string::npos) {
        PYIS_THROW("preferred_filepath could only be a filename, without any folder structure. preferred_filepath:%s",
                   preferred_filepath.c_str());
    }

    std::string p = get_file_prefix() + preferred_filepath;
    if (!file_exists(p)) {
        return p;
    }

    std::string filename = fs::filename(p, false);
    std::string ext = fs::extname(p);

    std::string res;
    int i = 1;
    do {
        res = get_file_prefix() + filename + std::to_string(i) + ext;
        i += 1;
    } while (file_exists(res));

    return res;
}

std::shared_ptr<std::ostream> ModelStorageLocal::open_ostream(const std::string& file_path,
                                                              std::ios_base::openmode mode) {
    return fs::open_ostream(abs_path(file_path), mode);
}

std::shared_ptr<std::istream> ModelStorageLocal::open_istream(const std::string& file_path,
                                                              std::ios_base::openmode mode) {
    return fs::open_istream(abs_path(file_path), mode);
}

std::shared_ptr<FILE> ModelStorageLocal::open_file(const char* file_path, const char* mode) {
    return fs::open_file(abs_path(file_path).c_str(), mode);
}

void ModelStorageLocal::add_file(const std::string& source_path, const std::string& internal_path) {
#if defined(_WIN32) || defined(__unix__)
    efs::path path_src(source_path);
    efs::path path_internal(fs::join_path({root_dir(), internal_path}));
    if (path_src != path_internal) {
        efs::copy_file(path_src, path_internal);
    }
#else
    throw std::runtime_error("ModelStorageLocal::add_file is not implemented");
#endif
}

}  // namespace pyis