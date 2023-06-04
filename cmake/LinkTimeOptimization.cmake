# Enable LTO for a target
# Linker may then cancel out nested function calls, etc.
function(target_enable_lto TARGET EN_LTO)
    if(NOT ${EN_LTO})
        return()
    endif()

    # Use Cmake built-in module to check if Inter-Procedure Optimization is supported
    include(CheckIPOSupported)

    # Call to the function returns
    # - boolean result in the 'result' var.
    # 
    check_ipo_supported(RESULT result OUTPUT output)

    if(result)

        message(STATUS "LTO supported and enabled for ${TARGET}.")
        set_property(
            TARGET ${TARGET}
            PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

    else()

        message(WARNING "LTO not supported!")

    endif()

endfunction()