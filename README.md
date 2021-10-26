# Python Inference Script(PyIS)

Python Inference Script is a Python package that enables developers to author machine learning workflows in Python and deploy without Python.

Various tools could be available for fast experimentation, for example sklearn, CNTK, Tensorflow, PyTorch and etc. However, when it comes to deployement, problems will emerge:

* Is it optimized, fast or memory efficient?
* Is the runtime or model compact enough for edge devices?
* Is it easy to learn or cross-platform?

To solve those puzzles, the Python Inference Script(PyIS) is introduced.

![Image](docs_src/images/pyis_overview.png)

## Installation

### Build and install from source

[Instruction](https://microsoft.github.io/python-inference-script/dev/build_from_source.html)

### *from pip source (Coming Soon)*

```bash
python -m pip install pyis-python --upgrade 
```



## Verification
### Python Backend
```python
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
```

### LibTorch Backend
```python
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
```

### ONNXRuntime Backend
*COMING SOON...*


## Build the Docs

Run the following commands and open `docs/_build/html/index.html` in browser.
```
    pip install sphinx myst-parser sphinx-rtd-theme sphinxemoji
    cd docs/

    make html         # for linux
    .\make.bat html   # for windows
```

## Contributing

Please refer to [CONTRIBUTING.md](CONTRIBUTING.md) for the agreement and the instructions if you want to participate in this project.

## Data Collection

The software may collect information about you and your use of the software and send it to Microsoft. Microsoft may use this information to provide services and improve our products and services. You may turn off the telemetry as described in the repository. There are also some features in the software that may enable you and Microsoft to collect data from users of your applications. If you use these features, you must comply with applicable law, including providing appropriate notices to users of your applications together with a copy of Microsoft’s privacy statement. Our privacy statement is located at https://go.microsoft.com/fwlink/?LinkID=824704. You can learn more about data collection and use in the help documentation and our privacy statement. Your use of the software operates as your consent to these practices.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft trademarks or logos is subject to and must follow Microsoft’s Trademark & Brand Guidelines. Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those third-party’s policies.

## Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## License

Copyright (c) Microsoft Corporation. All rights reserved.

Licensed under the [MIT](LICENSE.txt) license.