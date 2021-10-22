add_library(pyis_onnx_static STATIC)

target_sources(
    pyis_onnx_static
    PRIVATE "onnx_base_kernel.cpp"
            "onnx_base_kernel.h"
            "onnx_tensor_dimensions.cpp"
            "onnx_tensor_dimensions.h"
            "language_detector/language_detector_op.cpp"
            "language_detector/language_detector_op.h"
            "tokenizers/bert_tokenizer_op.h"
            "tokenizers/bert_tokenizer_op.cpp")

target_include_directories(pyis_onnx_static PUBLIC ${ONNXRUNTIME_INCLUDE_DIR})
target_link_libraries(pyis_onnx_static PUBLIC pyis_operators)
target_compile_definitions(pyis_onnx_static PUBLIC ORT_API_MANUAL_INIT)

add_library(pyis_onnx SHARED)
target_sources(pyis_onnx PRIVATE "onnx_exports.h" "onnx_exports.cpp")

if(MSVC)
    target_sources(pyis_onnx PRIVATE "pyis_onnx.def")
endif()

target_link_libraries(pyis_onnx PRIVATE pyis_onnx_static)
