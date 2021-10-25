# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from pyis.python import ops

featurizer = ops.TextFeatureConcat(1)
feature_group0 = [ops.TextFeature(0, 1.0, 0, 0), ops.TextFeature(1, 1.0, 0, 0)]
feature_group1 = [ops.TextFeature(0, 1.0, 0, 0), ops.TextFeature(1, 1.0, 0, 0)]
featurizer.fit([feature_group0, feature_group1])
features = featurizer.transform([feature_group0, feature_group1])
for f in features:
    print(f.to_tuple())

'''Output:
(1, 1.0, 0, 0)
(2, 1.0, 0, 0)
(3, 1.0, 0, 0)
(4, 1.0, 0, 0)
'''