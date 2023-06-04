# This file sets up individual test cases / suite into an executable linked with Google Test

# DEPRECATED: The GoogleTest framework has its own folder, which is dropped-in from its github page 
# DEPRECATED: (i.e., no system-wide installation or dependency configuration).
# DEPRECATED: Use the following three lines if GTest is in-source (test/GoogleTest_Framework)
# DEPRECATED: add_subdirectory(GoogleTest_Framework)
# DEPRECATED: include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
# DEPRECATED: include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

# Our own test executable main() (discovers all compiled tests and runs them)
set(TEST_EXECUTABLE main_test.cpp)
set(TEST_DIR testcases)

# The test files containing unit tests/cases to be compiled (no auto file discovery!).
set(TEST_FILES 
    ${TEST_DIR}/test_pcn.cpp
    ${TEST_DIR}/test_cofactors.cpp
    ${TEST_DIR}/test_algebra.cpp
    ${TEST_DIR}/test_singular.cpp
)

# Test executable: target name is SomeProjectName_test_run.
# Executable uses the main() in main_test.cpp to run all tests in TEST_FILES.
add_executable(${TESTRUNNER_NAME} ${TEST_EXECUTABLE} ${TEST_FILES})

# The Google Test dependency is managed via Conan or FetchContent.
if(USE_EXTERNAL_PACKAGE_MANAGER)
    # Config-mode search for the packages. The <package>-config.cmake files are in build/conan_deps/
    message(STATUS "Finding packages for ${TESTRUNNER_NAME}.")
    
    # https://cmake.org/cmake/help/latest/module/FindGTest.html
    find_package(GTest REQUIRED)
endif()

# Link our unit test executable (main_test.cpp) to our library and to the Test Framework.
# We are currently using our own entry point from main_test.cpp. Otherwise, link against BOTH gtest and gtest_main.
target_link_libraries(${TESTRUNNER_NAME} PUBLIC ${LIBRARY_NAME})
#  DEPRECATED: target_link_libraries(${TESTRUNNER_NAME} PUBLIC gtest)
target_link_libraries(${TESTRUNNER_NAME} PUBLIC GTest::gtest GTest::gmock)

if(ENABLE_COVERAGE)

    message(STATUS "Coverage for ${TESTRUNNER_NAME} is ON.")

    # Define coverage scope, in particular, we only want to test the library (src/) and thus exclude all other things external to that.
    set(COVERAGE_MAIN "coverage")
    set(COVERAGE_EXCLUDES
        "${PROJECT_SOURCE_DIR}/app/*"
        "${PROJECT_SOURCE_DIR}/build/*"
        "${PROJECT_SOURCE_DIR}/cmake/*"
        "${PROJECT_SOURCE_DIR}/docs/*"
        "${PROJECT_SOURCE_DIR}/test/*"
        "${PROJECT_SOURCE_DIR}/third_party/*"
        "${PROJECT_SOURCE_DIR}/tools/*"
        "/usr/include/*"
        "/root/.conan2/*")

    # Make coverage for the testrunner only
    setup_target_for_coverage_lcov(
        NAME ${COVERAGE_MAIN}
        EXECUTABLE ${TESTRUNNER_NAME}
        DEPENDENCIES ${TESTRUNNER_NAME})

endif()
