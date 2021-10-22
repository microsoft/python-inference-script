// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "bert_tokenizer_op.h"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <vector>

#include "pyis/bindings/onnx/onnx_tensor_dimensions.h"
#include "pyis/share/exception.h"

namespace pyis {
namespace onnx {

KernelBertTokenizer::KernelBertTokenizer(OrtApi api, const OrtKernelInfo* info) : BaseKernel(api) {
    std::string vocab_file = ort_.KernelInfoGetAttribute<std::string>(info, "vocab_file");
    std::string start_token = ort_.KernelInfoGetAttribute<std::string>(info, "start_token");
    std::string end_token = ort_.KernelInfoGetAttribute<std::string>(info, "end_token");
    std::string unknown_token = ort_.KernelInfoGetAttribute<std::string>(info, "unk_token");

    // tokenizer_ = std::make_shared<BertTokenizer>(vocab_file, start_token, end_token, unknown_token);
    PYIS_THROW("Not implemented");
}

void KernelBertTokenizer::Compute(OrtKernelContext* context) {
    // setup inputs
    const OrtValue* in1 = ort_.KernelContext_GetInput(context, 0);
    std::vector<std::string> strs;
    GetTensorAsString(context, in1, strs);

    // compute
    auto token_ids = tokenizer_->Encode(strs[0]);
    auto token_cnt = static_cast<int64_t>(token_ids.size());

    // setup outputs
    OrtTensorDimensions dimensions({token_cnt});
    OrtValue* out1 = ort_.KernelContext_GetOutput(context, 0, dimensions.data(), dimensions.count());
    auto* out1_data = ort_.GetTensorMutableData<int64_t>(out1);
    for (int64_t i = 0; i < token_cnt; i++) {
        out1_data[i] = token_ids[i];
    }
}

void* OrtOpBertTokenizer::CreateKernel(OrtApi api, const OrtKernelInfo* info) {
    return new KernelBertTokenizer(api, info);
};

const char* OrtOpBertTokenizer::GetName() { return "BertTokenizer"; };

size_t OrtOpBertTokenizer::GetInputTypeCount() { return 1; };

ONNXTensorElementDataType OrtOpBertTokenizer::GetInputType(size_t /*index*/) {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;
};

size_t OrtOpBertTokenizer::GetOutputTypeCount() { return 1; };

ONNXTensorElementDataType OrtOpBertTokenizer::GetOutputType(size_t /*index*/) {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
};

}  // namespace onnx
}  // namespace pyis