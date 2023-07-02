"""
Module to read CMakeCache.txt file and return it as an object.
Janus, June 2023.
"""

import os
import re
from typing import Dict

whitespace = re.compile(r"\s")                                          # Matches all whitespace characters
comment = re.compile(r"#|//")                                           # Comment lines start with # or //
key_type_value = re.compile(r"(\w+)(-ADVANCED)?:(\w+)=(.*)")            # Splits into three groups (with optional -ADVANCED), e.g. used on CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/c++


class CacheEntry:
    """
    Transforms a raw line from CMakeCache into a parsed entry.
    
    Example of a raw CMakeCache.txt line
    //CXX compiler
    CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/c++
    """

    _key: str = ""     # CMAKE_CXX_COMPILER
    _type: str = ""     # FILEPATH
    _value: str = ""    # /usr/bin/c++

    def __init__(self, raw_line: str):
        optional_advanced: str = None
        _, self._key, optional_advanced, self._type, self._value, _ = key_type_value.split(raw_line)     # "CMAKE_CXX_COMPILER", "FILEPATH", "/usr/bin/c++""
        if (optional_advanced):
            self._key += "_ADVANCED"
    
    @property
    def key(self) -> str:
        return self._key
    
    @property
    def type(self) -> str:
        return self._type
    
    @property
    def value(self) -> str:
        return self._value


class CMakeCache:
    """
    Parses a CMakeCache.txt file into Dictionary of key-value pairs.
    Comment lines are ignored (no semantic value).
    """

    _entries: Dict[str, str] = {}

    def __init__(self, cmake_cache_filename: str):

        print(f"Reading {cmake_cache_filename}.")

        if not bool(os.path.isfile(cmake_cache_filename)):
            raise FileNotFoundError
        
        with open(cmake_cache_filename, "r") as file:
            while(line := file.readline()):
                line = whitespace.sub("", line)
                if len(line)==0 or comment.match(line):
                    continue                        # Skip blank lines and comments
                
                # Process and store the entry
                entry = CacheEntry(line)
                self._entries[entry.key] = entry.value
                
    def __getitem__(self, key):
        return self._entries[key]
