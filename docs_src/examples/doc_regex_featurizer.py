# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from pyis.python import ops

featurizer = ops.RegexFeaturizer(['\d+', r'\d+:\d+'])
features = featurizer.transform(['the', 'answer', 'is', '42'])
# features = [(0, 1.0, 3, 3)]
features = featurizer.transform(['set', 'an', 'alarm', 'at', '7:00', 'tomorrow'])
# features = [(1, 1.0, 4, 4)]        
