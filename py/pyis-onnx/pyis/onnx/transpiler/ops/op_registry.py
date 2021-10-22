# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from typing import Callable
import numpy as np

class Signature:
    def __init__(self, args, ret):
        self.args = args
        self.ret = ret

class OpRegistry:
    signatures = {}

    def __init__(self):
        pass
    
    @staticmethod
    def register(f: Callable, args, ret):
        OpRegistry.signatures[f] = Signature(args, ret)
    
    @staticmethod
    def get_signature(f: Callable) -> Signature:
        return OpRegistry.signatures[f]
