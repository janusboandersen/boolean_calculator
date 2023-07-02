
"""
Parses a conan_toolchain.cmake file to get a line fitting a certain pattern.

Specifically made to find the include paths from the line
list(PREPEND CMAKE_INCLUDE_PATH <...list of include paths...>)

Janus, June 2023.
"""

import os
import re
from typing import List

include_paths_pattern = re.compile(r"list\(PREPEND\sCMAKE_INCLUDE_PATH\s(\".*\")")


def get_conan_include_paths(cmake_toolchain_filepath: str) -> List[str]:
    """Fetch line from cmake_toolchain.cmake file and return list of include paths."""
    if not os.path.isfile(cmake_toolchain_filepath):
        raise FileNotFoundError(f"File not found {cmake_toolchain_filepath}.")
    
    line: str = ""
    with open(cmake_toolchain_filepath, 'r') as file:
        while (line := file.readline()):
            if include_paths_pattern.match(line):
                break

    paths_str: str = include_paths_pattern.search(line).group(1)
    return paths_str.replace('"', '').split(' ')


if __name__ == '__main__':
    get_conan_include_paths("build/conan_deps/conan_toolchain.cmake")
