# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
import ast
import onnx
from pyis.onnx.ops.ort_session import OrtSession
from pyis.onnx.transpiler.function_call import FunctionCall
from pyis.onnx.transpiler.onnx_graph.onnx_graph import OnnxGraph
from pyis.onnx.transpiler.onnx_graph.ops import add
from pyis.onnx.fmt_error import fmt_error
from pyis.onnx.transpiler.types import Types
from pyis.onnx.transpiler.onnx_graph.ops import ort_session


class OnnxGen(ast.NodeVisitor):

    def __init__(self, obj, tree: ast.AST, type_dict, function_calls):
        self.tree = tree
        self.obj = obj
        self.type_dict = type_dict
        self.function_calls = function_calls
        self.onnx_graph = OnnxGraph()
        self.output_names = []
    
    def run(self):
        self.visit(self.tree)

    def get_type(self, var):
        return self.type_dict[var]

    def visit_BinOp(self, node: ast.BinOp):
        op = node.op
        inputs = [node.left, node.right]
        return (op, inputs)

    def visit_Assign(self, node: ast.Assign):
        op, inputs = self.visit(node.value)
        output = node.targets[0]
        if isinstance(op, ast.Add):
            self.process_Add(op, inputs, output)
    
    def visit_AnnAssign(self, node: ast.AnnAssign):
        print(f'line {node.lineno}')
        op, inputs = self.visit(node.value)
        output = node.target
        if isinstance(op, ast.Add):
            self.process_Add(op, inputs, output)

    def visit_Return(self, node: ast.Return):
        if isinstance(node.value, ast.Tuple):
            for i in node.value.elts:
                self.output_names.append(i.id)
        elif isinstance(node.value, ast.Name):
            self.output_names.append(node.value.id)
        else:
            raise RuntimeError(f'return value could only be a named variable or a tuple of names variables')

    def process_Add(self, op, inputs, output):
        left, right = inputs[0], inputs[1]
        add(self.onnx_graph, 
            (left.id, self.get_type(left.id)),
            (right.id, self.get_type(right.id)),
            (output.id, self.get_type(output.id))
            )

    def visit_Call(self, node: ast.Call):
        call: FunctionCall = self.function_calls[node]
        if call.is_global_function():
            return
        if call.class_type is OrtSession:
            ort_session(None, call.class_instance, None, None)
    
    def save_onnx_graph(self, model_file: str):       
        self.onnx_graph.save(model_file, self.output_names)
