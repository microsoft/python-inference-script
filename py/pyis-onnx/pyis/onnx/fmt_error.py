# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
import inspect


def fmt_error(obj, lineo: int, msg: str):
    if obj is str:
        return f'File "{obj}", line {lineo}: {msg}'
    
    t = type(obj)
    file_path = inspect.getfile(t)
    return f'File "{file_path}", line {lineo}: {msg}'