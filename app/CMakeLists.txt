
# Declare main executable to run project, like SomeProjectName_run. Variable defined in parent CMakeLists.txt.
set(EXE_SOURCES main.cpp)
add_executable(${EXECUTABLE_NAME} ${EXE_SOURCES})

# Link library to app.
target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${LIBRARY_NAME})

# Report warnings and use warnings as errors during compilation.
if(${ENABLE_WARNINGS})
    target_set_warnings(
        ${EXECUTABLE_NAME}
        ${ENABLE_WARNINGS}
        ${ENABLE_WARNINGS_AS_ERRORS})
endif()
