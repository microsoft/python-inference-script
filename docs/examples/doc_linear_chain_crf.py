# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
from pyis.python import ops
from pyis.python.offline import SequenceTagging as SeqTag

tmp_dir = 'tmp/doc_linear_chain_crf/'
os.makedirs(tmp_dir, exist_ok=True)

# training
xs = [
        [ops.TextFeature(0, 1.0, 0, 0), ops.TextFeature(1, 1.0, 1, 1)], # hello tom
        [ops.TextFeature(0, 1.0, 0, 0), ops.TextFeature(2, 1.0, 1, 1)], # hello jerry
    ]
ys = [
        [0, 1], # O NAME
        [0, 1], # O NAME
    ]

data_file = os.path.join(tmp_dir, 'lccrf.data.txt')
SeqTag.text_features_to_lccrf(xs, ys, data_file)

model_file = os.path.join(tmp_dir, 'lccrf.model.bin')
ops.LinearChainCRF.train(data_file, model_file, 'l1sgd')
lccrf = ops.LinearChainCRF(model_file)

# inference
values = lccrf.predict(2, [(0, 0, 1.0), (1, 1, 1.0)]) # hello tom
print(values)
