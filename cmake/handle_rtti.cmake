# Disable RTTI and turn usage of dynamic_cast and typeid into errors
if(PYIS_NO_RTTI)
    if(MSVC)
        add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/GR->"
                            "$<$<COMPILE_LANGUAGE:CXX>:/we4541>")
    else()
        add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>")
    endif()
endif()
