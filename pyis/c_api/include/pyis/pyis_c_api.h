// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#ifdef PYIS_DLL_EXPORT
#define PYIS_EXPORT __declspec(dllexport)
#else
#define PYIS_EXPORT __declspec(dllimport)
#endif
#define PYIS_API_CALL _stdcall
#elif defined(__APPLE__)
#define PYIS_EXPORT __attribute__((visibility("default")))
#define PYIS_API_CALL
#else
#define PYIS_EXPORT
#define PYIS_API_CALL
#endif

struct PyisApi {
    void* (*ModelContextCreate)(const char* model_path, const char* data_archive);
    void (*ModelContextDestroy)(void* ctx);

    // Set the global `active_model_context` instance. You CAN NOT activate two
    // model context instances at the same time.
    //
    // The `model_ctx` is NOT owned by pyis. You are responsiable for its lifecycle.
    //
    // Deactivation needs to be guaranteed when:
    // 1. model loading or saving is done.
    // 2. the execution is interrupted by unexpected exceptions.
    //
    // A good practive is to use a scope guard(unique_ptr or shared_ptr) to guarantee
    // deactivation of the model context.
    /*
        auto deactivator = [](void* p) {
            ModelContextDeactivate(p);
            ModelContextDestroy(p);
            std::cout << "model context deactivated" << std::endl;
        };
        std::unique_ptr<void, decltype(deactivator)> ctx(ModelContextCreate("path/to/model"),
                                                         deactivator);
        ModelContextDeactivate(ctx.get());
        std::cout << "model context activated" << std::endl;
    */
    void (*ModelContextActivate)(void* ctx);
    void (*ModelContextDeactivate)(void* ctx);

    void (*ModelContextSetFilePrefix)(void* ctx, const char* prefix);
};

PYIS_EXPORT PyisApi* PYIS_API_CALL GetPyisApi();

#ifdef __cplusplus
}
#endif