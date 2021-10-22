// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <fstream>

#include "pyis/ops/text/cedar_trie.h"
#include "pyis/share/model_context.h"

namespace pyis {
namespace python {

using pyis::ops::CedarTrie;

namespace py = pybind11;

void init_cedar_trie(py::module& m) {
    py::class_<CedarTrie, std::shared_ptr<CedarTrie>>(m, "CedarTrie",
                                                      R"pbdoc(
			CedarTrie implements a dual-array trie tree with speedy update support.

		)pbdoc")
        .def(py::init<>())
        .def(
            "lookup",
            [](CedarTrie& self, const std::string& key) {
                auto result = self.Lookup(key);
                if (result.has_error()) {
                    throw result.error();
                }
                return result.value();
            },
            py::arg("key"),
            R"pbdoc(
                Look up a specific key in trie. Returns the corresponding value if
                key-value pair exists.

                A RuntimeError exception will be thrown if the key is not in the trie tree.

                Args:
                    key (str): The key to look up.

                Returns:
                    The corresponding value of the queried key.
            )pbdoc")
        .def("contains", &CedarTrie::Contains, py::arg("key"),
             R"pbdoc(
                Check if the key exists in the trie.

                Returns:
                    True if the key exists, otherwise False.
            )pbdoc")
        .def("items", &CedarTrie::Items, R"pbdoc(
                Dump all key-value pair in trie into a list of tuple. Each tuple stores a 
                pair of key and value.

                Returns:
                    A list stores all key-value pair in the trie.
            )pbdoc")
        .def("predict", &CedarTrie::Predict, py::arg("prefix"),
             R"pbdoc(
                Find all key-value pair which the key starts with a specific prefix.

                Args:
                    prefix (str): The specific prefix.

                Returns:
                    A list stores key-value pair(s).
            )pbdoc")
        .def("prefix", &CedarTrie::Prefix, py::arg("query"),
             R"pbdoc(
                Find all key-value pair which the key is prefix of a specific string

                Args:
                    query (str): The specific string.
                
                Returns:
                    A list stores key-value pair(s).
            )pbdoc")
        .def(
            "longest_prefix",
            [](CedarTrie& self, const std::string& query) {
                auto result = self.LongestPrefix(query);
                if (result.has_error()) {
                    throw result.error();
                }
                return result.value();
            },
            py::arg("query"),
            R"pbdoc(
                Find a key-value pair which the key is the longest prefix of a specific string.
                A RuntimeError exception will be thrown if no key in trie is the prefix of query.

                For example, a trie with folloing key-value pairs.
                    
                ::
                
                    Alpha 1
                    Beta 2
                    Delta 3
                    AlphaBeta 4

                Will return ('AlphaBeta', 4) for longest_prefix('AlphaBetaDelta')

                Args:
                    query (str): The specific string

                Returns:
                    A tuple of key-value pair.
            )pbdoc")
        .def("erase", &CedarTrie::Erase, py::arg("key"), R"pbdoc(
                Remove a key-value pair from trie. Returns -1 if the key not exists. Returns 0 if
                the pair is successfully removed.
                
                Args:
                    key (str): The key to be removed.
            
                Returns:
                    int, 0 for removed, -1 for key not exists.
            )pbdoc")
        .def("insert", &CedarTrie::Insert, py::arg("key"), py::arg("value") = 0,
             R"pbdoc(
                Insert a key-value pair into trie. Returns -1 if the key is already exists (In this
                case, the value will be updated), or 0 for inserted key-value pair.

                Value should be in range [INT32_MIN+2, INT32_MAX]. INT32_MIN and INT32_MIN+1 are reserved values
                and you should never use them.

                Args:
                    key (str): The key string
                    val (int): The associated value.

                Returns:
                    int, 0 for inserted, -1 for updated, 1 if value is reserved.
            )pbdoc")
        .def("numkeys", &CedarTrie::NumKeys, R"pbdoc(
                Get the key-value pair count in the trie.
                
                Returns:
                    int, indicates the number of key-value pair in the trie.
            )pbdoc")
        .def(
            "load",
            [](CedarTrie& self, const std::string& path) {
                auto result = self.Open(path);
                if (result.has_error()) {
                    throw result.error();
                }
            },
            py::arg("path"),
            R"pbdoc(
                Load a binary trie file from given path. Throws RuntimeError exception if failed to
                load from file.
        
                Args:
                    path (str): The trie file path.
            )pbdoc")
        .def(
            "save",
            [](CedarTrie& self, const std::string& path) {
                auto result = self.Save(path);
                if (result.has_error()) {
                    throw result.error();
                }
            },
            py::arg("path"),
            R"pbdoc(
                Store the trie into a binary trie file. Throws RuntimeError exception if failed to
                save to file.
            
                Args:
                    path (str): The trie file path;
            )pbdoc")
        .def("build", &CedarTrie::Build, py::arg("data"), R"pbdoc(
                Insert/Update all key-value pair in a given list.
                
                Args:
                    data (List[Tuple[str, int]]): key-value pairs to be inserted

                Returns:
                    Number of key-value pairs inserted.
            )pbdoc")
        .def(
            "build_from_file",
            [](CedarTrie& self, const std::string& path) {
                auto result = self.BuildFromFile(path);
                if (result.has_error()) {
                    throw result.error();
                }
                return result.value();
            },
            py::arg("path"),
            R"pbdoc(
                Insert/Update all key-value pair in a given file.
                
                The data file should contains two columns, separated by seperators.
                The first and second columns are key and value correspondingly. 
                The row missing the second column will have a default value 0.

                Throws RuntimeError exception if failed to read the given file.

                For example
        
                ::
                
                    Alpha 1
                    Beta 2
                    Delta 3
                    AlphaBeta
            
                (AlphaBeta row has the default value 0)

                Args:
                    path (str): The path of given file.

                Returns:
                    Number of key-value pairs inserted.
            )pbdoc")
        .def("reset", &CedarTrie::Reset, R"pbdoc(
                Reset the trie to the empty state.
            )pbdoc")
        .def(py::pickle(
            [](CedarTrie& self) {
                // __getstate__
                // return a bytes array that fully encodes the state of the object
                std::string state = self.Serialize(ModelContext::GetActive()->Storage());
                return py::bytes(state);
            },
            [](py::bytes& state) {
                // __setstate__
                // create a new C++ instance from the state and files(optional) saved above
                std::shared_ptr<CedarTrie> obj = ModelContext::GetActive()->GetOrCreateObject<CedarTrie>(state);
                return obj;
            }));
}

}  // namespace python
}  // namespace pyis