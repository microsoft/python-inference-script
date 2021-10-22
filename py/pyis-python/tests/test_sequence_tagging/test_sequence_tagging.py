# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import unittest, os
from typing import Iterator, List, Tuple, Dict
from pyis.python import ops
from pyis.python import save
from pyis.python.offline import SequenceTagging as SeqTag
from pyis.python.offline import TextClassification as TextCls

tmp_dir = 'tmp/test_sequence_tagging/'
os.makedirs(tmp_dir, exist_ok=True)

class Model:
    def __init__(self, xs:Iterator[List[str]], ys:Iterator[List[int]]):
        super(Model, self).__init__()
        self.unigram_featurizer = None
        self.bigram_featurizer = None
        self.concat_featurizer = None
        self.lccrf = None
        self._train(xs, ys)
    
    def forward(self, x:List[str]) -> List[int]:
        unigrams = self.unigram_featurizer.transform(x)
        #self._print_features(unigrams, 'unigrams: ')
        bigrams = self.bigram_featurizer.transform(x)
        #self._print_features(bigrams, 'bigrams: ')
        features = self.concat_featurizer.transform([unigrams, bigrams])
        features_lccrf = self.text_feature_to_lccrf(features)
        #print(f'features: {features_lccrf}')
        values = self.lccrf.predict(len(x), features_lccrf)
        return values
    
    def text_feature_to_lccrf(self, features:List[ops.TextFeature]) -> List[Tuple[int, int, float]]:
        res: List[Tuple[int, int, float]] = []
        for f in features:
            idx = f.pos()[0]
            fid = f.id()
            fvalue = f.value()
            res.append((idx, fid, fvalue))
        return res

    def _print_features(self, features: List[ops.TextFeature], prefix=""):
        print(prefix, end='')
        for i in features:
            print(f'{i.to_tuple()} ', end='')
        print("")
                   
    def _train(self, xs, ys):
        self.unigram_featurizer = TextCls.create_ngram_featurizer(xs, n=1, boundaries=True)
        self.unigram_featurizer.dump_ngram(os.path.join(tmp_dir, '1gram.txt'))
        unigram_features = [self.unigram_featurizer.transform(q) for q in xs]

        self.bigram_featurizer = TextCls.create_ngram_featurizer(xs, n=2, boundaries=True)
        self.bigram_featurizer.dump_ngram(os.path.join(tmp_dir, '2gram.txt'))     
        bigram_features = [self.bigram_featurizer.transform(q) for q in xs]

        self.concat_featurizer = TextCls.create_text_feature_concat(unigram_features, bigram_features)
        concat_features = [self.concat_featurizer.transform(list(x)) for x in zip(unigram_features, bigram_features)]
        for x in concat_features:
            self._print_features(x)
       
        data_file = os.path.join(tmp_dir, 'lccrf.data.txt')
        model_file = os.path.join(tmp_dir, 'lccrf.model.bin')
        self.lccrf = SeqTag.create_linear_chain_crf(concat_features, ys, data_file, model_file, 'l1sgd')


class TestSequenceTagging(unittest.TestCase):
    def test_sequence_tagging(self):
        xs = [
            ['from', 'beijing', 'to', 'seattle'],
            ['from', 'shanghai', 'to', 'beijing']
        ]
        ys = [
            [0, 1, 0, 1],
            [0, 1, 0, 1]
        ]

        m = Model(xs, ys)

        labels = m.forward(['from', 'seattle', 'to', 'shanghai'])
        print(labels)
        self.assertListEqual(labels, [0, 1, 0, 1])
        
        model_file = os.path.join(tmp_dir, 'model.pkl')
        save(m, model_file)
        

if __name__ == "__main__":
    unittest.main()