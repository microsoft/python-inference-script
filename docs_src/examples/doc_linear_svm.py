# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import io, os
from typing import List, Tuple, Dict
from pyis.python import ops
from pyis.python.offline import TextClassification as TextCls

tmp_dir = 'tmp/doc_linear_svm/'
os.makedirs(tmp_dir, exist_ok=True)

# training
# id `zero` is illegal for liblinear
xs = [
        [(1, 1.0), (2, 1.0)],
        [(2, 1.0), (3, 1.0)],
    ]
ys = [1, 2]

data_file = os.path.join(tmp_dir, 'svm.data.txt')     
with io.open(data_file, 'w', newline='\n') as f:
    for x, y in zip(xs, ys):
        features: Dict[int, float] = {}
        for fid, fvalue in x:
            if fid not in features:
                features[fid] = 0.0
            features[fid] += fvalue
        print(y, file=f, end='')
        for k in sorted(features):
            print(f" {k}:{features[k]}", file=f, end='')
        print("", file=f) 

model_file = os.path.join(tmp_dir, 'svm.model.bin')
ops.LinearSVM.train(data_file, model_file, 5, 0.1, 1.0, 0.5, 1.0)
linear_svm = ops.LinearSVM(model_file)

# inference
values = linear_svm.predict([(1, 1.0)])
print(values)

values = linear_svm.predict([(2, 1.0)])
print(values)

values = linear_svm.predict([(3, 1.0)])
print(values)
