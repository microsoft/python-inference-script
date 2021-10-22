# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from pyis.python import ops

featurizer = ops.NGramFeaturizer(2, True)
featurizer.fit(['the', 'answer', 'is', '42'])
features = featurizer.transform(['the', 'answer', 'is', '42'])

for f in features:
    print(f.to_tuple())

'''Output:     
# (0, 1.0, 0, 1)
# (1, 1.0, 0, 1)
# (2, 1.0, 1, 2)
# (3, 1.0, 2, 3)
# (4, 1.0, 2, 3)
'''