if(PYTHON_BACKEND)
    add_subdirectory(pyis-python)
endif()

if(TORCH_BACKEND)
    add_subdirectory(pyis-torch)
endif()

if(ONNX_BACKEND)
    add_subdirectory(pyis-onnx)
endif()
