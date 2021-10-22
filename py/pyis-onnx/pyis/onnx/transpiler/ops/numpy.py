# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import numpy as np
from .op_registry import OpRegistry, Signature

OpRegistry.register(np.array, None, np.ndarray)
