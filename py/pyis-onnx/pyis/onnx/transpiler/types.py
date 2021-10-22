# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from os import stat
from onnx import TensorProto
import typing
from typing import List
import ast

class Types:
    def __init__(self) -> None:
        pass

    @staticmethod
    def annotation_to_py(ast_node):
        if isinstance(ast_node, ast.Name):
            type = ast_node.id
            if type == 'bool':
                return bool
            elif type == 'int':
                return int
            elif type == 'float':
                return float
            elif type == 'str':
                return str
            else:
                raise TypeError(f'unsupported annotation type {type}')

        if isinstance(ast_node, ast.Subscript):
            type = ast_node.value.id
            if type == 'List':
                inner_type = Types.annotation_to_py(ast_node.slice.value)
                return typing.List[inner_type]
            else:
                raise TypeError(f'unsupported annotation type {type}')

    @staticmethod
    def py_to_tensor(t):
        if t is str or t is List[str]:
            return TensorProto.STRING
        elif t is float or t is List[float]:
            return TensorProto.DOUBLE
        elif t is int or t is List[int]:
            return TensorProto.INT64
        elif t is bool or t is List[bool]:
            return TensorProto.BOOL
        else:
            raise RuntimeError(f'unsupported type {t}')

    @staticmethod
    def annotation_to_tensor(t):
        py_type = Types.annotation_to_py(t)
        return Types.py_to_tensor(py_type)
