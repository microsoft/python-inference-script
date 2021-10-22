# Python Inference Script(PyIS)

Python Inference Script is a Python package that enables developers to author machine learning workflows in Python and deploy without Python.

Various tools could be available for fast experimentation, for example sklearn, CNTK, Tensorflow, PyTorch and etc. However, when it comes to deployement, problems will emerge:

* Is it optimized, fast or memory efficient?
* Is the runtime or model compact enough for edge devices?
* Is it easy to learn or cross-platform?

To solve those puzzles, the Python Inference Script(PyIS) is introduced.

## Installation

```bash
python -m pip install pyis-python --upgrade 
```

## Verification

```python
    import torch
    import pyis
    from typing import List

    class Tokenizer(torch.jit.ScriptModule):
        def forward(self, q: str) -> List[int]:
            tokens = pyis.text.BertTokenizer(q)
            return tokens

    m = Tokenizer()

    # save for onnxruntime
    pyis.onnx.save(m, 'model.onnx')

    # save for libtorch
    pyis.torch.save(m, 'model.pt')
```

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