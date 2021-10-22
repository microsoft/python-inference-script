===================================
Build LibTorch for JIT
===================================

Debug Torch Script(Windows)
===================================

**Prerequisites**

* CMake >= 3.14
* Python >= 3.6

**Step 1:** Clone the code

.. code:: bat

    git clone --recursive https://github.com/pytorch/pytorch.git
    cd pytorch

**Step 2:** Generate Visual Studio Solution

Open a command prompt, and run the following commands to open cmake-gui.

.. code:: bat

    REM cwd=pytorch root folder
    set BUILD_TEST=OFF
    set USE_DISTRIBUTED=OFF
    set USE_CUDA=OFF
    set USE_OPENMP=OFF
    set USE_MKLDNN=OFF
    set USE_FBGEMM=OFF   
    set USE_QNNPACK=OFF
    set BUILD_PYTHON=OFF
    set BUILD_CAFFE2_OPS=OFF

    REM set USE_NNPACK=OFF
    REM set USE_XNNPACK=OFF

    set USE_NINJA=OFF
    set CMAKE_GENERATOR="Visual Studio 16 2019"

    python setup.py install --cmake-only
    cmake-gui

Uncheck ``BUILD_PYTHON``, and then run ``Configure->Generate->Open Project`` to open the Visual Studio solution.

**Step 3:** Build libtorch with ``Debug`` configuration

**Step 4:** Find one unittest to debug

For example, use :file:`test/cpp/api/jit.cpp` as an entrance to start with.

LibTorch Minimal Build
=====================================

Windows
-------------------------------------

.. code-block:: bat

    cmake -DBUILD_SHARED_LIBS:BOOL=ON `
        -DBUILD_TEST=OFF `
        -DUSE_DISTRIBUTED=OFF `
        -DUSE_CUDA=OFF `
        -DUSE_OPENMP=OFF `
        -DUSE_MKLDNN=OFF `
        -DUSE_FBGEMM=OFF `
        -DUSE_QNNPACK=OFF `
        -DBUILD_PYTHON=OFF `
        -DUSE_XNNPACK=OFF `
        -DUSE_NNPACK=OFF `
        -DBUILD_CAFFE2=OFF `
        -DPYTHON_EXECUTABLE:PATH=C:\Path\to\Python\python.exe `
        -DCMAKE_INSTALL_PREFIX:PATH=libtorch_install `
        -G "Visual Studio 16 2019" -A x64 `
        -S . -B libtorch_build
    cmake --build libtorch_build --config Release --target install

A functional cmake file with the above configurations is prepared for you to build libtorch directly.

.. code:: bat

    cmake -S ./third_party/libtorch/ -B libtorch_build `
        -DCMAKE_INSTALL_PREFIX=libtorch_install `
        -G "Visual Studio 16 2019" -A x64
    cmake --build libtorch_build
