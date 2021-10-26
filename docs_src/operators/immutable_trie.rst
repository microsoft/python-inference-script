===========================
ImmutableTrie
===========================

Immutable Trie has a much smaller memory footprint, compared to cedar trie. See the benchmark section for details.

APIs
===========================

.. autoclass:: pyis.python.ops.ImmutableTrie
    :members:
    :undoc-members:

Example
============================

.. literalinclude:: ../examples/doc_immutable_trie.py
    :language: python

Benchmark
============================

.. list-table:: Performance Comparsion of Immutable Trie and Cedar Trie
    :header-rows: 1

    * - Test Item
      - Immutable Trie
      - Cedar Trie
    * - Construction Time
      - 49.319 sec
      - 15.245 sec
    * - Memory used during construction
      - 5.1 GB
      - 199 MB
    * - Runtime memory footprint
      - 85 MB
      - 170 MB
    * - Query all keys
      - 0.192 sec
      - 0.979 sec
    * - Dump
      - 1.05 sec
      - 1.428 sec

The test dataset contains 5,000,000 randomly generated strings, length from 5 to 20 characters including upper/lower characters and digits. The following is the script used to generate the dataset.

.. literalinclude:: ../examples/doc_trie_generate_testset.py
    :language: python