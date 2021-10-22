// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ort_globals.h"

#include "pyis/share/exception.h"
#include "pyis/share/file_system.h"

typedef const OrtApiBase* (*FUNCPTR_OrtGetApiBaseFuncType)();

namespace pyis {

void OrtGlobals::Initialize(const std::string& ort_dll_file) {
    // if already loaded, unload the existing one
    if (OrtDllInstance != nullptr) {
#if defined(_WIN32)
        ::FreeLibrary((HMODULE)OrtDllInstance);
#endif

#if defined(__unix__)
        dlclose(OrtDllInstance);
#endif
    }

    std::string ort_dll_dir;
#if defined(_WIN32)
    std::string ort_dll_filename = "onnxruntime.dll";
#endif
#if defined(__unix__)
    std::string ort_dll_filename = "libonnxruntime.so";
#endif
    if (ort_dll_file.empty()) {
        // no ort dll file specified, used ort in pyis
        ort_dll_dir = FileSystem::dirname(FileSystem::get_assembly_path());
    } else {
        ort_dll_dir = FileSystem::dirname(ort_dll_file);
        ort_dll_filename = FileSystem::filename(ort_dll_file);
    }

    // read the function from dynamic linking library
#if defined(_WIN32)
    SetDllDirectoryA(ort_dll_dir.c_str());
    OrtDllInstance = static_cast<void*>(::LoadLibrary(TEXT(ort_dll_filename.c_str())));
    if (OrtDllInstance == nullptr) {
        PYIS_THROW("onnxruntime dll loading failed!");
    }
    auto funcptr_ort_get_api_base =
        reinterpret_cast<FUNCPTR_OrtGetApiBaseFuncType>(::GetProcAddress((HINSTANCE)OrtDllInstance, "OrtGetApiBase"));
#endif
#if defined(__unix__)
    OrtDllInstance = dlopen(FileSystem::join_path({ort_dll_dir, ort_dll_filename}).c_str(), RTLD_LAZY);
    if (OrtDllInstance == nullptr) {
        PYIS_THROW("libonnxruntime so loading failed!");
    }
    auto funcptr_ort_get_api_base =
        reinterpret_cast<FUNCPTR_OrtGetApiBaseFuncType>(dlsym(OrtDllInstance, "OrtGetApiBase"));
#endif
    const auto* ort_api_base = funcptr_ort_get_api_base();
    Ort::Global<void>::api_ = ort_api_base->GetApi(ORT_API_VERSION);

    Env = std::make_shared<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ort");
    Allocator = std::make_shared<Ort::AllocatorWithDefaultOptions>();
}

std::shared_ptr<Ort::Env> OrtGlobals::Env;                                // NOLINT
std::shared_ptr<Ort::AllocatorWithDefaultOptions> OrtGlobals::Allocator;  // NOLINT
void* OrtGlobals::OrtDllInstance;                                         // NOLINT

}  // namespace pyis
