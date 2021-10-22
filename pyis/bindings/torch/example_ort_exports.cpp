// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)
#include "pyis/ops/example_ort/language_detector.h"
#include "pyis/ops/example_ort/naive_tokenizer.h"

namespace pyis {
namespace torchscript {

class NaiveLanguageDetectorWrapper : public ::torch::CustomClassHolder {
  public:
    int64_t detect(const std::string& str) { return obj_.detect(str); }

  private:
    pyis::ops::NaiveLanguageDetector obj_;
};

class NaiveTokenizerWrapper : public ::torch::CustomClassHolder {
  public:
    explicit NaiveTokenizerWrapper(const std::string& vocab_file_path) : obj_(vocab_file_path){};

    std::vector<int64_t> tokenize(const std::string& str) { return obj_.tokenize(str); }

  private:
    pyis::ops::NaiveTokenizer obj_;
};

void init_example_ort(::torch::Library& m) {
    m.class_<NaiveLanguageDetectorWrapper>("NaiveLanguageDetector")
        .def(::torch::init())
        .def("detect", &NaiveLanguageDetectorWrapper::detect)
        .def_pickle(
            [](const c10::intrusive_ptr<NaiveLanguageDetectorWrapper>&) -> std::string {
                return "NaiveLanguageDetector";
            },
            [](const std::string&) -> c10::intrusive_ptr<NaiveLanguageDetectorWrapper> {
                return c10::make_intrusive<NaiveLanguageDetectorWrapper>();
            });

    m.class_<NaiveTokenizerWrapper>("NaiveTokenizer")
        .def(torch::init<std::string>())
        .def("tokenize", &NaiveTokenizerWrapper::tokenize)
        .def_pickle([](const c10::intrusive_ptr<NaiveTokenizerWrapper>&) -> std::string { return "NaiveTokenizer"; },
                    [](const std::string& state) -> c10::intrusive_ptr<NaiveTokenizerWrapper> {
                        return c10::make_intrusive<NaiveTokenizerWrapper>(state);
                    });
}

}  // namespace torchscript
}  // namespace pyis