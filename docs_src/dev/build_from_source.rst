=================
Build from Source
=================

Linux
=================

**Prerequisites**

* CMake >= 3.14
* Python >= 3.6
* PyTorch==1.9.0

Keep the root folder of the git repository as your current working directory. Run the following commands:

.. code:: bash

    python -m pip install --upgrade pip setuptools onnx==1.9.0 onnxruntime

    # build
    cmake -S . -B build
    cmake --build build -j4

    # run all unittests
    export CTEST_OUTPUT_ON_FAILURE=TRUE
    cmake --build build --target test

Windows
================

**Prerequisites**

* Visual Studio 2019
* CMake >= 3.14
* Python >= 3.6
* PyTorch==1.9.0

Keep the root folder of the git repository as your current working directory. Run the following commands:

.. code:: bash

    python -m pip install --upgrade pip setuptools onnx==1.9.0 onnxruntime mymy

    # Generate VS Project Files
    # use system defaults, works well for most cases
    cmake -S . -B build -A x64
    # use VS 2019 explicitly
    # cmake -S . -B build -G "Visual Studio 16 2019" -A x64
    # (legacy) use VS 2017 explicitly
    # cmake -S . -B build -G "Visual Studio 15 2017 Win64" -A x64

    # Either you open the solution file ``pyis.sln`` in the ``build/`` folder with Visual Studio, 
    # or you build with CMake commands:
    cmake --build build --config RelWithDebInfo -- /m:4

    # run all unittests
    # check stdout/stderr at `build/Testing/Temporary/LastTest.log`
    cmake --build build --config RelWithDebInfo --target RUN_TESTS
    
.. note::
    Debug build is the default CMake build setting. Since most third-party libraries only have Release build provided, e.g., libtorch and onnxruntime, RelWithDebInfo will be recommended during development if you want to debug the code step-by-step.

Install Pip Package in Develop Mode
=======================================

To test and debug the wheel package in an agile way, it is recommended to install pyis source tree as the package locally. Any changes made will take effect immediately as you make.

.. code:: bash

    cd py/
    python -m pip install --upgrade pip setuptools
    python -m pip install -e .
    python -m unittest discover tests/

    # uninstall if you don't need it any more
    python -m pip uninstall -e .

In case you see any package import issue, you may want to take a look at files on disk related to installed packages. Here is a shortcut for where you can find them:

.. code:: bash

    # find your site-packages path by
    python -m site
