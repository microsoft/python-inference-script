# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import sys
from os.path import dirname
sys.path.append(dirname(__file__))

from types import FunctionType
import unittest
import inspect
from pyis.onnx.transpiler.symbol_resolver import resolve_symbol, resolve_class_function

class TestSymbolResolver(unittest.TestCase):

    def test_global_function(self):       
        # module = importlib.import_module("pkg1.pkg2.cls2")
        from pkg1.pkg2.cls2 import Cls2
        module = inspect.getmodule(Cls2)
        symbol = resolve_symbol(module, "A.cls1.fun_in_pkg1")

        self.assertIsInstance(symbol, FunctionType)
        self.assertEqual(symbol.__name__, 'fun_in_pkg1')
    
    def test_member_function(self):
        from pkg1.pkg2.cls2 import Cls2
        obj = Cls2()
        symbol = resolve_class_function(obj, 'cls1', "fun_in_cls1")

        self.assertIsInstance(symbol, FunctionType)
        self.assertEqual(symbol.__name__, 'fun_in_cls1')


if __name__ == '__main__':
    unittest.main()