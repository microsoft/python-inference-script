# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

from pyis.python import ops
import os

trie = ops.Trie()
trie.insert("Alpha", 1)
trie.insert("Beta", 2)

1 == trie.lookup("Alpha")
2 == trie.lookup("Beta")
        
trie.lookup("Gamma") # Raises RuntimeError

os.makedirs('tmp',511,True)

trie.save('tmp/trie.bin')

trie2 = ops.Trie()

trie2.load('tmp/trie.bin')
        
trie.items() == trie2.items()
trie.freeze()
trie.items() == trie2.items()

trie.insert("Delta" ,1) # Raises RuntimeError
