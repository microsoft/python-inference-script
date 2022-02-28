# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import transformers
import unittest
from pyis.python import ops
import os

class TestBertTokenizer(unittest.TestCase):
    
    def _run_basic_test(self, query):
        self.assertEqual(self.tokenizer.tokenize(query), self.standard.tokenize(query))
        self.assertEqual(self.tokenizer.encode(query), self.standard.encode(query))
        # test encode_plus
        tokenizer_result = self.tokenizer.encode_plus(query)
        input_ids = [x[0] for x in tokenizer_result]
        attention_mask = [x[2] for x in tokenizer_result]
        self.assertDictEqual(dict(self.standard.encode_plus(query)), {'input_ids': input_ids, 'attention_mask': attention_mask})

    def _run_encode_test_2(self, query1, query2):
        self.assertEqual(self.tokenizer.encode2(query1, query2), self.standard.encode(query1, query2))
        tokenizer_result = self.tokenizer.encode_plus2(query1, query2)
        input_ids = [x[0] for x in tokenizer_result]
        attention_mask = [x[2] for x in tokenizer_result]
        self.assertDictEqual(dict(self.standard.encode_plus(query1, query2)),
                             {'input_ids': input_ids, 'attention_mask': attention_mask})

    def test_tokenize(self):
        self._run_basic_test('hello world')
        self._run_basic_test('a berttokenizer')
        self._run_basic_test('The quick brown fox jumps over the lazy dog.')
        self._run_basic_test('It\'s high noon, isn\'t it?')
        self._run_basic_test('D.Va爱你呦😙❤')
        self._run_encode_test_2('hello', 'world')
        self._run_encode_test_2('他吃了两碗粉','却只给了一碗的   钱')
        self._run_encode_test_2('😙','❤')

    def setUp(self):
        os.makedirs('tmp',511,True)
        self.standard = transformers.GPT2Tokenizer.from_pretrained('gpt2')
        self.standard.save_vocabulary('tmp')
        self.tokenizer = ops.GPT2Tokenizer('tmp/vocab.json', 'tmp/merges.txt')

if __name__ == '__main__':
    unittest.main()