# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
from pyis.python import ops

data = [('Alpha', 1), ('Beta', 2), ('Delta', 3), ('AlphaBeta', 4)]

os.makedirs('tmp', exist_ok=True)
ops.ImmutableTrie.compile(data, 'tmp/trie.bin')

trie = ops.ImmutableTrie()
trie.load('tmp/trie.bin')

set(data) == set(trie.items())

1 == trie.lookup('Alpha')
2 == trie.lookup('Beta')
4 == trie.lookup('AlphaBeta')
trie.lookup('NonExists') # raise RuntimeError  
