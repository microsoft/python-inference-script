# libzip

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.2.11)

FetchContent_MakeAvailable(zlib)

FetchContent_Declare(
    libzip
    GIT_REPOSITORY https://github.com/nih-at/libzip.git
    GIT_TAG v1.8.0)
set(ENABLE_COMMONCRYPTO
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_GNUTLS
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_MBEDTLS
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_OPENSSL
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_BZIP2
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_LZMA
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_MBEDTLS
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_ZSTD
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_TOOLS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_REGRESS
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_DOC
    OFF
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libzip)
