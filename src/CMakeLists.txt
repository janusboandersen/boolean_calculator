# List source files for our library
set(LIB_SOURCES
    pcn.cpp
    meta.cpp
    algorithm.cpp
    file_adaptor.cpp
    
)

# List header files, if needed for pre-compiling
set(LIB_HEADERS
    include/urp/pcn.h
    include/urp/meta.h
    include/urp/algorithm.h
    include/adaptor/file_adaptor.h
    )

# Include paths from our library
set(LIB_INCLUDES 
    "./include/")


# Third-party dependencies in our library
# Find these if using an external package manager
if(USE_EXTERNAL_PACKAGE_MANAGER)
    # Config-mode search for the packages. The <package>-config.cmake files are in build/conan_deps/
    message(STATUS "Finding packages for ${LIBRARY_NAME}.")
    #find_package(nlohmann_json REQUIRED)
    #find_package(fmt REQUIRED)
    #find_package(lexy REQUIRED)
endif()

set(LIB_DEPENDENCIES
    #nlohmann_json::nlohmann_json
    #fmt::fmt
    #foonathan::lexy
)

# Create compiled, statically-linked library (.a) from the sources. Variable defined in parent CMakeLists.txt.
add_library(${LIBRARY_NAME} STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# Propagate our library's include path(s) for the precompiler to locate header files used in #include <...> clauses in other compilation units.
# Any target that links this library also needs its include directory.
target_include_directories(${LIBRARY_NAME} PUBLIC ${LIB_INCLUDES})

# Link the dependencies for our library
# PUBLIC/PRIVATE/INTERFACE linking affects downstream users of our library (e.g. other libraries):
#   PRIVATE:    OUR <- DEP.     Later: OTHER <- OUR.            OTHER can't use API of DEP. DEP gets private API. Need to link OTHER <- DEP.
#   PUBLIC:     OUR <- DEP.     Later: OTHER <- OUR (<- DEP).   OTHER can now use API of DEP. DEP gets public API. No need to link again.
#   INTERFACE:  Header-only libraries.
target_link_libraries(${LIBRARY_NAME} PUBLIC ${LIB_DEPENDENCIES})

# Report warnings and use warnings as errors during compilation.
if(${ENABLE_WARNINGS})
    target_set_warnings(
        ${LIBRARY_NAME}
        ${ENABLE_WARNINGS}
        ${ENABLE_WARNINGS_AS_ERRORS})
endif()

# Potentially enable LTO
target_enable_lto(${LIBRARY_NAME} ${ENABLE_LTO})

# Potentially enable linting
if(ENABLE_CLANG_TIDY)
    add_clang_tidy_to_target(${LIBRARY_NAME})
endif()
