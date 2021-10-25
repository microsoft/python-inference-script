===========================
Trie
===========================

A trie implementation combined cedar trie and immutable trie. The trie 
is able to modify initially. When the trie is determined and not required to be
modified, it is allowed to be frozen to achieve a better performance. 
When it is saved, the saved binary file is always frozen trie binary.

APIs
===========================

.. autoclass:: pyis.python.ops.Trie
    :members:
    :undoc-members:

Example
============================

.. literalinclude:: ../examples/doc_trie.py
    :language: python
