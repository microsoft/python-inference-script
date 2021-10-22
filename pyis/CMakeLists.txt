add_subdirectory(share)
add_subdirectory(ops)
add_subdirectory(c_api)

if(PYTHON_BACKEND)
    add_subdirectory(bindings/python)
endif()

if(TORCH_BACKEND)
    add_subdirectory(bindings/torch)
endif()

if(ONNX_BACKEND)
    add_subdirectory(bindings/onnx)
endif()
