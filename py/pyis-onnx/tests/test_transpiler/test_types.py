# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from typing import List
import unittest
import inspect
import ast
from textwrap import dedent
from typing import List
from pyis.onnx.transpiler.types import Types


class TestSymType(unittest.TestCase):

    @staticmethod
    def add(a:List[int], b:List[int]) -> List[int]:
        return a + b
    
    def test_add(self):
        ast_tree = ast.parse(dedent(inspect.getsource(TestSymType.add)))
        py_type = Types.annotation_to_py(ast_tree.body[0].args.args[0].annotation)
        self.assertIs(py_type, List[int])

if __name__ == '__main__':
    unittest.main()
