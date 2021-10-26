# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
from pyis.python import ops
from typing import List

class Model:
    def __init__(self, dict_data_file: str):
        super().__init__()
        self.dictionary = ops.WordDict(dict_data_file)

    def run(self, tokens: List[str]) -> List[str]:
        res = self.dictionary.translate(tokens)
        return res

dict_data_file = os.path.join(os.path.dirname(__file__), 'word_dict.data.txt')

m = Model(dict_data_file)
res = m.run(["life", "in", "suzhou"])
print(res)
