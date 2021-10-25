# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from pyis.python import ops
from typing import List

tokenizer: ops.BertTokenizer = ops.BertTokenizer('vocab.txt')

query: str = 'what is the time in US?'

token_ids: List[int] = tokenizer.tokenize(query)
