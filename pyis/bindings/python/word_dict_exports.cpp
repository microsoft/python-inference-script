// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pyis/ops/example_op/word_dict.h"
#include "pyis/share/model_context.h"
#include "pyis/share/scope_guard.h"

namespace pyis {
namespace python {

using pyis::ops::WordDict;

namespace py = pybind11;

void init_word_dict(py::module& m) {
    py::class_<WordDict, std::shared_ptr<WordDict>>(m, "WordDict",
                                                    R"pbdoc(
            WordDict maps source words(tokens) of a sentence into target words based on a defined dictionary.

        )pbdoc")
        .def(py::init<std::string>(), py::arg("data_file"),
             R"pbdoc(
                Create a WordDict object given the dictionary data file.

                The data file should contains two columns, separated by WHITESPACE characters. 
                The first and second columns are source words and target words correspondingly. For example,

                ::

                    suzhou  苏州
                    beijing 北京
                
                Ensure the file is encoded in utf-8.

                Args:
                    data_file (str): The dictionary data file for translation.
            )pbdoc")
        .def("translate", &WordDict::Translate, py::arg("tokens"),
             R"pbdoc(
                Given words of a sentence, tranlate each word that appears in the dictionary to 
                the target word.

                If the word doesn't exist in the dictionary, it will be intact and copied to the output
                sentence. 

                Ensure words are encoded in utf-8.

                Args:
                    tokens (List[str]): List of words of the sentence.

                Returns:
                    Target words of the sentence.
            )pbdoc")
        .def(py::pickle(
            [](WordDict& self) {
                // __getstate__
                // return a bytes array that fully encodes the state of the object
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                // __setstate__
                // create a new C++ instance from the state and files(optional) saved above
                std::shared_ptr<WordDict> obj = ModelContext::GetActive()->GetOrCreateObject<WordDict>(state);
                return obj;
            }));
}

}  // namespace python
}  // namespace pyis
