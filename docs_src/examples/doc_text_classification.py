# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
from typing import List, Tuple, Dict
from pyis.python import ops
from pyis.python.offline import TextClassification as TextCls

tmp_dir = 'tmp/doc_linear_svm/'
os.makedirs(tmp_dir, exist_ok=True)

# training
xs = [
        ['this', 'is', 'heaven'],
        ['this', 'is', 'hell']
    ]
ys = [1, 2]

unigram_featurizer = TextCls.create_ngram_featurizer(xs, n=1, boundaries=True)
unigram_features = [unigram_featurizer.transform(q) for q in xs]

bigram_featurizer = TextCls.create_ngram_featurizer(xs, n=2, boundaries=True)  
bigram_features = [bigram_featurizer.transform(q) for q in xs]

concat_featurizer = TextCls.create_text_feature_concat(unigram_features, bigram_features)
concat_features = [concat_featurizer.transform(list(x)) \
    for x in zip(unigram_features, bigram_features)]

data_file = os.path.join(tmp_dir, 'svm.data.txt')
model_file = os.path.join(tmp_dir, 'svm.model.bin')
linear_svm = TextCls.create_linear_svm(concat_features, ys, data_file, model_file)

# inference
def text_feature_to_liblinear(features:List[ops.TextFeature]) -> List[Tuple[int, float]]:
    feature_map: Dict[int, float] = {}
    for f in features:
        fid = f.id()
        fvalue = f.value()
        if fid in feature_map:
            feature_map[fid] += fvalue
        else:
            feature_map[fid] = fvalue  
    return [(k, feature_map[k]) for k in sorted(feature_map)]

x = ['is', 'heaven']
unigrams = unigram_featurizer.transform(x)
bigrams = bigram_featurizer.transform(x)
features = concat_featurizer.transform([unigrams, bigrams])
features_svm = text_feature_to_liblinear(features)
values = linear_svm.predict(features_svm)
print(values)
