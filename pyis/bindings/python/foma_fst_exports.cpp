#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <fstream>

#include "pyis/ops/text/foma_fst.h"

namespace pyis {
namespace python {

using pyis::ops::FomaFst;

namespace py = pybind11;

void init_foma_fst(py::module& m) {
    py::class_<FomaFst, std::shared_ptr<FomaFst>>(m, "FomaFst",
                                                  R"pbdoc(
			FomaFst implements finite state transducer based on open-sourced library foma.

		)pbdoc")
        .def(py::init<std::string>(), py::arg("fst_path"),
             R"pbdoc(
                Create a FomaFst instance

                Args:
                    fst_path (str): path to the pre-built foma fst file

            )pbdoc")
        .def_static("compile_from_str", &FomaFst::CompileFromStr, py::arg("infile_str"),
                    R"pbdoc(
                Compile a foma script from a string

                Args:
                    infile_str (str): foma script to be compiled.
            )pbdoc")
        .def_static("compile_from_file", &FomaFst::CompileFromFile, py::arg("infile"),
                    R"pbdoc(
                Compile a foma script from a file

                Args:
                    infile (str): foma script path to be compiled.
            )pbdoc")
        .def("apply_down", &FomaFst::ApplyDown, py::arg("query"), py::return_value_policy::move,
             R"pbdoc(
            Apply input query to the pre-built foma fst

            Args:
                    query (str): input query to be applied to fst
            
            Returns:
                    transduced query (str)

        )pbdoc");
}

}  // namespace python
}  // namespace pyis