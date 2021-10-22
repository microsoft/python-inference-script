// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma warning(push, 0)
#include <torch/custom_class.h>
#include <torch/script.h>
#pragma warning(pop)

#include <sstream>  // std::stringstream

#include "pyis/ops/text/ngram_featurizer.h"
#include "pyis/ops/text/regex_featurizer.h"
#include "pyis/ops/text/text_feature.h"
#include "pyis/ops/text/text_feature_concat.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace torchscript {

using pyis::ops::NGramFeaturizer;
using pyis::ops::RegexFeaturizer;
using pyis::ops::TextFeature;
using pyis::ops::TextFeatureConcat;

class TextFeatureAdaptor : public ::torch::CustomClassHolder, public pyis::ops::TextFeature {
  public:
    using TextFeature::TextFeature;

    explicit TextFeatureAdaptor(TextFeature&& base) : TextFeature(std::move(base)) {}

    explicit operator TextFeature() const { return *this; }

    int64_t id() { return static_cast<int64_t>(TextFeature::id()); }
    double value() { return TextFeature::value(); }
    std::tuple<int64_t, int64_t> pos() {
        auto pos = TextFeature::pos();
        return std::tuple<int64_t, int64_t>(static_cast<int64_t>(std::get<0>(pos)),
                                            static_cast<int64_t>(std::get<1>(pos)));
    }

    std::tuple<int64_t, double, int64_t, int64_t> to_tuple() { return TextFeature::to_tuple(); }
};

void init_text_feature(::torch::Library& m) {
    m.class_<TextFeatureAdaptor>("TextFeature")
        .def(::torch::init<int64_t, double, int64_t, int64_t>(), "",
             {::torch::arg("id"), ::torch::arg("value"), ::torch::arg("start"), ::torch::arg("end")})
        .def("id", &TextFeatureAdaptor::id)
        .def("value", &TextFeatureAdaptor::value)
        .def("pos", &TextFeatureAdaptor::pos)
        .def("to_tuple", &TextFeatureAdaptor::to_tuple);
}

class NGramFeaturizerAdaptor : public ::torch::CustomClassHolder {
  public:
    NGramFeaturizerAdaptor(int64_t n, bool boundaries) {
        obj_ = std::make_shared<NGramFeaturizer>(static_cast<int>(n), boundaries);
    }
    explicit NGramFeaturizerAdaptor(std::shared_ptr<NGramFeaturizer>& obj) { obj_ = obj; }

    void Fit(std::vector<std::string> tokens) { obj_->Fit(tokens); }  // NOLINT

    std::vector<c10::intrusive_ptr<TextFeatureAdaptor>> Transform(std::vector<std::string> tokens) {  // NOLINT
        auto fs = obj_->Transform(tokens);
        std::vector<c10::intrusive_ptr<TextFeatureAdaptor>> res;
        res.reserve(fs.size());
        for (auto& f : fs) {
            res.push_back(c10::make_intrusive<TextFeatureAdaptor>(std::move(f)));
        }
        return res;
    }

    void LoadNGram(std::string file_path) { obj_->LoadNGram(file_path); }
    void DumpNGram(std::string file_path) { obj_->DumpNGram(file_path); }

    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<NGramFeaturizer> obj_;
};

void init_ngram_featurizer(::torch::Library& m) {
    m.class_<NGramFeaturizerAdaptor>("NGramFeaturizer")
        .def(::torch::init<int64_t, bool>(), "", {torch::arg("n"), torch::arg("boundaries") = true})
        .def("fit", &NGramFeaturizerAdaptor::Fit, "", {torch::arg("tokens")})
        .def("transform", &NGramFeaturizerAdaptor::Transform, "", {torch::arg("tokens")})
        .def("load_ngram", &NGramFeaturizerAdaptor::LoadNGram, "", {torch::arg("ngram_file")})
        .def("dump_ngram", &NGramFeaturizerAdaptor::DumpNGram, "", {torch::arg("file_path")})
        .def_pickle(
            [](const c10::intrusive_ptr<NGramFeaturizerAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<NGramFeaturizerAdaptor> {
                auto obj = ModelContext::GetActive()->GetOrCreateObject<NGramFeaturizer>(state);
                return c10::make_intrusive<NGramFeaturizerAdaptor>(obj);
            });
}

class RegexFeaturizerAdaptor : public ::torch::CustomClassHolder {
  public:
    explicit RegexFeaturizerAdaptor(const std::vector<std::string>& regexes) {
        obj_ = std::make_shared<RegexFeaturizer>(regexes);
    }
    explicit RegexFeaturizerAdaptor(std::shared_ptr<RegexFeaturizer>& obj) { obj_ = obj; }

    void AddRegex(const std::string& regex) { obj_->AddRegex(regex); }

    std::vector<c10::intrusive_ptr<TextFeatureAdaptor>> Transform(const std::vector<std::string>& tokens) {
        auto fs = obj_->Transform(tokens);
        std::vector<c10::intrusive_ptr<TextFeatureAdaptor>> res;
        res.reserve(fs.size());
        for (auto& f : fs) {
            res.push_back(c10::make_intrusive<TextFeatureAdaptor>(std::move(f)));
        }
        return res;
    }

    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<RegexFeaturizer> obj_;
};

void init_regex_featurizer(::torch::Library& m) {
    m.class_<RegexFeaturizerAdaptor>("RegexFeaturizer")
        .def(::torch::init<std::vector<std::string>>(), "", {torch::arg("regexes")})
        .def("add_regex", &RegexFeaturizerAdaptor::AddRegex, "", {torch::arg("regex")})
        .def("transform", &RegexFeaturizerAdaptor::Transform, "", {torch::arg("tokens")})
        .def_pickle(
            [](const c10::intrusive_ptr<RegexFeaturizerAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<RegexFeaturizerAdaptor> {
                auto obj = std::make_shared<RegexFeaturizer>();
                obj->Deserialize(state, ModelContext::GetActive()->Storage());
                return c10::make_intrusive<RegexFeaturizerAdaptor>(obj);
            });
}

class TextFeatureConcatAdaptor : public ::torch::CustomClassHolder {
  public:
    explicit TextFeatureConcatAdaptor(int64_t start_id) {
        obj_ = std::make_shared<TextFeatureConcat>(static_cast<uint64_t>(start_id));
    }
    explicit TextFeatureConcatAdaptor(std::shared_ptr<TextFeatureConcat>& obj) { obj_ = obj; }

    void Fit(std::vector<std::vector<c10::intrusive_ptr<TextFeatureAdaptor>>> feature_groups) {
        std::vector<std::vector<TextFeature>> inputs(feature_groups.size());
        for (auto i = 0; i < feature_groups.size(); i++) {
            for (auto& feature : feature_groups[i]) {
                inputs[i].push_back(*(feature.get()));
            }
        }
        obj_->Fit(inputs);
    }

    std::vector<c10::intrusive_ptr<TextFeatureAdaptor>> Transform(
        std::vector<std::vector<c10::intrusive_ptr<TextFeatureAdaptor>>> feature_groups) {
        std::vector<std::vector<TextFeature>> inputs(feature_groups.size());
        for (auto i = 0; i < feature_groups.size(); i++) {
            for (auto& feature : feature_groups[i]) {
                inputs[i].push_back(*(feature.get()));
            }
        }

        auto fs = obj_->Transform(inputs);
        std::vector<c10::intrusive_ptr<TextFeatureAdaptor>> res;
        res.reserve(fs.size());
        for (auto& f : fs) {
            res.push_back(c10::make_intrusive<TextFeatureAdaptor>(std::move(f)));
        }
        return res;
    }

    std::string Serialize(ModelStorage& storage) { return obj_->Serialize(storage); }

  private:
    std::shared_ptr<TextFeatureConcat> obj_;
};

void init_text_feature_concat(::torch::Library& m) {
    m.class_<TextFeatureConcatAdaptor>("TextFeatureConcat")
        .def(::torch::init<int64_t>(), "", {torch::arg("start_id") = 0})
        .def("fit", &TextFeatureConcatAdaptor::Fit, "", {torch::arg("feature_groups")})
        .def("transform", &TextFeatureConcatAdaptor::Transform, "", {torch::arg("feature_groups")})
        .def_pickle(
            [](const c10::intrusive_ptr<TextFeatureConcatAdaptor>& self) -> std::string {
                std::string state = self->Serialize(ModelContext::GetActive()->Storage());
                return state;
            },
            [](const std::string& state) -> c10::intrusive_ptr<TextFeatureConcatAdaptor> {
                auto obj = std::make_shared<TextFeatureConcat>();
                obj->Deserialize(state, ModelContext::GetActive()->Storage());
                return c10::make_intrusive<TextFeatureConcatAdaptor>(obj);
            });
}

}  // namespace torchscript
}  // namespace pyis