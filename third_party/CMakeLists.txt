

message(STATUS "Configuring third-party libraries with package manager.")

if(USE_CONAN)
    # Packages from: https://conan.io/center/.
    # Conan resolves dependencies and versions using conanfile.py.
    # Install using command line tool "conan".
    # CMake scripts to compile use these packages come via include(<...>/conan_toolchain.cmake) and/or -DCMAKE_TOOLCHAIN_FILE=<...>/conan_toolchain.cmake.
    # Packages must be made available using find_package(<package-name> REQUIRED) in the CMakeLists.txt of each dependant compilation target.

    message(STATUS "Running Conan2 package manager.")
    #include(${CMAKE_BINARY_DIR}/conan_deps/conan_toolchain.cmake)

else()
    # FetchContent downloads and prepares dependencies. Slower than Conan, as all packages are built from source.
    # Declare step      : Set up and clone third-party CMake library from github/gitlab, but do not clone recursively (shallow).
    # MakeAvailable step: Make available inside our CMake project. Available for all compilation targets (don't use find_package()).

    message(STATUS "Running FetchContent package manager.")

    # nlohmann/json: JSON parser
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.11.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)

    # fmt: String formatting lib
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 9.1.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(fmt)

    # gtest: Google Test & Mock
    FetchContent_Declare(
        gtest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG 1.13.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(gtest)

    # spdlog: Logging lib
    #FetchContent_Declare(
    #    spdlog
    #    GIT_REPOSITORY https://github.com/gabime/spdlog
    #    GIT_TAG 1.11.0
    #    GIT_SHALLOW TRUE
    #)
    #FetchContent_MakeAvailable(spdlog)

    # cxxopts: Commandline option parser
    #FetchContent_Declare(
    #    cxxopts
    #    GIT_REPOSITORY https://github.com/jarro2783/cxxopts
    #    GIT_TAG v3.1.1
    #    GIT_SHALLOW TRUE
    #)
    #FetchContent_MakeAvailable(cxxopts)

    # Catch2: Simpler unit-testing library
    #FetchContent_Declare(
    #    Catch2 
    #    GIT_REPOSITORY https://github.com/catchorg/Catch2
    #    GIT_TAG v2.13.9
    #    GIT_SHALLOW TRUE
    #)
    #FetchContent_MakeAvailable(Catch2)
endif()