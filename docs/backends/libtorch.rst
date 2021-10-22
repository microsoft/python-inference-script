===================================
LibTorch Backend |:fire:|
===================================

The LibTorch backend is the right choice when:

* You plan to host the model on servers as remote services. But if you don't mind the binary size overhead made by `LibTorch Mobile <https://pytorch.org/mobile/android/>`_, you could also deploy the model to mobile devices.  
* Your serving platform doesn't provide a Python interpreter.
* You care about throughput and need to avoid `Python GIL <https://wiki.python.org/moin/GlobalInterpreterLock>`_.
* You want to embed the model into your own C/C++/Java/C# applications.

Author Model with Python
===================================
As an example, we will buid a rule-based model. The model tells if an user query is a weather related one by checking if it appears in the allow-list. It supports two languages:English and Chinese.

.. literalinclude:: ../examples/doc_libtorch_backend.py
    :language: python
    :lines: 4-31

Save Model
===================================
The model object with all dependent files are serialized to .pt (torchscript) file.

.. literalinclude:: ../examples/doc_libtorch_backend.py
    :language: python
    :lines: 37-41

Load Model and Validate in Python
===================================
Load the model with Pytorch and run the loaded model.

.. literalinclude:: ../examples/doc_libtorch_backend.py
    :language: python
    :lines: 44-53

Load Model and Run without Python
===================================
The ultimate step is to deploy the generated torchscript to production environment (normally C++). Below is the sample code to load and run the model in C++. Please refer to `this pytorch tutorial <https://pytorch.org/tutorials/advanced/torch_script_custom_ops.html>`_ for more details on running pytorch in C++.
 
.. literalinclude:: ../examples/cpp_pyis_torch/doc_torchscript_load.cpp
    :language: cpp

Along with CMakeLists.txt file to build the sample.

.. literalinclude:: ../examples/cpp_pyis_torch/CMakeLists.txt
    :language: CMake

Build this sample

.. code:: bash

    cmake -S . -B build -A x64
    cmake --build build

Now we can run the successfully built executable:

.. code:: bat

    > .\build\Debug\pyis_torch_test.exe
    Weather intent : 1