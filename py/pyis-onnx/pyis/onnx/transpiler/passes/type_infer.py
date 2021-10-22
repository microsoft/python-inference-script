# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import ast
import json
from pyis.onnx.transpiler.function_call import FunctionCall
from pyis.onnx.transpiler.types import Types


class TypeInfer(ast.NodeVisitor):
    def __init__(self, obj, tree: ast.AST):
        self.tree = tree
        self.obj = obj
        self.type_dict = {}
        self.function_calls = {}
    
    def run(self):
        self.visit(self.tree)

    def visit_FunctionDef(self, node: ast.FunctionDef):
        for i, arg in enumerate(node.args.args):
            if i==0 and arg.arg == 'self':
                continue
            if arg.annotation == None:
                raise TypeError(f'missing type annotation. function:{node.name} argument:{arg.arg}')
            self.type_dict[arg.arg] = Types.annotation_to_py(arg.annotation)
        for line in node.body:
            self.visit(line)
    
    # annotated assignment
    def visit_AnnAssign(self, node: ast.AnnAssign):
        if not hasattr(node, 'annotation'):
            raise TypeError(f'missing type annotation. varibale:{node.target.id}')
        var_name = node.target.id
        if var_name in self.type_dict:
            raise RuntimeError(f'varibale {var_name} is already defined')
        self.type_dict[node.target.id] = Types.annotation_to_py(node.annotation)
    
    # deduce varibale types via return type annotations of a function call
    def visit_Assign(self, node: ast.Assign):
        types = []
        if isinstance(node.value, ast.Call):
            # FunctionCall.returns is a python class @property, keep it to 
            # avoid duplicate calculations
            types = list(self.visit_Call(node.value))
        else:
            # variable assignment
            if isinstance(node.value, ast.Tuple):
                for i, elt in enumerate(node.value.elts):
                    types.append(self.visit(elt))
            else:
                types.append(self.visit(node.value))

        targets = node.targets[0]
        if isinstance(targets, ast.Tuple):
            for i, target in enumerate(targets.elts):
                if target.id == "_":
                    continue
                self.type_dict[target.id] = types[i]
        else:
            self.type_dict[targets.id] = types[0]
    
    def visit_Name(self, node: ast.Name):
        return self.type_dict[node.id]
    
    def visit_Str(self, node: ast.Str):
        return str

    def visit_Num(self, node: ast.Num):
        if isinstance(node.n, int):
            return int
        elif isinstance(node.n, float):
            return float
        else:
            raise TypeError(f'unknown type. contant:{node.value}')
 
    def visit_Call(self, node: ast.Call):
        func_ns = []  # e.g., self.mem1.func
        attr_or_name = node.func
        while isinstance(attr_or_name, ast.Attribute):
            func_ns.insert(0, attr_or_name.attr)
            attr_or_name = attr_or_name.value
        func_ns.insert(0, attr_or_name.id)
        call = FunctionCall(self.obj, func_ns)
        self.function_calls[node] = call
        return call.returns
    
    def get_type_dict(self):
        return self.type_dict
    
    def print_type_dict(self):
        res = {}
        for k, v in self.type_dict.items():
            res[k] = str(v)
        print(json.dumps(res, indent=4))
    
    def get_function_calls(self):
        return self.function_calls
