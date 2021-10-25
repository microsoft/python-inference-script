==================================
Native C++ Library
==================================

We also provide C++ library for you to link to in case:

* You are only interested in one or two specific algorithms implemented in some operators.
* You don't like the concept or methodology that PyIS imposes on you, e.g., defination of a model, deployment with libtorch or onnxruntime. It is a pity. But we understand that and are with you.

Use the Operator in C++
==================================

.. literalinclude:: ../examples/cpp_library/cpp_library_test.cpp
    :language: cpp

Build the Example as a C++ Project with CMake
==============================================

All core algorithms are implemented in the CMake target `pyis_operators`.

.. literalinclude:: ../examples/cpp_library/CMakeLists.txt
    :language: CMake

To optimize the binary size, PyIS allows you to disable C++ exceptions or RTTI.

.. code:: cmake

    # disable C++ exceptions
    set(PYIS_NO_EXCEPTIONS
        ON
        CACHE BOOL "" FORCE)
    # disable C++ RTTI
    set(PYIS_NO_RTTI
        ON
        CACHE BOOL "" FORCE)

Run
==================================

.. code:: bash

    cmake -S . -B build -A x64
    cmake --build build

Now we can run the successfully built executable:

.. code:: bat

    > .\build\Debug\cpp_library_test.exe
    the answer is 42
