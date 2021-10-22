# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from onnx import helper
from onnx import AttributeProto, TensorProto, GraphProto
from pyis.onnx.transpiler.onnx_graph.onnx_graph import OnnxGraph
from pyis.onnx.transpiler.onnx_graph.extractor import Extractor


def NP
def add(graph:OnnxGraph, left, right, res):
    left_t, right_t = left[1], right[1]
    if left_t is not right_t:
        raise TypeError('invalid type')

    return graph.create_node('Add', [left, right], [res])

def ort_session(graph: OnnxGraph, ort_session_op:, inputs, outputs):
    
