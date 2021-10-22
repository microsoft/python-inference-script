# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import unittest, os
from typing import List, Tuple
import numpy as np
from pyis.onnx import ops
import inspect
import ast
from textwrap import dedent
from pyis.onnx.transpiler.passes.type_infer import TypeInfer
from pyis.onnx.transpiler.passes.onnx_gen import OnnxGen

class Model:
    onnx_file = os.path.join(os.path.dirname(__file__), 'a_plus_b.onnx')

    def __init__(self) -> None:
         self.ort_session = ops.OrtSession(
            model_file = Model.onnx_file,
            input_names = ['x', 'y'],
            output_names = ['r1', 'r2'])
    
    def run(self, x:List[int], y:List[int]) -> Tuple[List[int], List[int]]:
        i1 = np.array(x, dtype=np.int64)
        i2 = np.array(y, dtype=np.int64)
        o1, o2 = self.ort_session.run(i1, i2)
        r1 = o1.tolist()
        r2 = o2.tolist()
        return r1, r2
        
class TestOrtSession(unittest.TestCase):
    def test_run_in_python(self):
        m = Model()
        x, y = m.run([1], [2])
        print((x, y))

    def test_transpile(self):
        source = dedent(dedent(inspect.getsource(Model.run)))
        t = ast.parse(source)

        m = Model()
        p1 = TypeInfer(m, t)
        p1.run()
        p1.print_type_dict()
        p2 = OnnxGen(m, t, p1.get_type_dict(), p1.get_function_calls())
        p2.run()
        
        
if __name__ == "__main__":
    unittest.main()
