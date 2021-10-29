# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import unittest, os
from pathlib import Path
from unittest.case import skip
import numpy as np
from pyis.python import ops 
from pyis.python import save, load

@unittest.skip("local dependencies and already verified")
class TestOrtSessionWithCustomOrtDll(unittest.TestCase):
    def setUp(self) -> None:
        os.makedirs('tmp', exist_ok=True)
        # initialize model
        self.model_path = os.path.join(os.path.dirname(__file__), 'data', 'a_plus_b.onnx')
        custom_dll_path = str(Path(__file__).parent / 'customdll' / 'libonnxruntime.so.1.8.0')
        ops.OrtSession.initialize_ort(custom_dll_path)
        self.ort_session: ops.OrtSession = ops.OrtSession(
            model_path = self.model_path,
            input_names = ['x', 'y'],
            output_names = ['r1', 'r2']
        )

    def test(self):
        i1 = np.array([1], dtype=np.int64)
        i2 = np.array([2], dtype=np.int64)
        o1, o2 = self.ort_session.run([i1, i2])
        x = o1.tolist()
        y = o2.tolist()

class TestOrtSessionSimple(unittest.TestCase):
    def setUp(self) -> None:
        ops.OrtSession.initialize_ort()
        # initialize model
        self.model_path = os.path.join(os.path.dirname(__file__), 'data', 'a_plus_b.onnx')
        self.ort_session: ops.OrtSession = ops.OrtSession(
            model_path = self.model_path,
            input_names = ['x', 'y'],
            output_names = ['r1', 'r2']
        )

    def test_run(self):
        i1 = np.array([1], dtype=np.int64)
        i2 = np.array([2], dtype=np.int64)
        o1, o2 = self.ort_session.run([i1, i2])
        x, y = o1.tolist(), o2.tolist()
        self.assertListEqual(x, [3])
        self.assertListEqual(y, [3])
        
    def test_save(self):
        save(self.ort_session, 'tmp/test_ort_session/model.pkl')
        loaded_session = load('tmp/test_ort_session/model.pkl')
        i1 = np.array([1], dtype=np.int64)
        i2 = np.array([2], dtype=np.int64)
        o1, o2 = loaded_session.run([i1, i2])
        x, y = o1.tolist(), o2.tolist()
        self.assertListEqual(x, [3])
        self.assertListEqual(y, [3])

if __name__ == "__main__":
    unittest.main()
