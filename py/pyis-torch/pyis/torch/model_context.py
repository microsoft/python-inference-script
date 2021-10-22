# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
import torch
import traceback
from pyis.torch import ops

class ModelContextMgr(object):

    def __init__(self, model_path:str, data_archive:str):
        self.model_path = model_path
        self.data_archive = data_archive
        self.ctx = None
    
    def __enter__(self):
        self.ctx = ops.ModelContext(self.model_path, self.data_archive)
        ops.ModelContext.activate(self.ctx)
        return self.ctx
  
    def __exit__(self, exp_type, exp_value, tb):
        ops.ModelContext.deactivate(self.ctx)
        if exp_type is not None:
            traceback.print_exception(exp_type, exp_value, tb)
            return False

        return True

def save(obj: torch.jit.ScriptModule, model_path:str, prefix:str="", data_archive:str="") -> str:  
    if os.path.isdir(model_path):
        raise FileNotFoundError(f'model path should be a file, but it is an existing directory. path:{model_path}')
    
    model_dir = os.path.dirname(model_path)
    os.makedirs(model_dir, exist_ok=True)

    model_file = os.path.basename(model_path)
    if not model_file.startswith(prefix):
        model_file = f'{prefix}{model_file}'
    model_file_path = os.path.join(model_dir, f"{model_file}")

    if isinstance(obj, torch.nn.Module):
        obj = torch.jit.script(obj)
    with ModelContextMgr(model_path, data_archive) as ctx:
        ctx.set_file_prefix(prefix)
        torch.jit.save(obj, model_file_path)

def load(model_path:str, data_archive:str="") -> torch.jit.ScriptModule:
    if not os.path.isfile(model_path):
        raise FileNotFoundError(f'model path does not exist. path:{model_path}')
    
    with ModelContextMgr(model_path, data_archive) as ctx:
        restored_obj = torch.jit.load(model_path)
    return restored_obj
