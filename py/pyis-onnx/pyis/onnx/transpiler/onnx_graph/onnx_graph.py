# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
from typing import Dict, List, Tuple
import onnx
from onnx import helper
from onnx.onnx_ml_pb2 import ValueInfoProto
from pyis.onnx.transpiler.types import Types
from pathlib import Path

# https://github.com/onnx/onnx/blob/master/docs/PythonAPIOverview.md

class OnnxGraph:
    def __init__(self) -> None:
        self.onnx_input_vars: Dict[str, ValueInfoProto] = {}
        self.onnx_output_vars: Dict[str, ValueInfoProto] = {}
        self.onnx_nodes = []

    def _get_or_create_input_var(self, name:str, type):
        if name in self.onnx_output_vars:
            return self.onnx_output_vars[name]

        if name in self.onnx_input_vars:
            return self.onnx_input_vars[name]
        
        var = helper.make_tensor_value_info(name, Types.py_to_tensor(type), [1])
        self.onnx_input_vars[name] = var

        return var

    def _get_or_create_output_var(self, name:str, type):
        if name in self.onnx_output_vars:
            raise RuntimeError(f'variable {name} redefined')

        if name in self.onnx_input_vars:
            raise RuntimeError(f'variable {name} redefined')
        
        var = helper.make_tensor_value_info(name, Types.py_to_tensor(type), [1])
        self.onnx_output_vars[name] = var

        return var
    
    def create_node(self, onnx_op_name,
                    inputs: List[Tuple[str, object]],  # object is py type hint
                    outputs: List[Tuple[str, object]]):
        for i in inputs:
            self._get_or_create_input_var(*i)
        for i in outputs:
            self._get_or_create_output_var(*i)
        onnx_node = helper.make_node(onnx_op_name, [i[0] for i in inputs], [i[0] for i in outputs])
        self.onnx_nodes.append(onnx_node)
        return onnx_node

    def save(self, model_file: str, output_names: List[str]):
        model_name = Path(model_file).stem
        model_dir = os.path.dirname(model_file)   
        graph_def = helper.make_graph(
            self.onnx_nodes,                                    # type: List[NodeProto]
            model_name,
            self.onnx_input_vars.values(),                      # type: List[ValueInfoProto]
            [self.onnx_output_vars[i] for i in output_names]    # type: List[ValueInfoProto]
            )
        model_def = helper.make_model(graph_def, producer_name='pyis')
        onnx.checker.check_model(model_def)
        os.makedirs(model_dir, exist_ok=True)
        onnx.save_model(model_def, model_file)
