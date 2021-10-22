# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import pyis.python.ops

class OrtSession:
    def __init__(self,
                 model_file,
                 input_names,
                 output_names,
                 device = 'CPU',
                 inter_op_thread_num = 1,
                 intra_op_thread_num = 0,
                 dynamic_batching = False,
                 batch_size = 1):
        self.input_name = input_names
        self.output_names = output_names
        self.model_file = model_file
        self.inner_session = pyis.python.ops.OrtSession(
            model_file, input_names, output_names, device,
            inter_op_thread_num, intra_op_thread_num,
            dynamic_batching, batch_size)

    def run(self, *inputs):
        inputs = list(inputs)
        outputs = self.inner_session.run(inputs)
        return tuple(outputs)
 