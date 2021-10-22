# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import unittest
import ast
from textwrap import dedent
import inspect
from typing import List, Tuple
from pyis.onnx.transpiler.passes.type_infer import TypeInfer
from pyis.onnx.transpiler.ast_printer import pformat_ast

class SubModel:
    def __init__(self) -> None:
        pass
    
    def run(self, q: str) -> Tuple[str, List[int]]:
        return ('hello', [1, 2, 3])

class Model2:
    def __init__(self) -> None:
         self.sub_model = SubModel()
    
    def run(self, q: str):
        q_copy = q
        q_copy1, q_copy2 = q, q
        int1 = 42
        float1 = 4.2
        str1 = '42'
        o1, o2 = self.sub_model.run(str1)
        return (o1, o2)

class TestResolveFunction(unittest.TestCase):
    def test_resolve_function_call(self):
        source = dedent(dedent(inspect.getsource(Model2.run)))
        t = ast.parse(source)
        m = Model2()
        pass1 = TypeInfer(m, t)
        pass1.run()
        pass1.print_type_dict()
        self.assertDictEqual(pass1.get_type_dict(), 
            {
                "q": str,
                "q_copy": str,
                "q_copy1": str,
                "q_copy2": str,
                "int1": int,
                "float1": float,
                "str1": str,
                "o1": str,
                "o2": List[int]
            })
        
if __name__ == "__main__":
    unittest.main()
