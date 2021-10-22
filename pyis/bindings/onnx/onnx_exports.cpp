// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "onnx_exports.h"

#include <set>
#include <string>

#include "language_detector/language_detector_op.h"
#include "tokenizers/bert_tokenizer_op.h"

static const char* const OP_DOMAIN = "pyis";

pyis::onnx::OrtOpNaiveLanguageDetector op_naive_language_detector;
pyis::onnx::OrtOpBertTokenizer op_bert_tokenzier;

OrtCustomOp* binding_operators[] = {&op_naive_language_detector, &op_bert_tokenzier};

extern "C" OrtStatus* ORT_API_CALL RegisterCustomOps(OrtSessionOptions* options, const OrtApiBase* api) {
    OrtCustomOpDomain* domain = nullptr;
    const OrtApi* ort_api = api->GetApi(ORT_API_VERSION);

    if (auto* status = ort_api->CreateCustomOpDomain(OP_DOMAIN, &domain)) {
        return status;
    }

    size_t num_ops = sizeof(binding_operators) / sizeof(OrtCustomOp*);
    for (auto i = 0; i < num_ops; i++) {
        if (auto* status = ort_api->CustomOpDomain_Add(domain, binding_operators[i])) {
            return status;
        }
    }

    return ort_api->AddCustomOpDomain(options, domain);
}
