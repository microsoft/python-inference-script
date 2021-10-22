// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pyis/ops/text/immutable_trie.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace python {

using pyis::ops::ImmutableTrie;

namespace py = pybind11;

void init_immutable_trie(py::module& m) {
    py::class_<ImmutableTrie, std::shared_ptr<ImmutableTrie>>(m, "ImmutableTrie",
                                                              R"pbdoc(
			An Implementation of compile-stype immutable trie.

		)pbdoc")
        .def(py::init<>(), R"pbdoc(
                Constructs an empty immutable trie.
        )pbdoc")
        .def("load_items", &ImmutableTrie::LoadItems, py::arg("data"), R"pbdoc(
                Re-construct the trie with given data.

                Args:
                    data (List[Tuple[str, int]]): Key-value pairs to construct the new trie.
        )pbdoc")
        .def(
            "load",
            [](ImmutableTrie& self, const std::string& path) {
                auto result = self.Load(path);
                if (result.has_error()) {
                    throw result.error();
                }
            },
            py::arg("path"), R"pbdoc(
                Load the trie from a pre-compiled binary file.
        
                Args:
                    path (str): Binary file path.
        )pbdoc")
        .def("items", &ImmutableTrie::Items, R"pbdoc(
                Dump all key-value pairs from the trie.

                Returns:
                    data (List[Tuple[str, int]]): All key-value pairs in the trie.
        )pbdoc")
        .def(
            "lookup",
            [](ImmutableTrie& self, const std::string& key) {
                auto result = self.Match(key);
                if (result.has_error()) {
                    throw result.error();
                }
                return result.value();
            },
            py::arg("key"),
            R"pbdoc(
                Look up a specific key from the trie. Return the corrsponding value if found. Throws RuntimeError
                if trie is not initialized or the key not found.

                Args:
                    key (str): The specific key to look up.

                Returns:
                    value (int): The corrsponding value.
        )pbdoc")
        .def("contains", &ImmutableTrie::Contains, py::arg("key"), R"pbdoc(
                Look for a specific key.
        
                Args:
                    key (str): The specific key to look up.
    
                Returns:
                    result (bool) : True if the trie contains the key, False otherwise.
        )pbdoc")
        .def_static(
            "compile",
            [](const std::vector<std::tuple<std::string, uint32_t>>& data, const std::string& path) {
                auto result = ImmutableTrie::Compile(data, path);
                if (result.has_error()) {
                    throw result.error();
                }
            },
            py::arg("data"), py::arg("path"),
            R"pbdoc(
                Compile a list of key-value pairs into a immutable trie file. This process could costs lots of memory.
                Values in the list should not exceeded the presenting capacity of unsigned 32bit integer.
                Throws RuntimeError if compile process could not finish (e.g. file cannot write).

                Args:
                    data (List[Tuple[str, int]]): The key-value pairs to be compiled.
                    path (str): The path the compiled file to be stored.
                
        )pbdoc")
        .def(py::pickle(
            [](ImmutableTrie& self) {
                // __getstate__
                // return a bytes array that fully encodes the state of the object
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                // __setstate__
                // create a new C++ instance from the state and files(optional) saved above
                std::shared_ptr<ImmutableTrie> obj = ModelContext::GetActive()->GetOrCreateObject<ImmutableTrie>(state);
                return obj;
            }));
}

}  // namespace python
}  // namespace pyis