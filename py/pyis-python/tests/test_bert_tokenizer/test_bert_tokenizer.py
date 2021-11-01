# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import transformers
import unittest
from pyis.python import ops
import os

class TestBertTokenizer(unittest.TestCase):
    
    def _run_basic_test(self, query):
        os.makedirs('tmp',511,True)
        standard = transformers.BertTokenizer.from_pretrained('bert-base-uncased')
        standard.save_vocabulary('tmp/vocab.txt')
        tokenizer = ops.BertTokenizer('tmp/vocab.txt')
        self.assertEqual(tokenizer.tokenize(query), standard.tokenize(query))
        self.assertEqual(tokenizer.encode(query), standard.encode(query))
        code = tokenizer.encode(query)
        self.assertEqual(tokenizer.decode(code), standard.decode(code))
        self.assertEqual(tokenizer.decode(code, skip_special_tokens = True, clean_up_tokenization_spaces = False), standard.decode(code, skip_special_tokens = True, clean_up_tokenization_spaces = False))

    def test_tokenize(self):
        self._run_basic_test('hello world')
        self._run_basic_test('a berttokenizer')
        self._run_basic_test('The quick brown fox jumps over the lazy dog.')
        self._run_basic_test('It\'s high noon, isn\'t it?')

if __name__ == '__main__':
    unittest.main()