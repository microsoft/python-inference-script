add_library(ziplib STATIC)

file(GLOB_RECURSE ZIPLIB_SOURCE_FILES Source/ZipLib/*.cpp Source/ZipLib/*.c)
target_sources(ziplib PRIVATE ${ZIPLIB_SOURCE_FILES})

target_include_directories(ziplib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/ZipLib/)
