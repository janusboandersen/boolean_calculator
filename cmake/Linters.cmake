# To run Clang-Tidy, we can use the clang-tidy commandline tool, or use the multi-threaded python script for running clang-tidy.

function(add_clang_tidy_to_target TARGET)

    # Find all source-files for this target
    # https://cmake.org/cmake/help/v3.16/command/get_target_property.html
    get_target_property(
        TARGET_SOURCES
        ${TARGET}
        SOURCES)

    # Filter list of source files to only include C++ sources/headers
    # https://cmake.org/cmake/help/v3.16/command/list.html
    # https://cmake.org/cmake/help/v3.16/command/list.html#filter
    list(
        FILTER              # Operation
        TARGET_SOURCES      # <list>
        INCLUDE             # include items matching pattern
        REGEX
        ".*.(h|hpp|cpp|cc)" # <regex> filename with one of the specified extensions
    )

    # Get python interpreter executable
    find_package(Python3 COMPONENTS Interpreter)
    if(NOT ${PYTHON_FOUND})
        message(WARNING "Install Python3 to use Clang-Tidy.")
        return()
    endif()

    # https://cmake.org/cmake/help/v3.16/command/find_program.html
    find_program(
        CLANGTIDY           # cache <var> storing result
        clang-tidy          # program name
    )

    # No clang-tidy, try again later
    if(NOT CLANGTIDY)
        message(WARNING "Install clang-tidy first.")
        return()
    endif()

    # Using Clang-Tidy for CLANG-compiler or GCC/GNU
    if(CMAKE_CXX_COMPILER_ID MATCHES "CLANG" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message(STATUS "Adding Clang-Tidy linter for target ${TARGET} (${TARGET}_clangtidy) files (${TARGET_SOURCES}).")

        # By making a custom commandline target that can be run before compilation
        add_custom_target(
            ${TARGET}_clangtidy                                         # target name
            COMMAND                                                     # to run on commandline
                ${Python3_EXECUTABLE}
                "${CMAKE_SOURCE_DIR}/tools/run-clang-tidy.py"           # parallel script from https://github.com/llvm-mirror/clang-tools-extra/blob/master/clang-tidy/tool/run-clang-tidy.py
                ${TARGET_SOURCES}
                "-config-file=${CMAKE_SOURCE_DIR}/tools/.clang-tidy"    # YAML config file
                "-extra-arg-before=-std=${CMAKE_CXX_STANDARD}"         
                -header-filter="\(src|app\)\/*.\(h|hpp\)"               # Only from src/ or app/ directories
                "-p=${CMAKE_BINARY_DIR}"                                # Build path
                "-use-color"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            USES_TERMINAL
        )

    endif()

endfunction()