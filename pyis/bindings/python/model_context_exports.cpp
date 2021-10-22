// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pyis/share/model_context.h"

namespace pyis {
namespace python {

namespace py = pybind11;

void init_model_context(py::module& m) {
    py::class_<ModelContext, std::shared_ptr<ModelContext>>(m, "ModelContext")
        .def(py::init<std::string, std::string>(), py::arg("model_path"), py::arg("data_archive") = "",
             R"pbdoc(
                Create a ModelContext for loading and saving external data files.

                The data file should contains two columns, separated by WHITESPACE characters. 
                The first and second columns are source words and target words correspondingly. For example,

                Args:
                    model_path (str): The model file path.
                    data_archive (str): If specified, the external data files will be archived into
                    a single archive file. By default, external files are not archived and live in the same 
                    directory as model file.
            )pbdoc")
        .def("set_file_prefix", &ModelContext::SetFilePrefix, py::arg("prefix"),
             R"pbdoc(
                Add a common prefix to model file and all external data files. It is useful to avoid file path
                conflicts.

                If the model file name already starts with the prefix, the prefix will be ignored. But it still works 
                for external files.

                Args:
                    prefix (str): Common prefix.
            )pbdoc")
        .def_static("activate", &ModelContext::Activate)
        .def_static("deactivate", &ModelContext::Deactivate);
}

}  // namespace python
}  // namespace pyis
