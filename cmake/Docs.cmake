find_package(Doxygen)

if(DOXYGEN_FOUND)
    # Utility target to be created in terminal, by running doxygen inside the docs/ directory
    add_custom_target(
        docs
        ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/docs"
    )
endif()