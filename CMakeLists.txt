# CMake template for a modern C++17 project with toolchain
# 

# Directory structure:
#   app/            Executable source code
#   build/          CMake and Make build process output, including binaries
#   cmake/          CMake custom modules/functions
#   config/         Header file(s) with configurable parameters
#   docs/           Documentation by Doxygen
#   src/            Own library files source code
#   test/           Test files (suite) and test framework (GoogleTest)
#   third_party/    Third-party libraries and dependencies
#
#
# CMake configuration sources:
# ./
# ├── CMakeLists.txt                             Project (this file)
# ├── app/CMakeLists.txt                         App-executable and dependencies
# ├── src/CMakelists.txt                         Library and dependecies
# ├── config/CMakeLists.txt                      Auto-populate source code with project name, version, etc. 
# ├── test/CMakeLists.txt                        Unit test executable using GoogleTest
# └── third_party/CMakeLists.txt                 get/manage 3rd party dependencies
#
#
# To provide from environment or at the commandline
# Var./opt.             Default/fallback
# -------------         ------------------------
# $PROJECT_NAME         None
# -DCMAKE_BUILD_TYPE    Debug
# -DBUILD_TESTS         False
# -DUSE_CONAN           False - Use FetchContent
#
#
# Code quality tools:
# -------------------
# - Code formatting:                        ClangFormat                             https://clang.llvm.org/docs/ClangFormat.html
#
# - Static linting (pre-compile):           Clang-Tidy                              https://clang.llvm.org/extra/clang-tidy/
#                                                                                   https://github.com/llvm-mirror/clang-tools-extra/blob/master/clang-tidy/tool/run-clang-tidy.py
# - Warning levels and warnings-as-errors:  Per compilation
#                                           Per file                                https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
# - Sanitizers (run-time):                  Asan (*nix, win),                       https://clang.llvm.org/docs/AddressSanitizer.html 
#                                           UBsan (*nix only)                       https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
# - Unit tests (behavioral, functional):    Google Test & Mock                      https://github.com/google/googletest
#
# - Dependency graph output                 build/dependency_graph.png
#
#
# Package Management tools:                 Shared package location
# ------------------------------------      --------------------------------------------------------------------------
# - Git submodule management via CMake      -> third_party/
# - FetchContent built-in CMake             -> build/_deps/
# - Conan                                   -> packages installed in /root/.conan2/p/ and scripts in build/conan_deps/
#
# Janus Bo Andersen, 2023.

cmake_minimum_required(VERSION 3.16)

project(
    $ENV{PROJECT_NAME}
    VERSION 1.0.0
    LANGUAGES CXX)

# Define compiler standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type, check if set from command line, otherwise default to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}.")

# Third-party package manager for dependencies. If none is set, defaults to CMake built-in FetchContent.
# Packages are managed in third_party/CmakeLists.txt AND any files related to the external manager.
if(NOT DEFINED USE_CONAN)
    set(USE_CONAN FALSE)
endif()
message(STATUS "Use Conan: ${USE_CONAN}.")

# Expand the if() any with additional external package managers...
if(USE_CONAN)
    set(USE_EXTERNAL_PACKAGE_MANAGER ON)
else()
    set(USE_EXTERNAL_PACKAGE_MANAGER OFF)
endif()
message(STATUS "Use external package manager: ${USE_EXTERNAL_PACKAGE_MANAGER}.")

# Code coverage only makes sense for a debug build with tests
# Also modify test/CMakeLists.txt to ensure correct files
if((CMAKE_BUILD_TYPE STREQUAL "Debug") AND (BUILD_TESTS))
    set(ENABLE_COVERAGE TRUE)
    message(STATUS "Enabled coverage.")
else()
    set(ENABLE_COVERAGE FALSE)
    message(STATUS "Disabled coverage.")
endif()

# Names for compile targets
set(LIBRARY_NAME    "${CMAKE_PROJECT_NAME}_lib")
set(TESTRUNNER_NAME "${CMAKE_PROJECT_NAME}_test_run")
set(EXECUTABLE_NAME "${CMAKE_PROJECT_NAME}_run")

# Report warnings and treat warnings as errors during compilation.
# Uses the custom Warnings.cmake function in the CMakeList of each target.
# add_compile_options(-Wall -Wextra -pedantic -Werror)
set(ENABLE_WARNINGS TRUE)
set(ENABLE_WARNINGS_AS_ERRORS TRUE)

# Sanitizers to catch hard to detect errors
set(ENABLE_SANITIZE_ADDRESS TRUE)       # Catch memory errors
set(ENABLE_SANITIZE_UNDEFINED TRUE)     # Catch undefined behavior

# Linting
set(ENABLE_CLANG_TIDY TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Link-time optimization. Rmb. to also call target_enable_lto(...) on each compile-target.
set(ENABLE_LTO FALSE)

# Import custom CMake modules here
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(AddGitSubmodule)        #-> add_git_submodule(...)
include(Docs)                   #-> emits custom target 'docs'
include(Warnings)               #-> target_set_warnings(...)
include(Sanitizers)             #-> add_sanitizer_flags(...)
include(LinkTimeOptimization)   #-> target_enable_lto(...)
include(Linters)                #-> add_clang_tidy_to_target(...)
include(CodeCoverage)           #-> append_coverage_compiler_flags,
                                #   append_coverage_compiler_flags_to_target
                                #   setup_target_for_coverage_lcov(), 
                                #   setup_target_for_coverage_gcovr_xml, 
                                #   setup_target_for_coverage_gcovr_html, 
                                #   setup_target_for_coverage_fastcov

                                
# Set global compiler options via CMake modules
# target-based options are set in individual CMakeList.txt files
add_sanitizer_flags(${ENABLE_SANITIZE_ADDRESS} ${ENABLE_SANITIZE_UNDEFINED})

if(ENABLE_COVERAGE)
    append_coverage_compiler_flags()        # appends --coverage and some management of absolute paths.
endif()

# Using config files for global CMake/project-parameters to be baked into code
# Config file (config/config.hpp.in) is populated by CMake, and then available as header for #include <...>
set(CONFIG_PATH "${CMAKE_BINARY_DIR}/config/include/")
include_directories(${CONFIG_PATH})
add_subdirectory(config)

# Configure third-party dependencies (external CMake projects)
# Remember to also add third-party library name to the target_link_libraries for the relevant targets.
# Use CMake Fetch module to configure third-party dependencies
include(FetchContent)
add_subdirectory(third_party)

# Configure third-party dependencies (not CMake projects) using custom CMake module
# Remember to add new targets
# add_git_submodule(third_party/name)

# Build library for this project from src/.
# Header files are propagated via PUBLIC to all targets that link to it (linking is done below).
add_subdirectory(src)

# Build unit tests from test/ (using Google Test Framework), in case that option is set ON from command line.
if(BUILD_TESTS)
    add_subdirectory(test)
    #string(APPEND CMAKE_CXX_FLAGS " --coverage")
endif()

# Build the app from app/
add_subdirectory(app)
