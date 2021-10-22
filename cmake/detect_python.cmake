# detect python executable
if(PYTHON STREQUAL "")
    find_package(Python3 COMPONENTS Interpreter Development)
else()
    find_package(Python3 ${PYTHON} EXACT COMPONENTS Interpreter Development)
endif()
if(NOT ${Python3_FOUND} OR NOT ${Python3_Interpreter_FOUND})
    message(
        FATAL_ERROR
            "Python_FOUND=${Python3_FOUND} Python_Interpreter_FOUND=${Python3_Interpreter_FOUND}")
endif()
set(Python3_EXECUTABLE
    ${Python3_EXECUTABLE}
    CACHE FILEPATH "" FORCE)
message(PYTHON_EXECUTABLE=${Python3_EXECUTABLE})
