# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import os
from pyis.python import ops

trie = ops.CedarTrie()
trie.insert("Alpha", 1)
trie.insert("Beta", 2)
trie.insert("Delta", 3)
trie.insert("AlphaBeta", 4)

trie.predict("Alpha") # [('Alpha', 1), ('AlphaBeta', 4)]
trie.predict("Gamma") # []
trie.prefix("AlphaBetaGamma") # [('Alpha', 1), ('AlphaBeta', 4)]
trie.numkeys() # 4
trie.lookup("Alpha") # 1

# RuntimeError raised if key not found
try:
    trie.lookup("Gamma")
except RuntimeError as e:
    print("not found")

trie.insert("Alpha", 5)
trie.lookup("Alpha") # 5

trie.erase("Beta")

# Runtime Error raised
try:
    trie.lookup("Beta")
except RuntimeError as e:
    print("not found")

os.makedirs('tmp', exist_ok=True)
trie.save("tmp/trie.bin") # Store to a binary file

trie2 = ops.CedarTrie()
# Load from a binary file
trie2.load("tmp/trie.bin")

# [('Alpha, 5'), ('Delta, 3'), ('AlphaBeta', 4)]
trie2.items() # Dump all key-value pairs from the trie