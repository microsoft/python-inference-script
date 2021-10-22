// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "onnx_base_kernel.h"

#include "onnx_tensor_dimensions.h"
#include "pyis/share/exception.h"

namespace pyis {
namespace onnx {

bool BaseKernel::HasAttribute(const char* name) const {
    if (info_ == nullptr) {
        PYIS_THROW("Kernel was incorrectly initialized, pointer info_ cannot be null.");
    }
    size_t size = 0;
    std::string out;
    // Crashes here.
    OrtStatus* status = api_.KernelInfoGetAttribute_string(info_, name, nullptr, &size);
    auto r = api_.GetErrorCode(status);
    bool has = (r == ORT_INVALID_ARGUMENT) || (r == ORT_OK);
    if (has) {
        api_.ReleaseStatus(status);
        return has;
    }
    const char* error = api_.GetErrorMessage(status);
    if (strstr(error, "No attribute") == error) {
        api_.ReleaseStatus(status);
        return false;
    }
    api_.ReleaseStatus(status);
    return true;
}

void BaseKernel::GetTensorAsString(OrtKernelContext* /*context*/, const OrtValue* value,
                                   std::vector<std::string>& output) {
    OrtTensorDimensions dimensions(ort_, value);

    auto len = static_cast<size_t>(dimensions.size());
    size_t data_len;
    Ort::ThrowOnError(api_, api_.GetStringTensorDataLength(value, &data_len));
    output.resize(len);
    std::vector<char> result(data_len + len + 1, '\0');
    std::vector<size_t> offsets(len);
    Ort::ThrowOnError(api_, api_.GetStringTensorContent(value, reinterpret_cast<void*>(result.data()), data_len,
                                                        offsets.data(), offsets.size()));
    output.resize(len);
    for (int64_t i = static_cast<int64_t>(len) - 1; i >= 0; --i) {
        if (i < len - 1) {
            result[offsets[i + 1]] = '\0';
        }
        output[i] = result.data() + offsets[i];
    }
}

OrtErrorCode BaseKernel::GetErrorCodeAndRelease(OrtStatusPtr status) const {
    if (status == nullptr) {
        return ORT_OK;
    }
    auto error_code = api_.GetErrorCode(status);
    api_.ReleaseStatus(status);
    return error_code;
}

bool BaseKernel::TryGetAttribute(const char* name, std::string& value) {
    if (info_ == nullptr) {
        PYIS_THROW("kernel was incorrectly initialized, pointer info_ cannot be null.");
    }

    size_t size = 0;
    OrtStatus* status = api_.KernelInfoGetAttribute_string(info_, name, nullptr, &size);

    // The status should be ORT_INVALID_ARGUMENT because the size is
    // insufficient to hold the string
    if (GetErrorCodeAndRelease(status) != ORT_INVALID_ARGUMENT) {
        return false;
    }

    value.resize(size);
    status = api_.KernelInfoGetAttribute_string(info_, name, &value[0], &size);
    if (GetErrorCodeAndRelease(status) != ORT_OK) {
        return false;
    }
    value.resize(size - 1);

    return true;
}

bool BaseKernel::TryGetAttribute(const char* name, int64_t& value) {
    if (info_ == nullptr) {
        PYIS_THROW("Kernel was incorrectly initialized, pointer info_ cannot be null.");
    }

    return GetErrorCodeAndRelease(api_.KernelInfoGetAttribute_int64(info_, name, &value)) == ORT_OK;
}

bool BaseKernel::TryGetAttribute(const char* name, float& value) {
    if (info_ == nullptr) {
        PYIS_THROW("Kernel was incorrectly initialized, pointer info_ cannot be null.");
    }

    return GetErrorCodeAndRelease(api_.KernelInfoGetAttribute_float(info_, name, &value)) == ORT_OK;
}

}  // namespace onnx
}  // namespace pyis