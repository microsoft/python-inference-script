// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <algorithm>
#include <cstdlib>

#include "pybind11/numpy.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pyis/ops/ort_session/ort_session.h"

namespace pyis {
namespace python {

namespace py = pybind11;
using pyis::ops::OrtSession;

template <typename T>
inline py::array_t<T> ConvertToNumpyArray(const std::shared_ptr<Ort::Value>& tensor) {
    auto tensor_shape = tensor->GetTensorTypeAndShapeInfo().GetShape();
    auto elem_cnt = tensor->GetTensorTypeAndShapeInfo().GetElementCount();
    T* tensor_data_ptr = tensor->GetTensorMutableData<T>();
    std::vector<ssize_t> py_arr_shape;
    py_arr_shape.resize(tensor_shape.size());
    std::transform(tensor_shape.begin(), tensor_shape.end(), py_arr_shape.begin(),
                   [](int64_t elem) { return static_cast<ssize_t>(elem); });
    auto ret = py::array_t<T>(py_arr_shape);
    auto buf = ret.request();
    T* data_ptr = static_cast<T*>(buf.ptr);
    for (int i = 0; i < elem_cnt; i++) {
        data_ptr[i] = tensor_data_ptr[i];
    }
    return ret;
}

inline ONNXTensorElementDataType get_data_type(py::buffer_info& buffer_info) {
    if (buffer_info.format == "q" || buffer_info.format == "l") {
        return ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
    }
    if (buffer_info.format == py::format_descriptor<float>::format()) {
        return ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
    }
    if (buffer_info.format == py::format_descriptor<int>::format()) {
        return ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
    }
    if (buffer_info.format == py::format_descriptor<double>::format()) {
        return ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE;
    }
    if (buffer_info.format == py::format_descriptor<bool>::format()) {
        return ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL;
    }
    throw std::runtime_error("Data type not supported yet");
}

inline std::shared_ptr<Ort::Value> ToOrtTensor(py::buffer_info& buffer_info) {
    std::vector<int64_t> shape(buffer_info.ndim);
    transform(buffer_info.shape.begin(), buffer_info.shape.end(), shape.begin(),
              [](py::ssize_t elem) -> int64_t { return static_cast<int64_t>(elem); });
    auto ndim = static_cast<size_t>(buffer_info.ndim);

    auto size = static_cast<size_t>(buffer_info.size);

    auto data_type = get_data_type(buffer_info);
    auto tensor_ptr =
        std::make_shared<Ort::Value>(Ort::Value::CreateTensor(*OrtGlobals::Allocator, shape.data(), ndim, data_type));

    memcpy(tensor_ptr->GetTensorMutableData<void>(), buffer_info.ptr,
           size * pyis::ops::GetTensorElementBytes(*tensor_ptr));

    return tensor_ptr;
}

void init_ort_session(py::module& m) {
    py::class_<OrtSession, std::shared_ptr<OrtSession>>(m, "OrtSession",
                                                        R"pbdoc(
            Augmented OrtSession ONNX Runtime Session

        )pbdoc")
        .def(py::init<std::string&, std::vector<std::string>&, std::vector<std::string>&, int, int, bool, int>(),
             py::arg("model_path"), py::arg("input_names"), py::arg("output_names"), py::arg("inter_op_thread_num") = 1,
             py::arg("intra_op_thread_num") = 0, py::arg("dynamic_batching") = false, py::arg("batch_size") = 1,
             R"pbdoc(
                Create an ORT(ONNX Runtime) Session.

                Args:
                    model_file (str): path to the onnx model,
                    input_names (List[str]): input names to the onnx model,
                    output_names (List[str]): output names to the onnx model,
                    inter_op_thread_num (int): inter-op thread num, default to 1,
                    intra_op_thread_num (int): intra-op thread num, default to 0, use all cores
                    dynamic_batching (bool): use dynamic batching or not,
                    batch_size (int): dynamic batch size

            )pbdoc")
        .def_static("initialize_ort", &OrtSession::InitializeOrt, py::arg("ort_dll_file") = "",
                    R"pbdoc(
                Initialize ORT (ONNX Runtime) dynamically
                
                Args:
                    ort_dll_file (str): the filename of ort dll file. Make sure the specified ort dll with all its dependencies are in the search path

            )pbdoc")
        .def(
            "run",
            [](OrtSession& self, std::vector<py::array>& inputs) -> std::vector<py::array> {
                std::vector<std::shared_ptr<Ort::Value>> input_tensors;
                for (const auto& buffer : inputs) {
                    auto buffer_info = buffer.request();
                    input_tensors.emplace_back(ToOrtTensor(buffer_info));
                }
                py::gil_scoped_release release;
                auto outputs = self.Run(input_tensors);
                py::gil_scoped_acquire acquire;

                std::vector<py::array> ret;
                ret.reserve(outputs.size());

                for (const auto& output : outputs) {
                    switch (output->GetTensorTypeAndShapeInfo().GetElementType()) {
                        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
                            ret.emplace_back(ConvertToNumpyArray<float>(output));
                            break;
                        case ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
                            ret.emplace_back(ConvertToNumpyArray<bool>(output));
                            break;
                        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
                            ret.emplace_back(ConvertToNumpyArray<int64_t>(output));
                            break;
                        case ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
                            ret.emplace_back(ConvertToNumpyArray<double>(output));
                            break;
                        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
                            ret.emplace_back(ConvertToNumpyArray<int>(output));
                            break;
                        default:
                            break;
                    }
                }
                return ret;
            },
            R"pbdoc(
            Run Ort Session with input tensors as list of numpy array.

            Args:
                    model_path (List[numpy.ndarray]): input tensors as list of numpy array.
            
            Returns:
                    output tensors as list of numpy array


        )pbdoc")
        .def(py::pickle(
            [](OrtSession& self) {
                // __getstate__
                // return a bytes array that fully encodes the state of the object
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                // __setstate__
                // create a new C++ instance from the state and files(optional) saved above
                std::shared_ptr<OrtSession> obj = ModelContext::GetActive()->GetOrCreateObject<OrtSession>(state);
                return obj;
            }));
}

}  // namespace python
}  // namespace pyis
