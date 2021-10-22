# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import unittest
import numpy as np
from onnx import helper, onnx_pb as onnx_proto
import onnxruntime as ort
from pyis.onnx import get_onnx_extensions_lib


def create_test_model(input_dims):
    nodes = []
    nodes[2:] = [helper.make_node(
        'NaiveLanguageDetector', ['text'], ['output'], domain='pyis')]

    text = helper.make_tensor_value_info(
        'text', onnx_proto.TensorProto.STRING, [None])
    output = helper.make_tensor_value_info(
        'output', onnx_proto.TensorProto.INT64, [None])

    graph = helper.make_graph(nodes, 'test0', [text], [output])
    model = helper.make_model(
        graph, opset_imports=[helper.make_operatorsetid("", 12)])
    return model


def run_language_detector(input1, expect_result):
    model = create_test_model(input1.ndim)

    options = ort.SessionOptions()
    options.register_custom_ops_library(get_onnx_extensions_lib())
    sess = ort.InferenceSession(model.SerializeToString(), options)
    result = sess.run(None, {'text': input1})

    np.testing.assert_array_equal(result, [expect_result])


class TestLanguageDetector(unittest.TestCase):
    
    @classmethod
    def setUpClass(cls):
        pass

    @unittest.skip("test failed, fix it @haojin")
    def test_language_detector(self):
        run_language_detector(np.array(["a"]), np.array([0], dtype=np.int64))


if __name__ == "__main__":
    unittest.main()
