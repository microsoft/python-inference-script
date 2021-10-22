#pragma once
#include <onnxruntime_cxx_api.h>

#include <memory>

namespace pyis {
namespace OrtGlobals {
extern std::shared_ptr<Ort::Env> Env;                                // NOLINT
extern std::shared_ptr<Ort::AllocatorWithDefaultOptions> Allocator;  // NOLINT
extern void* OrtDllInstance;                                         // NOLINT

void Initialize(const std::string& ort_dll_file = "");
}  // namespace OrtGlobals
}  // namespace pyis