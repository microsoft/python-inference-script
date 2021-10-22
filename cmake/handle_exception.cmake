# Disable exceptions
if(PYIS_NO_EXCEPTIONS)
    if(MSVC)
        add_compile_options("/EHs-c-" "/wd4834" "/wd4702")
        # Microsoft STL supports exception deactivation
        add_compile_definitions("_HAS_EXCEPTIONS=0")
    else()
        add_compile_options("-fno-exceptions" "-fno-unwind-tables" 
                            "-fno-asynchronous-unwind-tables")
    endif()
endif()
