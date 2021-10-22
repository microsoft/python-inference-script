// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pyis/ops/text/ngram_featurizer.h"
#include "pyis/ops/text/regex_featurizer.h"
#include "pyis/ops/text/text_feature_concat.h"
#include "pyis/share/model_context.h"
#include "pyis/share/scope_guard.h"

namespace pyis {
namespace python {

using pyis::ops::NGramFeaturizer;
using pyis::ops::RegexFeaturizer;
using pyis::ops::TextFeature;
using pyis::ops::TextFeatureConcat;

namespace py = pybind11;

void init_text_feature(py::module& m) {
    py::class_<TextFeature, std::shared_ptr<TextFeature>>(m, "TextFeature")
        .def(py::init<uint64_t, double, int, int>(), py::arg("id"), py::arg("value"), py::arg("start"), py::arg("end"))
        .def("id", &TextFeature::id)
        .def("value", &TextFeature::value)
        .def("pos", &TextFeature::pos)
        .def("to_tuple", &TextFeature::to_tuple);
}

void init_ngram_featurizer(py::module& m) {
    py::class_<NGramFeaturizer, std::shared_ptr<NGramFeaturizer>>(m, "NGramFeaturizer",
                                                                  R"pbdoc(
            NGramFeaturizer extracts ngram features given a string token list.
            )pbdoc")
        .def(py::init<int, bool>(), py::arg("n"), py::arg("boundaries"),
             R"pbdoc(
                Create a NGramFeaturizer object.

                Args:
                    n (int): The n gram token length, valid numbers are [1, 8].
                    boundaries (bool): Capture query boundaries or not. 
                        If True, the ngrams at start or end of the sentence are treated
                        as additional features.
             )pbdoc")
        .def("fit", &NGramFeaturizer::Fit, py::arg("tokens"),
             R"pbdoc(
                Build ngrams from the token list and add them to ngrams list if not already seen.

                Args:
                    tokens (List[str]): The token list for collecting new ngrams.
             )pbdoc")
        .def("transform", &NGramFeaturizer::Transform, py::arg("tokens"),
             R"pbdoc(
                Extract ngrams given the token list based on known ngrams.

                Args:
                    tokens (List[str]): The token list.
             )pbdoc")
        .def("load_ngram", &NGramFeaturizer::LoadNGram, py::arg("ngram_file"),
             R"pbdoc(
                Load ngram list from file.

                The ngram file should contains two columns, separated by WHITESPACE or TABULAR characters. 
                The first and second columns are ngram words and assigned ids correspondingly. For example,

                ::

                    the answer 0
                    answer is 1
                
                Ensure the file is encoded in utf-8.

                Args:
                    ngram_file (str): The source ngram file.
             )pbdoc")
        .def("dump_ngram", &NGramFeaturizer::DumpNGram,
             R"pbdoc(
                Save ngram words to file.

                Args:
                    ngram_file (str): The target ngram file.
             )pbdoc")
        .def(py::pickle(
            [](NGramFeaturizer& self) {
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                std::shared_ptr<NGramFeaturizer> obj =
                    ModelContext::GetActive()->GetOrCreateObject<NGramFeaturizer>(state);
                return obj;
            }));
}

void init_regex_featurizer(py::module& m) {
    py::class_<RegexFeaturizer, std::shared_ptr<RegexFeaturizer>>(m, "RegexFeaturizer",
                                                                  R"pbdoc(
            RegexFeaturizer extracts token spans that match regex patterns specified.
            )pbdoc")
        .def(py::init<const std::vector<std::string>&>(), py::arg("regexes"),
             R"pbdoc(
                Create a RegexFeaturizer object.

                Args:
                    regexes (List[str]): Regex patterns for matching. Each pattern will 
                        be assigned an id. The id is its index in the list, starting from 0.
             )pbdoc")
        .def("add_regex", &RegexFeaturizer::AddRegex, py::arg("regex"),
             R"pbdoc(
                Add a new regex pattern.

                Args:
                    regex (str): The new regex pattern.
             )pbdoc")
        .def("transform", &RegexFeaturizer::Transform, py::arg("tokens"),
             R"pbdoc(
                Extract token spans that match regex patterns specified.

                Args:
                    tokens (List[str]): The token list.
             )pbdoc")
        .def(py::pickle(
            [](RegexFeaturizer& self) {
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                std::shared_ptr<RegexFeaturizer> obj = std::make_shared<RegexFeaturizer>();
                obj->Deserialize(state, ModelContext::GetActive()->Storage());
                return obj;
            }));
}

void init_text_feature_concat(py::module& m) {
    py::class_<TextFeatureConcat, std::shared_ptr<TextFeatureConcat>>(m, "TextFeatureConcat",
                                                                      R"pbdoc(
            Concatenate TextFeatures from different feature spaces(groups) into a single linear feature space.
            )pbdoc")
        .def(py::init<uint64_t>(), py::arg("start_id"),
             R"pbdoc(
                Create a TextFeatureConcat object.

                Args:
                    start_id (int): The starting feature id in the target feature space. 
                        Usually, you should set it to 0.
                        For libsvm/liblinear, 0 is illegal id. You should set it to 1.
             )pbdoc")
        .def("fit", &TextFeatureConcat::Fit, py::arg("feature_groups"),
             R"pbdoc(
                Collect new features from features from a series of feaure spaces(groups).

                For example, the feature group 0 provides TextFeature 0 and TextFeature 1,
                the feature space 1 provides TextFeature 0 and TextFeature 1.
                In the contatenated feature space, 4 features will be created:

                ::

                    (group 0, TextFeature 0) mapped to TextFeature 0
                    (group 0, TextFeature 1) mapped to TextFeature 1
                    (group 1, TextFeature 0) mapped to TextFeature 2
                    (group 1, TextFeature 1) mapped to TextFeature 3

                Args:
                    feature_groups (List[List[TextFeature]]): The feaures from all feature spaces.
             )pbdoc")
        .def("transform", &TextFeatureConcat::Transform, py::arg("feature_groups"),
             R"pbdoc(
                Collect features that are already seen given features from all feature spaces.

                Args:
                    feature_groups (List[List[TextFeature]]): The feaures from all feature spaces.
             )pbdoc")
        .def(py::pickle(
            [](TextFeatureConcat& self) {
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                std::shared_ptr<TextFeatureConcat> obj = std::make_shared<TextFeatureConcat>();
                obj->Deserialize(state, ModelContext::GetActive()->Storage());
                return obj;
            }));
}

}  // namespace python
}  // namespace pyis
