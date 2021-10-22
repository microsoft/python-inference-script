# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import inspect
import importlib


def resolve_symbol(module, symbol: str):

    if len(symbol) == 0:
        return module
    
    ns_fields = symbol.split('.')
    ns = ns_fields[0]

    # module has an attribute named ns
    if hasattr(module, ns):
        sub_module = getattr(module, ns)
        return resolve_symbol(sub_module, ".".join(ns_fields[1:]))
    
    # ns doesn't exist in module, try import it as a module
    if inspect.ismodule(module):
        sub_module_spec = importlib.util.find_spec(f'{module.__name__}.{ns}')
        if sub_module_spec is not None:
            sub_module = importlib.util.module_from_spec(sub_module_spec)
            sub_module_spec.loader.exec_module(sub_module)
            return resolve_symbol(sub_module, ".".join(ns_fields[1:]))

    raise ImportError(f'failed to resolve symbol {symbol} in module {module.__name__}')

def resolve_class_function(obj, attibute: str, function: str):
    class_type = type(getattr(obj, attibute))
    return resolve_symbol(class_type, function)

def resolve_global_function(obj, function: str):
    module = inspect.getmodule(obj)
    return resolve_symbol(module, function)