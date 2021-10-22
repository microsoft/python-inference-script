# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
import unittest
from pyis.python import ops
from pyis.python import save, load

class TestPickle(unittest.TestCase):
    word_dict_file = os.path.join(os.path.dirname(__file__), 'word_dict.data.txt')

    def test_save(self):
        obj = ops.WordDict(TestPickle.word_dict_file)
        res = obj.translate(["HELLO", "WORLD"])
        self.assertEqual(res, ['hello', 'world'])

        # save model data
        save(obj, 'tmp/test_pickle/model.pkl')

        # restore model
        restored_obj = load('tmp/test_pickle/model.pkl')

        res = restored_obj.translate(["HELLO", "WORLD"])
        self.assertEqual(res, ['hello', 'world'])

    def test_save_with_prefix(self):
        obj = ops.WordDict(TestPickle.word_dict_file)

        # save model data
        save(obj, 'tmp/test_pickle/model.pkl', prefix="PREFIX.")

        # restore model
        restored_obj = load('tmp/test_pickle/model.pkl')

        res = restored_obj.translate(["HELLO", "WORLD"])
        self.assertEqual(res, ['hello', 'world'])

if __name__ == "__main__":
    unittest.main()