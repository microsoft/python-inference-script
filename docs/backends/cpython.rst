==================================
CPython Backend |:snake:|
==================================

The CPython backend is the right choice when:

* You plan to host the model on servers as remote services.
* Your serving platform provides a Python interpreter.

Author Model with Python
==================================

As an example, we will buid a rule-based model. The model tells if an user query is a weather related one by checking if it appears in the allow-list. It supports two languages:English and Chinese.

.. literalinclude:: ../examples/doc_cpython_backend.py
    :language: python
    :lines: 4-29

Save Model
==================================

The model object could be serialized in `standard pickle protocol <https://docs.python.org/3/library/pickle.html>`_.

.. literalinclude:: ../examples/doc_cpython_backend.py
    :language: python
    :lines: 32-38

Load Model and Run
==================================

The pickled model could be further de-serialized again.

.. literalinclude:: ../examples/doc_cpython_backend.py
    :language: python
    :lines: 41-50
