// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <initializer_list>
#include <memory>

#include "str_utils.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(__unix__)
#include <dlfcn.h>
#endif

namespace pyis {

class FileSystem {
  public:
    explicit FileSystem() = default;
    virtual ~FileSystem() = default;

    static std::string join_path(std::initializer_list<std::string> paths);

    static std::string dirname(const std::string& path);

    static std::string filename(const std::string& path, bool extension = true);

    static std::string extname(const std::string& path);

    static bool is_absolute(const std::string& path);

    static std::string abs_path(const std::string& path);

    static bool file_exists(const std::string& path);

    static std::string uniq_file(const std::string& variant, const std::string& suffix);

    static std::string uniq_file(const std::string& preferred_filepath);

    static std::shared_ptr<std::ostream> open_ostream(const std::string& file_path,                           // NOLINT
                                                      std::ios_base::openmode mode = std::ios_base::out |     // NOLINT
                                                                                     std::ios_base::binary);  // NOLINT

    static std::shared_ptr<std::istream> open_istream(const std::string& file_path,                           // NOLINT
                                                      std::ios_base::openmode mode = std::ios_base::in |      // NOLINT
                                                                                     std::ios_base::binary);  // NOLINT

    static std::shared_ptr<FILE> open_file(const char* file_path, const char* mode);

    static std::string get_assembly_path();
};

}  // namespace pyis