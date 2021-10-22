# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from inspect import signature
from pyis.onnx.transpiler.ops.op_registry import OpRegistry
from pyis.onnx.transpiler.symbol_resolver import resolve_class_function, resolve_global_function


class FunctionCall:
    def __init__(self, containing_obj, function_ns) -> None:
        self.containing_obj = containing_obj
        self.function_ns = function_ns
        self.return_types = []

    def is_global_function(self):
        if self.function_ns[0] == 'self':
            return False
        else:
            return True
    
    @property
    def qualname(self):
        return '.'.join(self.function_ns)

    @property
    def function(self):
        if not self.is_global_function():
            attribute, function_name = self.function_ns[1], self.function_ns[2]
            return resolve_class_function(self.containing_obj, attribute, function_name)
        else:
            return resolve_global_function(self.containing_obj, self.qualname)
            #raise RuntimeError(f'{".".join(self.function_ns)} is a global function, which is not supported yet')
    
    @property
    def class_instance(self):
        if not self.is_global_function():
            attribute = self.function_ns[1]
            return getattr(self.containing_obj, attribute)
        else:
            raise RuntimeError(f'function {self.qualname} is not class member function')

    @property
    def class_type(self):
        if not self.is_global_function():
            attribute = self.function_ns[1]
            t = type(getattr(self.containing_obj, attribute))
            return t
        else:
            raise RuntimeError(f'function {self.qualname} is not class member function')

    @property
    def returns(self):
        f = self.function
        try:
            sig = signature(f)
            if sig.return_annotation.__origin__ is tuple:
                if getattr(sig.return_annotation, '__args__'):
                    # Python 3.6+
                    use_ellipsis = sig.return_annotation.__args__[-1] is Ellipsis
                    return sig.return_annotation.__args__[:-1 if use_ellipsis else None]
                else:
                    # Python 3.5
                    return tuple(sig.return_annotation.__tuple_params__)
            else:
                return (sig.return_annotation,)
        except:
            # deduce return value type from OpRegistry
            sign = OpRegistry.get_signature(f)
            if sign.ret is tuple:
                return sign.ret
            else:
                return (sign.ret,)
