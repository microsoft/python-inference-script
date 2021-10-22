# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
import pickle
import traceback
from .lib.pyis_python import ModelContext

class ModelContextMgr(object):

    def __init__(self, model_path:str, data_archive:str):
        self.model_path = model_path
        self.data_archive = data_archive
        self.ctx = None
    
    def __enter__(self):
        self.ctx = ModelContext(self.model_path, self.data_archive)
        ModelContext.activate(self.ctx)
        return self.ctx
  
    def __exit__(self, exp_type, exp_value, tb):
        ModelContext.deactivate(self.ctx)
        if exp_type is not None:
            traceback.print_exception(exp_type, exp_value, tb)
            return False

        return True

def save(obj, model_path:str, prefix:str="", data_archive:str="") -> str:  
    if os.path.isdir(model_path):
        raise FileNotFoundError(f'model path should be a file, but it is an existing directory. path:{model_path}')
    
    model_dir = os.path.dirname(model_path)
    os.makedirs(model_dir, exist_ok=True)

    model_file = os.path.basename(model_path)
    if not model_file.startswith(prefix):
        model_file = f'{prefix}{model_file}'
    pickle_file = os.path.join(model_dir, f"{model_file}")

    with ModelContextMgr(model_path, data_archive) as ctx, open(pickle_file, "wb") as f:
        ctx.set_file_prefix(prefix)
        pickle.dump(obj, f)

def load(model_path:str, data_archive:str="") -> str:
    if not os.path.isfile(model_path):
        raise FileNotFoundError(f'model path does not exist. path:{model_path}')
        
    pickle_file = model_path   
    with ModelContextMgr(model_path, data_archive) as ctx, open(pickle_file, "rb") as f:
        restored_obj = pickle.load(f)
    return restored_obj
