__version__ = '0.1.dev2'

import os
import platform
from packaging import version
import platform
import types

SUPPORTE_TORCH_VERSIONS = ['1.8.1', '1.9.0']
TORCH_VERSION = None

try:
    import torch   
except ImportError:   
    raise ImportError(f"please install torch {SUPPORTE_TORCH_VERSIONS[-1]}")

TORCH_VERSION = version.Version(torch.__version__).base_version
print(f"TORCH_VERSION={TORCH_VERSION}")

if TORCH_VERSION not in SUPPORTE_TORCH_VERSIONS:
    raise ImportError(f"torch {TORCH_VERSION} is not supported, please install torch {SUPPORTE_TORCH_VERSIONS[-1]}")

TORCH_VERSION_STD = TORCH_VERSION.replace(".", "")

if platform.system() == 'Windows':
    torch.classes.load_library(os.path.join(os.path.dirname(__file__), 'lib', 'onnxruntime.dll'))
    torch.classes.load_library(os.path.join(os.path.dirname(__file__), 'lib', f'pyis_torch{TORCH_VERSION_STD}.dll'))
elif platform.system() == "Linux":
    torch.classes.load_library(os.path.join(os.path.dirname(__file__), 'lib', f'libpyis_torch{TORCH_VERSION_STD}.so'))
else:
    raise NotImplementedError(f'platform {platform.system()} is not supported yet')

class Ops(types.ModuleType):
    def __init__(self):
        super(Ops, self).__init__('pyis.torch.ops')

    def __getattr__(self, name):
        module = getattr(torch.classes.pyis, name)
        setattr(self, name, module)
        return module

ops = Ops()
from .model_context import save, load