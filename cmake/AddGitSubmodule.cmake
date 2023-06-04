# Function add_git_submodule fetches/updates third party "dir".
# Requires that the submodule is already declared in Git.
# If "dir" doesn't exist/doesn't have a CMakeLists.txt then the submodule fetches it
# by running a the git submodule command.
# Usage:
#  - git submodule add https://github.com/nlohmann/json third_party/json
#  - 

function(add_git_submodule dir)

    # Find Git executable
    find_package(Git REQUIRED)

    if (NOT EXISTS ${dir}/CMakeLists.txt)

        # Run git submodule command
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${dir}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif()

    # Add to targets
    add_subdirectory(${dir})

endfunction(add_git_submodule)
