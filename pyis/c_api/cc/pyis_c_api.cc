// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "pyis/pyis_c_api.h"

#include "pyis/share/model_context.h"

void* ModelContextCreate(const char* model_path, const char* data_archive) {
    auto* ctx = new pyis::ModelContext(model_path, data_archive);
    return ctx;
}

void ModelContextDestroy(void* ctx) {
    auto* obj = reinterpret_cast<pyis::ModelContext*>(ctx);
    delete obj;
}

void ModelContextSetFilePrefix(void* ctx, const char* prefix) {
    auto* obj = reinterpret_cast<pyis::ModelContext*>(ctx);
    obj->SetFilePrefix(prefix);
}

void ModelContextActivate(void* ctx) {
    auto* obj = reinterpret_cast<pyis::ModelContext*>(ctx);
    pyis::ModelContext::Activate(obj);
}

void ModelContextDeactivate(void* ctx) {
    auto* obj = reinterpret_cast<pyis::ModelContext*>(ctx);
    pyis::ModelContext::Deactivate(obj);
}

PyisApi pyis_api;

#ifdef __cplusplus
extern "C" {
#endif

PYIS_EXPORT PyisApi* PYIS_API_CALL GetPyisApi() {
    pyis_api.ModelContextCreate = ModelContextCreate;
    pyis_api.ModelContextDestroy = ModelContextDestroy;
    pyis_api.ModelContextActivate = ModelContextActivate;
    pyis_api.ModelContextDeactivate = ModelContextDeactivate;
    pyis_api.ModelContextSetFilePrefix = ModelContextSetFilePrefix;
    return &pyis_api;
}

#ifdef __cplusplus
}
#endif