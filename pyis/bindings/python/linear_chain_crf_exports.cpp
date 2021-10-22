// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pyis/ops/linear_chain_crf/linear_chain_crf.h"
#include "pyis/share/model_context.h"
#include "pyis/share/scope_guard.h"

namespace pyis {
namespace python {

using pyis::ops::LinearChainCRF;

namespace py = pybind11;

void init_linear_chain_crf(py::module& m) {
    py::class_<LinearChainCRF, std::shared_ptr<LinearChainCRF>>(m, "LinearChainCRF",
                                                                R"pbdoc(
            A linear chain conditional random field(CRF) implementation.

        )pbdoc")
        .def(py::init<std::string>(), py::arg("model_file"),
             R"pbdoc(
                Create a LinearChainCRF object given the model file.

                Args:
                    model_file (str): The model file.
            )pbdoc")
        .def("predict", &LinearChainCRF::Predict, py::arg("len"), py::arg("features"),
             R"pbdoc(
                Given a list of features triggered by the input sample, return the label for each token of the input.

                Args:
                    len (int): token number of the input.
                    features (List[Tuple[int, int, float]]): List of features, each is represented by 
                        a tuple of (token index, feature id, feature value).
               
                Returns:
                    List of labels.
            )pbdoc")
        .def_static("train", &LinearChainCRF::Train, py::arg("data_file"), py::arg("model_file"),
                    py::arg("alg") = "l1sgd", py::arg("max_iter") = 150,
                    R"pbdoc(
                Train a crf model.

                Args:
                    data_file (str): Training data file. 
                    model_file (str): Target file for the generated model file.
                    alg (str): The training algorithm. 
                               perceptron: `Structured Perceptron <https://people.cs.umass.edu/~brenocon/inlp2015/09-discseq-perc.pdf>`_, 
                               l1sgd: `Stochastic Gradient Descent Training for L1-regularized Log-linear Models <https://dl.acm.org/doi/pdf/10.5555/1687878.1687946>`_
                    max_iter (int): Maximun iterations.
            )pbdoc")
        .def(py::pickle(
            [](LinearChainCRF& self) {
                // __getstate__
                // return a bytes array that fully encodes the state of the object
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                // __setstate__
                // create a new C++ instance from the state and files(optional) saved above
                std::shared_ptr<LinearChainCRF> obj =
                    ModelContext::GetActive()->GetOrCreateObject<LinearChainCRF>(state);
                return obj;
            }));
}

}  // namespace python
}  // namespace pyis
