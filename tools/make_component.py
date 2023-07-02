#!/usr/bin/env python3

"""
Make scaffolding for a component

Command: tools/make_component.py <group> <component_longname>
Effect: Will create scaffolding for a component contained in "group" with a name of "Component longname".
Example: tools/make_component.py algo merge_sort
  
Scaffolding is created using the templates in tools/templates, which are populated using this current script. The following filesystem effects occur:

1) Three new files will be created:
  + src/include/<group>/<component_longname>.hpp
  + src/<group_component_longname>.cpp
  + test/testcases/<group>/<component_longname>.cpp

2) Prompts are issued to make the required manual modifications (copy-paste) in CMake-files:
  src/CMakeLists.txt:
  -> LIB_HEADERS: + include/<group>/<component_longname>.hpp
  -> LIB_SOURCES: + <group_component_longname>.cpp

  test/CMakeLists.txt:
  -> TEST_FILES: + ${TEST_DIR}/<group>/<component_longname>.cpp

Janus Bo Andersen, June 2023.

"""

import sys
import os
from datetime import date
from jinja2 import Environment, FileSystemLoader, select_autoescape

env = Environment(
    loader=FileSystemLoader(os.path.join("tools", "templates")),
    autoescape=select_autoescape()
)

HEADER_EXT = ".hpp"
SOURCE_EXT = ".cpp"

def get_group() -> str:
    return sys.argv[1]

def get_component_longname() -> str:
    return sys.argv[2]

def make_component_name_cleartext(component_longname: str) -> str:
    return component_longname.replace("_", " ").capitalize()

# Header file: merge_sort.hpp
def make_header_filename(group: str, component_longname: str) -> str:
    return component_longname + HEADER_EXT

# Directory of header file: src/include/algo
def make_header_rel_path(group: str, component_longname: str) -> str:
    return os.path.join("src", "include", group)

# Path of header file: src/include/algo/merge_sort.hpp
def make_header_rel_path_with_filename(group: str, component_longname: str) -> str:
    return os.path.join(
        make_header_rel_path(group, component_longname),
        make_header_filename(group, component_longname)
    )

# Include path of header file: algo/merge_sort.hpp   ->   #include <algo/merge_sort.hpp>
def make_include_header(group: str, component_longname: str) -> str:
    return group + "/" + component_longname + HEADER_EXT


# Source code file: algo_merge_sort.cpp
def make_source_filename(group: str, component_longname: str) -> str:
    return os.path.join(group + "_" + component_longname + SOURCE_EXT)

# Source code directory: src
def make_source_rel_path(group: str, component_longname: str) -> str:
    return os.path.join("src")

# Path of source code file: src/algo_merge_sort.cpp
def make_source_rel_path_with_filename(group: str, component_longname: str) -> str:
    return os.path.join(
        make_source_rel_path(group, component_longname),
        make_source_filename(group, component_longname)
    )


# Test file name: test_merge_sort.cpp
def make_test_filename(group: str, component_longname: str) -> str:
    return os.path.join("test_" + component_longname + SOURCE_EXT)

# Test file directory: test/testcases/algo
def make_test_rel_path(group: str, component_longname: str) -> str:
    return os.path.join("test", "testcases", group)

# Test file path: test/testcases/algo/test_merge_sort.cpp
def make_test_rel_path_with_filename(group: str, component_longname: str) -> str:
    return os.path.join(
        make_test_rel_path(group, component_longname),
        make_test_filename(group, component_longname)
    )


# File system effects: Creates full path and writes to file, but doesn't overwrite any existing files
def write_to_file(basepath: str, rel_path: str, filename: str, textdata: str) -> int:
    path = os.path.join(basepath, rel_path)
    path_and_file = os.path.join(path, filename)

    if not os.path.exists(path):
        print("Making path: " + path)
        os.makedirs(path)

    if not os.path.isfile(path_and_file):
        print("Making file: " + path_and_file)
        f = open(path_and_file, "wt")
        n = f.write(textdata)
        f.close()
        return n
    else:
        print("File already exists: " + path_and_file)
        return 0


if __name__ == "__main__":
    component_longname = get_component_longname()
    component_group = get_group()
    
    render_vars = {
        "header_file": make_header_rel_path_with_filename(component_group, component_longname),
        "include_header": make_include_header(component_group, component_longname),
        "source_file": make_source_rel_path_with_filename(component_group, component_longname),
        "test_file": make_test_rel_path_with_filename(component_group, component_longname),
        "component_group": component_group,
        "component_longname": component_longname,
        "component_longname_clear_text": make_component_name_cleartext(component_longname),
        "make_date": date.today().isoformat(),
        "make_year": date.today().year,
        "component_namespace": component_group
    }

    # Build header file
    header_template = env.get_template("header.hpp.jinja2")
    write_to_file(
        os.getcwd(),
        make_header_rel_path(component_group, component_longname),
        make_header_filename(component_group, component_longname),
        header_template.render(**render_vars)
    )

    # Build source file
    source_template = env.get_template("source.cpp.jinja2")
    write_to_file(
        os.getcwd(),
        make_source_rel_path(component_group, component_longname),
        make_source_filename(component_group, component_longname),
        source_template.render(**render_vars)
    )

    # Build test file
    test_template = env.get_template("test.cpp.jinja2")
    write_to_file(
        os.getcwd(),
        make_test_rel_path(component_group, component_longname),
        make_test_filename(component_group, component_longname),
        test_template.render(**render_vars)
    )

    # Give user strings to copy to CMakeLists

    # src/CMakeLists.txt
    lib_sources = make_source_filename(component_group, component_longname)
    lib_headers = f"include/{component_group}/{make_header_filename(component_group, component_longname)}"
    print("\nManual edit required: src/CMakeLists.txt ->")
    print("LIB_SOURCES: " + lib_sources)        # algo_merge_sort.cpp
    print("LIB_HEADERS: " + lib_headers)        # include/algo/merge_sort.hpp
    
    #test/CMakeLists.txt
    test_files = "${TEST_DIR}/" + f"{component_group}/{make_test_filename(component_group, component_longname)}"
    print("\nManual edit required: test/CMakeLists ->")
    print("TEST_FILES: " + test_files)          # ${TEST_DIR}/algo/test_merge_sort.cpp
