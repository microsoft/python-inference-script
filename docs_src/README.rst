=============================
Python Inference Script(PyIS)
=============================

Python Inference Script is a Python package that enables developers to author machine learning workflows in Python and deploy without Python.

Various tools could be available for fast experimentation, for example sklearn, CNTK, Tensorflow, PyTorch and etc. However, when it comes to deployement, problems will emerge:

* Is it optimized, fast or memory efficient?
* Is the runtime or model compact enough for edge devices?
* Is it easy to learn and cross-platform?

To solve those puzzles, the Python Inference Script(PyIS) is introduced.

.. figure:: images/pyis_overview.png
    :scale: 60 %
    :alt: Python Inference Script
    :align: left

Installation
============================

**Build and Install from Source**

`Instruction <https://microsoft.github.io/python-inference-script/dev/build_from_source.html>`_

**Install from Pip** *(Coming Soon)*

.. code:: bash

    python -m pip install pyis-python --upgrade

Verification
============================

**Python Backend**

.. code:: python

    # Python backend
    from pyis.python import ops
    from pyis.python.model_context import save, load

    # create trie op
    trie = ops.CedarTrie()
    trie.insert('what time is it in Seattle?')
    trie.insert('what is the time in US?')

    # run trie match
    query = 'what is the time in US?'
    is_matched = trie.contains(query)

    # serialize
    save(trie, 'tmp/trie.pkl')

    # load and run
    trie = load('tmp/trie.pkl')
    is_matched = trie.contains(query)

**LibTorch Backend**

.. code:: python

    # LibTorch backend
    import torch
    from pyis.torch import ops
    from pyis.torch.model_context import save, load

    # define torch model
    class TrieMatcher(torch.nn.Module):
        def __init__(self):
            super().__init__()
            self.trie = ops.CedarTrie()
            self.trie.insert('what time is it in Seattle?')
            self.trie.insert('what is the time in US?')

        def forward(self, query: str) -> bool:
            return self.trie.contains(query)

    # create torch model
    model = torch.jit.script(TrieMatcher())

    # run trie match
    query = 'what is the time in US?'
    is_matched = model.forward(query)

    # serialize
    save(model, 'tmp/trie.pt')

    # load and run
    model = load('tmp/trie.pt')
    is_matched = model.forward(query)

**ONNXRuntime Backend**

*Coming Soon...*


Build the Docs
=============================

Run the following commands and open ``docs/_build/html/index.html`` in browser.

.. code:: bash

    pip install sphinx myst-parser sphinx-rtd-theme sphinxemoji
    cd docs/

    make html         # for linux
    .\make.bat html   # for windows
