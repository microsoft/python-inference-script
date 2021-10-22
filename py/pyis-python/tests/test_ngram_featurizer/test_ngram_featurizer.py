# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
import unittest
from pyis.python import ops

class TestNGramFeaturizer(unittest.TestCase):
    def test_add(self):
        featurizer = ops.NGramFeaturizer(2, False)
        featurizer.fit(['hello', 'world'])
        features = featurizer.transform(['hello', 'world'])
        for i in features:
            print(i.to_tuple())

    def test_load(self):
        ngram_file = os.path.join(os.path.dirname(__file__), 'data', 'ngram.vocab.txt')
        featurizer = ops.NGramFeaturizer(1, False)
        featurizer.load_ngram(ngram_file)
        features = featurizer.transform(['hello', 'world'])
        for i in features:
            print(i.to_tuple())

if __name__ == '__main__':
    unittest.main()