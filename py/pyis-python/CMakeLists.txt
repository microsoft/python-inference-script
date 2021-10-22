if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(DIST_PLATFORM "--plat-name=win_amd64")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DIST_PLATFORM "--plat-name=manylinux1_x86_64")
endif()

add_custom_target(
    pyis_python_whl ALL
    DEPENDS pyis_python
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/pyis/python/lib/
    COMMAND ${CMAKE_COMMAND} -E copy ${ONNXRUNTIME_DLLS}
            ${CMAKE_CURRENT_SOURCE_DIR}/pyis/python/lib/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pyis_python>
            ${CMAKE_CURRENT_SOURCE_DIR}/pyis/python/lib/
    COMMAND "${Python3_EXECUTABLE}" setup.py bdist_wheel ${DIST_PLATFORM}
    COMMAND "${Python3_EXECUTABLE}" -m pip uninstall -y pyis-python
    COMMAND "${Python3_EXECUTABLE}" -m pip install -e .
    COMMAND "stubgen" -p pyis.python.lib.pyis_python -o .
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_test(
    NAME test_pyis_python_whl
    COMMAND "${Python3_EXECUTABLE}" -m unittest discover tests/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set_tests_properties(test_pyis_python_whl PROPERTIES ENVIRONMENT CTEST_OUTPUT_ON_FAILURE=TRUE)
