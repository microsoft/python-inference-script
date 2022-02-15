# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
from typing import Dict, List, Tuple
import onnx
from onnx import helper
from onnx.onnx_ml_pb2 import ValueInfoProto
from pyis.onnx.transpiler.types import Types

# https://github.com/onnx/onnx/blob/b664eb315757e7aad66dadbdf6731e4e23c284c3/onnx/utils.py

'''
Extract a subgraph from an existing onnx graph
'''

class Extractor:
    def __init__(self, model:onnx.ModelProto,
                       input_names:List[str], 
                       output_names:List[str]):
        self.input_names = input_names
        self.output_names = output_names
        self.model = model
        self.graph = self.model.graph
        self.wmap = {obj.name: obj for obj in self.graph.initializer}
        self.vimap = {obj.name: obj for obj in self.graph.value_info}
        self.inmap = {obj.name: obj for obj in self.graph.input}
        self.outmap = {obj.name: obj for obj in self.graph.output}

        self.inputs, self.outputs = self._collect_inputs_outputs(self.input_names, self.output_names)
        self.nodes = self._collect_reachable_nodes(self.input_names, self.output_names)
        self.initilizers, self.value_infos = self._collect_reachable_tensors(self.nodes)

        for i in self.inputs:
            print(i)

    def _collect_inputs_outputs(
            self, 
            input_names:List[str],
            output_names:List[str]):
        unknown_inputs = set(input_names) - set(self.inmap.keys())
        if len(unknown_inputs) != 0:
            raise RuntimeError(f'unknown inputs {",".join(list(unknown_inputs))} in model {self.model.graph.name}')
        
        unknown_outputs = set(output_names) - set(self.outmap.keys())
        if len(unknown_outputs) != 0:
            raise RuntimeError(f'unknown outputs {",".join(list(unknown_outputs))} in model {self.model.graph.name}')
        
        inputs = [self.inmap[i] for i in input_names if i in self.inmap]
        outputs = [self.outmap[i] for i in output_names if i in self.outmap]

        return (inputs, outputs)

    def _dfs_search_reachable_nodes(
            self,
            node_output_name: str,
            graph_input_names: List[str],
            reachable_nodes: List[onnx.NodeProto],
        ) -> None:
        if node_output_name in graph_input_names:
            return
        for node in self.graph.node:
            if node in reachable_nodes:
                continue
            if node_output_name not in node.output:
                continue
            reachable_nodes.append(node)
            for name in node.input:
                self._dfs_search_reachable_nodes(name, graph_input_names, reachable_nodes)

    def _collect_reachable_nodes(
            self, 
            input_names: List[str],
            output_names: List[str]
        ) -> List[onnx.NodeProto]:
        reachable_nodes:List[onnx.NodeProto] = list()
        for name in output_names:
            self._dfs_search_reachable_nodes(name, input_names, reachable_nodes)
        # needs to be topology sorted.
        nodes = [n for n in self.graph.node if n in reachable_nodes]
        return nodes

    def _collect_reachable_tensors(
            self,
            nodes: List[onnx.NodeProto],
        ) -> Tuple[List[onnx.TensorProto], List[onnx.ValueInfoProto]]: # (initilizers, value_infos)
        all_tensors_name = set()
        for node in nodes:
            for name in node.input:
                all_tensors_name.add(name)
            for name in node.output:
                all_tensors_name.add(name)

        initializers = [self.wmap[t] for t in self.wmap.keys() if t in all_tensors_name]
        value_infos = [self.vimap[t] for t in self.vimap.keys() if t in all_tensors_name]
        assert(len(self.graph.sparse_initializer) == 0)
        assert(len(self.graph.quantization_annotation) == 0)
        return (initializers, value_infos)

    
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
        model_name = os.path.splitext(model_file)[0]
        model_dir = os.path.dirname(model_file)   
        graph_def = helper.make_graph(
            self.onnx_nodes,
            model_name,
            self.onnx_input_vars.values(),
            [self.onnx_output_vars[i] for i in output_names])
        model_def = helper.make_model(graph_def, producer_name='pyis')
        onnx.checker.check_model(model_def)
        os.makedirs(model_dir, exist_ok=True)
        onnx.save_model(model_def, model_file)
