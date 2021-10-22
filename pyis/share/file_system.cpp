// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "file_system.h"

#include <fstream>
#include <iostream>

#include "exception.h"
#include "str_utils.h"

#ifdef _WIN32
#include <io.h>
#define access _access_s
#elif __unix__
#include <unistd.h>
#else
#endif

namespace pyis {

std::string FileSystem::join_path(std::initializer_list<std::string> paths) { return join_str(paths, "/"); }

std::string FileSystem::dirname(const std::string& path) {
    auto idx = path.find_last_of("\\/");
    if (idx == std::string::npos) {
        return "";
    }

    std::string res = path.substr(0, idx);
    return res;
}

std::string FileSystem::filename(const std::string& path, bool extension) {
    std::string res;
    auto idx = path.find_last_of("\\/");
    if (idx == std::string::npos) {
        res = path;
    }
    res = path.substr(idx + 1);

    if (extension) {
        return res;
    }

    idx = path.find_last_of('.');
    if (idx == std::string::npos) {
        return res;
    }
    res = res.substr(0, idx);
    return res;
}

std::string FileSystem::extname(const std::string& path) {
    std::string res = filename(path);
    auto idx = res.find_last_of('.');
    if (idx == std::string::npos) {
        return "";
    }

    res = res.substr(idx);
    return res;
}

bool FileSystem::is_absolute(const std::string& path) {
    char ch = path[0];
    if (ch == '/' || ch == '\\') {
        return true;
    }

    if (((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) && (path.length() >= 2 && path[1] == ':')) {
        return true;
    }

    return false;
}

std::string FileSystem::abs_path(const std::string& path) {
    std::string abs_path = path;
    if (!is_absolute(path)) {
        PYIS_THROW("FileSystem::abs_path is not implemented");
    }
    return abs_path;
}

bool FileSystem::file_exists(const std::string& path) {
#if defined(_WIN32) || defined(__unix__)
    return access(path.c_str(), 0) == 0;
#else
    PYIS_THROW("FileSystem::file_exists is undefined on current platform");
#endif
}

std::string FileSystem::uniq_file(const std::string& variant, const std::string& suffix) {
    std::string res = variant + suffix;
    if (!file_exists(res)) {
        return res;
    }

    int i = 1;
    do {
        res = variant + std::to_string(i) + suffix;
        i += 1;
    } while (file_exists(res));

    return res;
}

std::string FileSystem::uniq_file(const std::string& preferred_filepath) {
    if (!file_exists(preferred_filepath)) {
        return preferred_filepath;
    }

    std::string dir = dirname(preferred_filepath);
    std::string file = filename(preferred_filepath, false);
    std::string ext = extname(preferred_filepath);

    std::string res;
    int i = 1;
    do {
        res = join_path({dir, file + std::to_string(i), ext});
        i += 1;
    } while (file_exists(res));

    return res;
}

std::shared_ptr<std::ostream> FileSystem::open_ostream(const std::string& file_path, std::ios_base::openmode mode) {
    std::shared_ptr<std::ofstream> ofs(new std::ofstream(), [](std::ofstream* s) {
        s->close();
        delete s;
        // std::cout << "ostream closed" << std::endl;
    });
#if defined(_WIN32)
    ofs->open(str_to_wstr(file_path), mode);
#else
    ofs->open(file_path, mode);
#endif

    if (!ofs->good()) {
        PYIS_THROW("failed to open file %s", file_path.c_str());
    }

    return ofs;
}

std::shared_ptr<std::istream> FileSystem::open_istream(const std::string& file_path, std::ios_base::openmode mode) {
    std::shared_ptr<std::ifstream> ifs(new std::ifstream(), [](std::ifstream* s) {
        s->close();
        delete s;
    });
#if defined(_WIN32)
    ifs->open(str_to_wstr(file_path), mode);
#else
    ifs->open(file_path, mode);
#endif

    if (!ifs->good()) {
        PYIS_THROW("failed to open file %s", file_path.c_str());
    }

    return ifs;
}

std::shared_ptr<FILE> FileSystem::open_file(const char* file_path, const char* mode) {
    FILE* file = nullptr;
#if defined(_WIN32)
    file = _wfopen(str_to_wstr(file_path).c_str(), str_to_wstr(mode).c_str());
#else
    file = std::fopen(file_path, mode);
#endif

    if (nullptr == file) {
        PYIS_THROW("failed to open file %s", file_path);
    }

    std::shared_ptr<FILE> fp(file, [](FILE* p) {
        std::fclose(p);
        // std::cout << "File closed" << std::endl;
    });

    return fp;
}

#if defined(_WIN32)

namespace {
char dummy_char;
}

std::string FileSystem::get_assembly_path() {
    HMODULE phModule = NULL;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCSTR)&dummy_char, &phModule)) {
        throw std::runtime_error("can't get dll path");
    }
    CHAR path[MAX_PATH];
    GetModuleFileNameA(phModule, path, MAX_PATH);
    return std::string(path);
}

#endif

#if defined(__unix__)

std::string FileSystem::get_assembly_path() {
    Dl_info dl_info;
    dladdr(reinterpret_cast<void*>(get_assembly_path), &dl_info);
    return std::string(dl_info.dli_fname);
}

#endif
}  // namespace pyis