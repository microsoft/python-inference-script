// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pyis/ops/linear_svm/linear_svm.h"
#include "pyis/share/model_context.h"
#include "pyis/share/scope_guard.h"

namespace pyis {
namespace python {

using pyis::ops::LinearSVM;

namespace py = pybind11;

void init_linear_svm(py::module& m) {
    py::class_<LinearSVM, std::shared_ptr<LinearSVM>>(m, "LinearSVM",
                                                      R"pbdoc(
            A support vector machine implementation using liblinear.

        )pbdoc")
        .def(py::init<std::string>(), py::arg("model_file"),
             R"pbdoc(
                Create a LinearSVM object given the model file in libsvm format.

                Args:
                    model_file (str): The model file in `libsvm format <https://www.csie.ntu.edu.tw/~cjlin/libsvm/faq.html#f402>`_.
            )pbdoc")
        .def("predict", &LinearSVM::Predict, py::arg("features"),
             R"pbdoc(
                Given a list of features triggered by the input sample, return the decision values for each of the classes.

                Args:
                    features (List[Tuple[int, double]]): List of features, each is represented by 
                        a tuple of (feature id, feature value). Don't repeat two identical feature ids
                        in the list.
               
                Returns:
                    List of decision values.
            )pbdoc")
        .def_static("train", &LinearSVM::Train, py::arg("libsvm_data_file"), py::arg("model_file"),
                    py::arg("solver_type") = 5, py::arg("eps") = 0.1, py::arg("C") = 1.0, py::arg("p") = 0.5,
                    py::arg("bias") = 1.0,
                    R"pbdoc(
                Train a SVM model using liblinear.

                Args:
                    libsvm_data_file (str): Training data in 
                        `libsvm data format <https://www.csie.ntu.edu.tw/~cjlin/libsvm/faq.html#/Q3:_Data_preparation>`_. 
                    model_file (str): Target file for the generated model file.
                    solver_type (int): L2R_LR(0), L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC, L2R_L1LOSS_SVC_DUAL, 
                        MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR, L2R_LR_DUAL, L2R_L2LOSS_SVR = 11, 
                        L2R_L2LOSS_SVR_DUAL, L2R_L1LOSS_SVR_DUAL, ONECLASS_SVM = 21
                    eps (float): Stopping criteria.
                    c (float): Regularization parameter.
                    p (float): Epsilon parameter (for Epsilon_SVR).
                    bias (float): If non negative then each instance is appended a constant bias term.
            )pbdoc")
        .def(py::pickle(
            [](LinearSVM& self) {
                // __getstate__
                // return a bytes array that fully encodes the state of the object
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                // __setstate__
                // create a new C++ instance from the state and files(optional) saved above
                std::shared_ptr<LinearSVM> obj = ModelContext::GetActive()->GetOrCreateObject<LinearSVM>(state);
                return obj;
            }));
}

}  // namespace python
}  // namespace pyis
