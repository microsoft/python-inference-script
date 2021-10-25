===========================
LexiconFeaturizer
===========================

In PyIS, an operator is either a stateless global function, or a member function of a class that holds external states(data members). In either case, an operator is thread-safe and reentrant. 

In most cases, we prefer a class member function. The WordDict is such a sample class. It demostrates how an operator is implemented in the form of a class, and its behavior.

APIs
===========================

.. autoclass:: pyis.python.ops.NGramFeaturizer
    :members:
    :undoc-members:

Example
============================

.. literalinclude:: ../examples/doc_word_dict.py
    :language: python
