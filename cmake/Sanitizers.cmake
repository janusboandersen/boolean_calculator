# Set sanitizer flags for different compilers/linkers
# Sanitizers must be set for the entire compilation, i.e. for ALL targets or for no targets.
function(add_sanitizer_flags EN_ADDR_FLAG EN_UNDEF_FLAG)

    if(NOT ${EN_ADDR_FLAG} AND NOT ${EN_UNDEF_FLAG})
        message(STATUS "Sanitizers disabled.")
        return()
    endif()

    # Sanitizer flags per compiler vendor
    if(CMAKE_CXX_COMPILER_ID MATCHES "CLANG" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")

        # Setting GLOBAL compile and link options (ALL targets)
        # Track stack usage by always storing frame pointers in a dedicated register (even if a function doesn't really need one)
        add_compile_options("-fno-omit-frame-pointer")
        add_link_options("-fno-omit-frame-pointer")

        if(${EN_ADDR_FLAG})
            # Detect memory errors, e.g. out-of-bounds access, use-after-free/-after-return/-after-scope, double-free
            # On supported platforms, leak detection should be enabled by default by the AddressSanitizer (otherwise try -fsanitize=leak)
            # https://clang.llvm.org/docs/AddressSanitizer.html
            add_compile_options("-fsanitize=address")
            add_link_options("-fsanitize=address")
        endif()

        if(${EN_UNDEF_FLAG})
            # Perform all checks for undefined behavior per lang. standard, e.g. overflows. Can alt. enable individual checks.
            # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
            add_compile_options("-fsanitize=undefined")
            add_link_options("-fsanitize=undefined")
        endif()

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

        if(${EN_ADDR_FLAG})
            # ASan for MSVC 19+
            # https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
            add_compile_options("/fsanitize=address")
            add_link_options("/fsanitize=address")
        endif()

    else()

        message(ERROR "Sanitizers not supported for ${CMAKE_CXX_COMPILER_ID} compiler.")

    endif()

endfunction()