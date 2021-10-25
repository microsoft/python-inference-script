# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import numpy as np
from pyis.python import ops
from typing import List

model_path: str = 'example.onnx'
input_names: List[str] = ['query_token_ids']
output_names: List[str] = ['class_label']

inputs: List[np.ndarray] = [np.array([3, 6, 8], dtype=np.int64)]

# create ort session object
ort_session: ops.OrtSession = ops.OrtSession(
    model_path,
    input_names,
    output_names,
    inter_op_thread_num = 1,
    intra_op_thread_num = 0,
    dynamic_batching = False
)

# run ort inference and get outputs
outputs: List[np.ndarray] = ort_session.run(inputs)
