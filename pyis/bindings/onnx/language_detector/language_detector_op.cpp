// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "language_detector_op.h"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <vector>

#include "pyis/bindings/onnx/onnx_tensor_dimensions.h"

namespace pyis {
namespace onnx {

KernelNaiveLanguageDetector::KernelNaiveLanguageDetector(OrtApi api) : BaseKernel(api), detector_() {}

void KernelNaiveLanguageDetector::Compute(OrtKernelContext* context) {
    // get inputs
    const OrtValue* input = ort_.KernelContext_GetInput(context, 0);
    std::vector<std::string> input_data;
    GetTensorAsString(context, input, input_data);

    OrtTensorDimensions dimensions({1});
    OrtValue* output = ort_.KernelContext_GetOutput(context, 0, dimensions.data(), dimensions.count());
    auto* output_data = ort_.GetTensorMutableData<int64_t>(output);
    output_data[0] = detector_.detect(input_data[0]);
}

void* OrtOpNaiveLanguageDetector::CreateKernel(OrtApi api, const OrtKernelInfo* /* info */) {
    return new KernelNaiveLanguageDetector(api);
};

const char* OrtOpNaiveLanguageDetector::GetName() { return "NaiveLanguageDetector"; };

size_t OrtOpNaiveLanguageDetector::GetInputTypeCount() { return 1; };

ONNXTensorElementDataType OrtOpNaiveLanguageDetector::GetInputType(size_t /*index*/) {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;
};

size_t OrtOpNaiveLanguageDetector::GetOutputTypeCount() { return 1; };

ONNXTensorElementDataType OrtOpNaiveLanguageDetector::GetOutputType(size_t /*index*/) {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
};
}  // namespace onnx
}  // namespace pyis