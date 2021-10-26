========================================
House Keeping
========================================

Using clang-format on C++ Code
==============================

``clang-format`` is a tool to automatically format C/C++ code, so that developers don't need to worry about style issues during code reviews.

Linux
----------------------------------------

.. code:: bash

    # make sure to install clang format >= 10
    sudo apt-get install -y clang-format-10
    sudo ln -sf /usr/bin/clang-format-10 /usr/bin/clang-format

    # check syntax issues only
    ./tools/run-clang-format.sh

    # format all source files inplace
    ./tools/run-clang-format.sh -i

Windows
----------------------------------------

* `Visual Studio Support <https://devblogs.microsoft.com/cppblog/clangformat-support-in-visual-studio-2017-15-7-preview-1/>`__ 
* `VSCode Integration <https://marketplace.visualstudio.com/items?itemName=xaver.clang-format>`__ 

Using clang-tidy on C++ Code
========================================

``clang-tidy`` is a tool to automatically check C/C++ code for style violations, programming errors, and best practices.

Linux
----------------------------------------

.. code:: bash

    # make sure to install clang-tidy >=10.0
    sudo apt-get install -y clang-tidy-10
    sudo ln -sf /usr/bin/clang-tidy-10 /usr/bin/clang-tidy

    # run code analysis
    ./tools/run-clang-tidy.sh

Windows
----------------------------------------

* `Visual Studio Integration <https://docs.microsoft.com/en-us/cpp/code-quality/clang-tidy?view=msvc-160>`__

Using cmake-format on CMake Files
========================================

The cmakelang project provides Quality Assurance (QA) and other language tools for cmake.

.. code:: bash

    python3 -m pip install cmakelang

    # check issues only
    tools/run-cmake-format.py --check

    # fix issues inplace
    tools/run-cmake-format.py
