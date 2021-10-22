# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import io
from typing import Dict, Iterator, List
from pyis.python import ops

class SequenceTagging:
    def __init__(self):
        pass

    @staticmethod
    def create_ngram_featurizer(
            xs: Iterator[List[str]], 
            n:int=1, 
            boundaries:bool=True) -> ops.NGramFeaturizer:
        featurizer = ops.NGramFeaturizer(n, boundaries)
        for x in xs:
            featurizer.fit(x)
        return featurizer

    @staticmethod
    # variadic_xs: Iterator[List[TextFeature]], Iterator[List[TextFeature]], ...
    def create_text_feature_concat(*variadic_xs, start_id=1) -> ops.TextFeatureConcat:
        tf_concat = ops.TextFeatureConcat(start_id)
        for x in zip(*variadic_xs):
            tf_concat.fit(list(x))
        return tf_concat
    
    @staticmethod
    def create_linear_chain_crf(
            xs:Iterator[List[ops.TextFeature]],
            ys:Iterator[int],
            data_file:str, 
            model_file:str,
            alg: str = 'l1sgd',
            max_iter = 150 ) -> ops.LinearChainCRF:
        # generate liblinear training data from text_features and ys.
        SequenceTagging.text_features_to_lccrf(xs, ys, data_file)
        ops.LinearChainCRF.train(data_file, model_file, alg, max_iter)
        lccrf = ops.LinearChainCRF(model_file)
        return lccrf

    @staticmethod
    def _transform_single_input(featurizer, xs):
        for x in xs:
            yield featurizer.transform(x)

    @staticmethod
    def _transform_multiple_inputs(featurizer, *variadic_xs):
        for x in zip(*variadic_xs):
            yield featurizer.transform(list(x))

    @staticmethod
    def text_features_to_lccrf(
            xs: Iterator[List[ops.TextFeature]],
            ys: Iterator[List[int]], 
            data_file:str):

        with io.open(data_file, 'w', newline='\n') as f:
            for x, y in zip(xs, ys):
                # process features:
                # 1. group features by its position(the `from` token idx)
                # 2. accumulate feature values of same feature id and same position
                # key: token idx, value: [feature id, feature value]
                features: List[Dict[int, float]] = []
                token_cnt = len(y)
                for i in range(token_cnt):
                    features.append({})
                for feature in x:
                    idx = feature.pos()[0]
                    if idx < 0 or idx >= token_cnt:
                        raise RuntimeError(f'invalid token idx {idx}, total token count is {token_cnt}')
                    
                    fid = feature.id()
                    if fid < 0:
                        raise RuntimeError(f'negative feature id is illegal')
                    if fid not in features:
                        features[idx][fid] = 0.0
                    features[idx][fid] += feature.value()

                for idx, fs in enumerate(features):
                    print(f'{y[idx]} 1.0 n/a', file=f, end='')
                    for fid in sorted(fs):
                        print(f' |1 {fid}:{fs[fid]}', file=f, end='')
                    print('', file=f)
                
                print('', file=f)
