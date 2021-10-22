# onnxruntime

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(ONNXRUNTIME_URL
        https://github.com/microsoft/onnxruntime/releases/download/v${ORT}/onnxruntime-win-x64-${ORT}.zip
    )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(ONNXRUNTIME_URL
        https://github.com/microsoft/onnxruntime/releases/download/v${ORT}/onnxruntime-linux-x64-${ORT}.tgz
    )
    set(BINARY_EXTENSION "so")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(ONNXRUNTIME_URL
        https://github.com/microsoft/onnxruntime/releases/download/v${ORT}/onnxruntime-osx-x64-${ORT}.tgz
    )
    set(BINARY_EXTENSION "dylib")
endif()

FetchContent_Declare(onnxruntime URL ${ONNXRUNTIME_URL})
FetchContent_MakeAvailable(onnxruntime)

set(ONNXRUNTIME_INCLUDE_DIR
    ${onnxruntime_SOURCE_DIR}/include
    CACHE PATH "" FORCE)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(ONNXRUNTIME_LIBS
        ${onnxruntime_SOURCE_DIR}/lib/onnxruntime.lib
        CACHE PATH "" FORCE)
    set(ONNXRUNTIME_DLLS
        ${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll
        ${onnxruntime_SOURCE_DIR}/lib/onnxruntime.pdb # useful for debugging into onnxruntime
        CACHE PATH "" FORCE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    file(GLOB ONNXRUNTIME_DLLS ${onnxruntime_SOURCE_DIR}/lib/*.${BINARY_EXTENSION})
    set(ONNXRUNTIME_DLLS
        ${ONNXRUNTIME_DLLS}
        CACHE PATH "" FORCE)
    set(ONNXRUNTIME_LIBS
        ${ONNXRUNTIME_DLLS}
        CACHE PATH "" FORCE)
endif()

#set(ONNXRUNTIME_LIBS
#    D:/src/github/onnxruntime/build/Windows/RelWithDebInfo/RelWithDebInfo/onnxruntime.lib
#    CACHE PATH "" FORCE)
#set(ONNXRUNTIME_DLLS
#    D:/src/github/onnxruntime/build/Windows/RelWithDebInfo/RelWithDebInfo/onnxruntime.dll
#    D:/src/github/onnxruntime/build/Windows/RelWithDebInfo/RelWithDebInfo/onnxruntime.pdb
#    CACHE PATH "" FORCE)

message("ONNXRUNTIME_DLLS=${ONNXRUNTIME_DLLS}")
