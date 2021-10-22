# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import unittest
import onnxruntime as ort
import numpy as np
import inspect
import ast
from textwrap import dedent
from pyis.onnx.transpiler.passes.onnx_gen import OnnxGen
from pyis.onnx.transpiler.passes.type_infer import TypeInfer


class TestAddInt(unittest.TestCase):

    class IntPlus:
        def forward(self, x: int, y: int) -> int:
            r1:int = x + y
            r2:int = x + y
            return (r1, r2)

    def test_add(self):
        source = dedent(dedent(inspect.getsource(TestAddInt.IntPlus.forward)))
        t = ast.parse(source)
        # astpretty.pprint(t, indent=4, show_offsets=True)

        m = TestAddInt.IntPlus()
        p1 = TypeInfer(m, t)
        p1.run()
        p1.print_type_dict()
        p2 = OnnxGen(m, t, p1.get_type_dict(), p1.get_function_calls())
        p2.run()
        model_file = './tmp/exported_model.onnx'
        p2.save_onnx_graph(model_file)

        sess = ort.InferenceSession(model_file)
        x = np.array([3], dtype=np.int64)
        y = np.array([2], dtype=np.int64)
        z = sess.run(['r1', 'r2'], {'x':x, 'y':y})
        self.assertListEqual(z, [5, 5])


if __name__ == '__main__':
    unittest.main()
