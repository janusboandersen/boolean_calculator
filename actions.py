#!/usr/bin/env python3

"""
Easy access to actions for the project through automated command-line invokations.
Janus Bo Andersen, 2023.

Examples: 

    Help on how to invoke from the command line
    ./actions.py --help

    Install dependencies with Conan and configure project with CMake:
    ./actions.py --conan-install --configure-project --with-tests --debug

    See some available targets after configuration
    ./actions.py --build-target-list
    ./actions.py --test-target-list
    ./actions.py --clang-tidy-target-list
    ./actions.py --docs-target-list

    Subsequently build (compile) and run the app:
    ./actions.py --build-run-project

    All in one step:
    ./actions.py --conan-install --configure-project --build-run-project --with-tests --debug

    Unit tests were also built (--with-tests), so run these:
    ./actions.py --build-test-project

    As above, and also run the code coverage analysis (extra compile step)
    ./actions.py --build-test-project --run-coverage

    Emit configuration JSON files for VSCode
    ./actions.py --vscode-launch
    ./actions.py --vscode-tasks --with-tests --use-conan
    ./actions.py --vscode-properties --with-tests --use-conan       

Tips:
- Easiest, if this file is executable (chmod +x). Otherwise use python3 ...

This tool works as a state-machine through three global dicts
- global_params         : Loaded (/inferred) parameters of the project
- targets               : Known/recognized targets (based on CMakeCache)
- shell_commands        : Valid invokations for the current parameters (based on params)

When running the tool, environment variables and flags/options together MUST provide resolution for the following parameters:

Guarded param.          Env. var.               Flag/option                             CMake interface
-------------           --------------          ------------------------------------    --------------------------------------
project-name            $PROJECT_NAME           None                                    Always gets from env. var.
project-build-type      $PROJECT_BUILD_TYPE     --debug, --release                      Via -DCMAKE_BUILD_TYPE=<Debug/Release>
project-build-tests     $PROJECT_BUILD_TESTS    --with-tests, --build-test-project      Via -DBUILD_TESTS=<ON/OFF>
project-use-conan       $PROJECT_USE_CONAN      --use-conan, --conan-install            Via -DUSE_CONAN=<ON/OFF>

Note: Flags/options will override environment variables.
Note: If any of these are not resolvable, fallback settings are used.

Regarding VSCode configurations, see: https://code.visualstudio.com/docs/cpp/config-msvc#_create-a-build-task

TODO: Figure out how/if to coordinate cppstd across files. Currently locked at 17. Not stored in CMakeCache.txt
TODO: Refactor to move further utility functions to actions_helper.py

Ver. 0.8.0 
Janus, 2023
"""

import argparse
import os
import subprocess
from functools import partial
from collections import OrderedDict
import re
from typing import List, Dict, OrderedDict, Union
import json


from tools.actions_helper import is_on_str, guard_single
from tools.actions_helper import VSCodeLaunch, VSCodeTasks, VSCodeTask, VSCodeTaskOption, VSCodeTaskGroup, VSCodeProperties
from tools.cmakecache_reader import CMakeCache
from tools.conantoolchain_reader import get_conan_include_paths


cppstd = 17     # No effect on CMake, only Conan and VSCode

fallback = {
    "project-name"              : "MISSING_PROJECT_NAME",
    "project-build-type"        : "Debug",
    "project-build-tests"       : "OFF",
    "project-use-conan"         : "ON",
}

global_params = {}
shell_commands = {}
targets = OrderedDict({})


def set_global_params():
    global global_params
    global_params = {
        "project-name"              : os.getenv('PROJECT_NAME'),                                              # Set when container starts
        "project-build-type"        : os.getenv('PROJECT_BUILD_TYPE'),                                        # Modify as needed
        "project-build-tests"       : is_on_str(os.getenv('PROJECT_BUILD_TESTS')),                            # Same
        "project-use-conan"         : is_on_str(os.getenv('PROJECT_USE_CONAN')),

        "base-directory"            : os.getenv('PWD'),
        "build-directory"           : os.path.join(os.getenv('PWD'), "build"),
        "source-directory"          : os.path.join(os.getenv('PWD'), "src"),
        "docs-directory"            : os.path.join(os.getenv('PWD'), "docs"),
        "third-party-directory"     : os.path.join(os.getenv('PWD'), "third_party"),
        "tools-directory"           : os.path.join(os.getenv('PWD'), "tools"),

        "conan-profile-file"        : os.path.join("/root", ".conan2", "profiles", "default"),
        "conan-output-directory"    : os.path.join(os.getenv('PWD'), "build", "conan_deps"),
        "conan-output-toolchain"    : os.path.join(os.getenv('PWD'), "build", "conan_deps", "conan_toolchain.cmake"),

        "dependency-graph-input"    : os.path.join(os.getenv('PWD'), "build", "dependency_graph", "dependency_graph.dot"),
        "dependency-graph-output"   : os.path.join(os.getenv('PWD'), "build", "dependency_graph.png"),

        "cmake-cache-file"          : os.path.join(os.getenv('PWD'), "build", "CMakeCache.txt"),
        "cmake-build-parallel-level": 8,                                                                        # Number of cores -j8

        "cmake-compile-commands"    : os.path.join(os.getenv('PWD'), "build", "compile_commands.json"),

        "binary-app-directory"      : os.path.join(os.getenv('PWD'), "build", "app"),
        "binary-app-postfix"        : "_run",

        "binary-test-directory"     : os.path.join(os.getenv('PWD'), "build", "test"),
        "binary-test-postfix"       : "_test_run",
        
        "clang-tidy-target-postfix" : "_clangtidy",

        # If a target list is needed, a CMake config is of course a prerequisite. The below actions run a process to extract target names from the CMake cache.
        "action-requires-targets"   : [
                                        "build_target_list", 
                                        "test_target_list", 
                                        "coverage_target_list",
                                        "run_coverage", 
                                        "clang_tidy_target_list",
                                        "clang_tidy_all",
                                        "clang_tidy_target",
                                        "docs_target_list",
                                        "vscode_tasks"
                                    ],

        # These actions require a run of the guard/resolution of environment variables and flags/options.
        "action-triggers-guard"     : [
                                        "conan_install",            # needs project-build-type
                                        "configure_project",        # needs all guarded variables
                                        "vscode_launch",            # needs project name
                                        "vscode_tasks",             # needs build type and to know about Conan
                                        "vscode_properties"         # needs everything in order to run reconfiguration
                                    ],

        # These actions will trigger a new CMake configuration run
        "action-triggers-config"    : [
                                        "configure_project",
                                        "vscode_tasks",             # Needs target names (e.g. to know whether to build a coverage task)
                                        "vscode_properties"         # Needs to know compiler, paths
                                    ],
    }


def set_targets():
    global targets
    # Target types and regex to identify it. Targets are parsed in order (e.g. build is the remainder after parsing the others) 
    targets = OrderedDict({
                            "coverage"          : {
                                                    "re"    : "^coverage$",
                                                    "ids"   : [],
                                                },
                            "docs"              : {
                                                    "re"    : "^docs$",
                                                    "ids"   : [],
                                                },
                            "clang-tidy"        : {
                                                    "re"    : ".*_clangtidy$",
                                                    "ids"   : [],
                                                },
                            "test"              : {
                                                    "re"    : ".*_test_run$",
                                                    "ids"   : [],
                                                },
                            "install"           : {
                                                    "re"    : "^install.*",
                                                    "ids"   : [],
                                                },
                            "build"             : {
                                                    "re"    : f"^{global_params['project-name']}_.*",
                                                    "ids"   : [],
                                                },
                        })


def set_shell_commands():
    global shell_commands
    shell_commands = {
        "conan-profile-cmd"         : "conan profile detect --force",
        "conan-install-cmd"         : f"conan install {global_params['third-party-directory']} "
                                      f"-s build_type={global_params['project-build-type']} "
                                      f"--output-folder={global_params['conan-output-directory']} "
                                      f"--build missing "
                                      f"-s compiler.cppstd={cppstd}",

        "cmake-configure-cmd"       : f"cmake -S {global_params['base-directory']} "
                                      f"-B {global_params['build-directory']} "
                                      f'-G "Unix Makefiles" '
                                      f"-DCMAKE_BUILD_TYPE={global_params['project-build-type']} "
                                      f"-DBUILD_TESTS={is_on_str(global_params['project-build-tests'])} "
                                      f"--graphviz={global_params['dependency-graph-input']} "
                                      f"-DUSE_CONAN={global_params['project-use-conan']} ",

        "cmake-conan-add-cmd"       : f"-DCMAKE_TOOLCHAIN_FILE={global_params['conan-output-toolchain']} "
                                      f"-DCMAKE_POLICY_DEFAULT_CMP0091=NEW",                                # cmake-configure-cmd + cmake-conan-add-cmd

        "cmake-build-target-cmd"    : f"cmake --build {global_params['build-directory']} --target ",        # + <target_name>

        "cmake-build-default-cmd"   : f"cmake --build {global_params['build-directory']} -j{global_params['cmake-build-parallel-level']} ", # -j N for multicore build

        "cmake-docs-cmd"            : f"cmake --build {global_params['build-directory']} --target docs",

        "execute-app-cmd"           : os.path.join(
                                        global_params['binary-app-directory'], 
                                        global_params['project-name'] + global_params['binary-app-postfix']),

        "execute-tests-cmd"         : os.path.join(
                                        global_params['binary-test-directory'], 
                                        global_params['project-name'] + global_params['binary-test-postfix']),

        "clean-project-cmd"         : f"rm -rf \"{global_params['build-directory']}/\"* && "
                                      f"touch \"{os.path.join(global_params['build-directory'],'.gitkeep')}\"",

        "dependency-graph-cmd"      : f"dot -Tpng {global_params['dependency-graph-input']} "
                                      f"-o {global_params['dependency-graph-output']}",
    }


def get_parser():
    """ Create and return a commandline parser object. """
    parser = argparse.ArgumentParser(
        description="Development, Test and Tooling Actions. Command-line automation of actions related to CMake, Conan, clang-* tools, etc."
    )

    conanopt    = parser.add_argument_group(title="Conan package manager options")
    configopt   = parser.add_argument_group(title="Configure CMake project options")
    buildopt    = parser.add_argument_group(title="Build CMake project options")
    testopt     = parser.add_argument_group(title="Test project options")
    cleanopt    = parser.add_argument_group(title="Clean CMake project options")
    toolopt     = parser.add_argument_group(title="Project tooling options")
    docsopt     = parser.add_argument_group(title="Project documentation options")
    vscodeopt   = parser.add_argument_group(title="VSCode options")

    conanopt.add_argument('--conan-profile', 
                            action='store_true', 
                            help='Detect and infer system capabilities and compiler versions (run once for a fresh system)')

    conanopt.add_argument('--conan-install', 
                            action='store_true', 
                            help='Install third-party dependencies with Conan')

    conanopt.add_argument('--use-conan', 
                            action='store_true', 
                            help='Enable Conan as package manager for CMake configuration. --conan-install must have been run first')

    configopt.add_argument('--configure-project', 
                            action='store_true', 
                            help='(Re-)configure entire CMake project. Ex. --configure-project --debug --with-tests --conan-install')

    buildopt.add_argument('--build-target-list', 
                            action='store_true',
                            help='List available build targets')

    buildopt.add_argument('--build-run-project', 
                            action='store_true', 
                            help='(Re-)build project from current configuration, then run app')
    
    buildopt.add_argument('--debug', 
                            action='store_true', 
                            help='Force debug build')

    buildopt.add_argument('--release', 
                            action='store_true', 
                            help='Force release build')

    testopt.add_argument('--with-tests', 
                            action='store_true', 
                            help='Also build unit tests (without running)')

    testopt.add_argument('--build-test-project', 
                            action='store_true', 
                            help='Force reconfiguration and build project with unit tests, then run tests')

    testopt.add_argument('--test-target-list', 
                            action='store_true',
                            help='List available test targets')
    
    testopt.add_argument('--coverage-target-list', 
                            action='store_true',
                            help='List available coverage targets')
    
    testopt.add_argument('--run-coverage', 
                            action='store_true',
                            help='Run coverage analysis on current artifacts (no recompile)')

    cleanopt.add_argument('--clean-project', 
                            action='store_true', 
                            help='Clean build files (delete all)')

    toolopt.add_argument('--clang-tidy-target-list', 
                            action='store_true',
                            help='List available clang-tidy targets')

    toolopt.add_argument('--clang-tidy-all', 
                            action='store_true', 
                            help='Run clang-tidy on all clang-tidy targets')

    toolopt.add_argument('--clang-tidy-target', 
                            metavar='<target-name>', 
                            help='Run clang-tidy on specific clang-tidy target')

    toolopt.add_argument('--clang-format-all', 
                            action='store_true')

    toolopt.add_argument('--clang-format-target', 
                            metavar='<target-name>')

    docsopt.add_argument('--docs-target-list', 
                            action='store_true',
                            help='List available docs targets')

    docsopt.add_argument('--docs', 
                            action='store_true',
                            help='Compile Doxygen documentation')

    vscodeopt.add_argument('--vscode-launch', 
                            action='store_true',
                            help='Create VSCode launch.json file')
    
    vscodeopt.add_argument('--vscode-tasks', 
                            action='store_true',
                            help='Create VSCode tasks.json file')

    vscodeopt.add_argument('--vscode-properties', 
                            action='store_true',
                            help='Create VSCode c_cpp_properties.json file')

    return parser


def guard_all_required(args):
    """
    Always run this for options that trigger reconfiguration, in order to ensure that needed parameters and env are set.
    This step (possibly) sets environment variables for child processes. Will not change the parent/calling environment.
    Flags will override environment variables.
    """
    print("Resolving required parameters... These may be different from the current CMake configuration.")

    # Curry the function with the globals from this file
    guard = partial(guard_single, global_params=global_params, fallback=fallback)

    # PROJECT_NAME: Project name doesn't have any possible --options
    guard(env_var='PROJECT_NAME', param_name='project-name')

    # PROJECT_BUILD_TYPE: potentially via --debug or --release
    # Only override if more than zero of the flags are set
    override_build_type = any([args.debug, args.release])    
    if all([args.debug, args.release]) or not any([args.debug, args.release]) or args.debug:
        # User specified neither flag or both flags (XOR: Either both on or both off), or --debug.
        override_type = "Debug"
    else:
        # Must be --release
        override_type = "Release"

    guard(env_var='PROJECT_BUILD_TYPE',  param_name='project-build-type',  override_with_flag=override_build_type, override_value=override_type)

    # PROJECT_BUILD_TESTS: If invoking --build-test-project or --with-tests, then we want to enable tests in the build
    override_testing = any([args.build_test_project, args.with_tests]) 
    guard(env_var='PROJECT_BUILD_TESTS', param_name='project-build-tests', override_with_flag=override_testing, override_value="ON")

    # PROJECT_USE_CONAN If invoking --conan-install or --use-conan (i.e., conan deps must be installed separately), then Conan is inferred
    override_conan = any([args.conan_install, args.use_conan])
    guard(env_var='PROJECT_USE_CONAN', param_name='project-use-conan', override_with_flag=override_conan, override_value="ON")


def handle_args(args):
    """ 
    Handle commandline arguments: Non-mutually exclusive handling.
    The order is important:
    - If using external package manager: Resolve this first (can't configure CMake without it)
    - (Re-)configure project before building
    - Perform runs before other tooling/docs targets
    - Perform runs before cleaning
    """

    # *** Get some target info first, if needed ***
    if is_requires_targets(args) and is_project_configured():
        populate_targets()

    elif is_requires_targets(args) and not is_project_configured():
        print("Project not configured. Run --configure-project first, then any of the --*-list")


    # *** Conan ***
    if (args.conan_profile):
        os.system(shell_commands['conan-profile-cmd'])

    if (args.conan_install):
        if not is_conan_configured():
            # Run config for conan first to avoid fail
            os.system(shell_commands['conan-profile-cmd'])
        os.system(shell_commands['conan-install-cmd'])


    # *** Configure without building ***
    if (is_triggers_config(args)):

        cmd: str = shell_commands['cmake-configure-cmd'] 
        if global_params['project-use-conan'] == "ON":
            cmd += shell_commands['cmake-conan-add-cmd']
        
        print(f"CMake configuration call: {cmd}")
        os.system(cmd)


    # *** Output for *-list ***
    # TODO: Refactor header into global object
    if args.build_target_list and is_project_configured():
        pretty_print_targets(target_type='build', header="Build targets:")

    if args.test_target_list and is_project_configured():
        pretty_print_targets(target_type='test', header="Test targets:")

    if args.coverage_target_list and is_project_configured():
        pretty_print_targets(target_type='coverage', header="Coverage targets:")

    if args.clang_tidy_target_list and is_project_configured():
        pretty_print_targets(target_type='clang-tidy', header="Clang-tidy targets:")

    if args.docs_target_list and is_project_configured():
        pretty_print_targets(target_type='docs', header="Docs targets:")


    # *** Build targets ***
    if args.build_run_project and is_project_configured():
        os.system(shell_commands['cmake-build-default-cmd'])
        os.system(shell_commands['execute-app-cmd'])

    elif args.build_run_project and not is_project_configured():
        print("Project not configured. Run --configure-project first, then --build-run")


    # *** Testing ***
    if (args.build_test_project):
        os.system(shell_commands['cmake-build-default-cmd'])
        os.system(shell_commands['execute-tests-cmd'])

    if (args.run_coverage):
        # Verify that a "coverage" target exists... For now, obly expecting one cov target.
        # As this may be run together with the build step, re-populate targets
        populate_targets()

        if not is_known_target_of_type(target_type="coverage", target_id="coverage"):
            print("Project not configured for coverage. Check ENABLE_COVERAGE in CMakeLists.txt and further settings in test/CMakeLists.txt")
        else:
            # Run the coverage target
            run_single_target("coverage")
    

    # *** Clang-tidy runs ***
    if (args.clang_tidy_all):
        for t in get_known_targets_of_type('clang-tidy'):
            # Execute clang-tidy one by one
            run_single_target(t)

    if (args.clang_tidy_target):
        # Check if the requested target is indeed a possible target
        if (args.clang_tidy_target not in get_known_targets_of_type('clang-tidy')):
            print(f"{args.clang_tidy_target} is not a valid clang-tidy target (possible targets: {get_known_targets_of_type('clang-tidy')}).")
            return
        else:
            run_single_target(args.clang_tidy_target)

    if (args.docs):
        os.system(shell_commands['cmake-docs-cmd'])

    if (args.clean_project):
        os.system(shell_commands['clean-project-cmd'])

    # *** VSCode config runs ***
    if (args.vscode_launch):
        launch_json = make_vscode_launch_json()
        print("Copy to ../.vscode/launch.json:")
        print(launch_json)

    if (args.vscode_tasks):
        tasks_json = make_vscode_tasks_json()
        print("Copy to ../.vscode/tasks.json:")
        print(tasks_json)

    if (args.vscode_properties):
        properties_json = make_vscode_properties_json()
        print("Copy to ../.vscode/c_cpp_properties.json:")
        print(properties_json)


def make_vscode_launch_dict() -> VSCodeLaunch:
    """
    Creates Dictionary object parsable as launch.json by VSCode.
    """
    launch: VSCodeLaunch = {
        'version': '0.2.0',
        'configurations': [
            {
                'name': f"~> {global_params['project-name']} debug",
                'cwd': '${workspaceFolder}',
                'type': 'cppdbg',
                'request': 'launch',
                'program': shell_commands['execute-app-cmd'],
                'stopAtEntry': True,
                'environment': [],
                'externalConsole': False,
                'MIMode': 'gdb',
                'setupCommands': [
                    {
                        'description': 'Enable pretty-printing for gdb',
                        'text': '-enable-pretty-printing',
                        'ignoreFailures': True
                    }
                ],
                'preLaunchTask': f"~> Build {global_params['project-name']}",              # References the task with this name
                'miDebuggerPath': '/usr/bin/gdb'
            }
        ]

    }
    return launch


def make_vscode_launch_json():
    return json.dumps(make_vscode_launch_dict(), indent=4)


def make_vscode_tasks_dict() -> VSCodeTasks:
    """
    Creates Dictionary object parsable as tasks.json by VSCode.
    """
    config_cmd: str = shell_commands['cmake-configure-cmd'] + shell_commands['cmake-conan-add-cmd']
    config_cmd_args = config_cmd.split(' ')

    build_cmd: str = shell_commands['cmake-build-default-cmd']
    build_cmd_args = build_cmd.split(' ')

    options_obj: VSCodeTaskOption = {
        'cwd': global_params['build-directory']
    }

    configure_debug_build_task: VSCodeTask = {
        'type': 'shell',
        'label': f"~> Configure {global_params['project-name']} with debug symbols",
        'detail': 'Invoke CMake to configure project, required to build it at a later stage.',
        'command': '/usr/bin/cmake',
        'args': config_cmd_args[1:],                    # exclude cmake
        'options': options_obj
    }

    make_debug_build_task: VSCodeTask = {
        'type': 'shell',
        'label': f"~> Build {global_params['project-name']}",
        'detail': 'Invoke CMake to build project now.',
        'command': '/usr/bin/cmake',
        'args': build_cmd_args[1:],                     # exclude cmake
        'options': options_obj,
        'group': {
            'kind': 'build',
            'isDefault': True
        },
        'dependsOn': [configure_debug_build_task['label']]
    }

    run_test_task: VSCodeTask = {
        'type': 'shell',
        'label': f"~> Run tests for {global_params['project-name']}",
        'detail': 'Run all unit tests now.',
        'command': shell_commands['execute-tests-cmd'],
        'args': [],
        'options': options_obj,
        'dependsOn': [make_debug_build_task['label']]
    }

    # Only make coverage task if coverage is set up as a target in CMakeLists.txt (or --use-coverage)
    if use_coverage := is_known_target_of_type(target_type="coverage", target_id="coverage"):
        # cmake --build {global_params['build-directory']} --target coverage
        coverage_cmd: str = shell_commands['cmake-build-target-cmd'] + "coverage"
        coverage_cmd_args = coverage_cmd.split(' ')

        run_coverage_task: VSCodeTask = {
            'type': 'shell',
            'label': f"~> Run code coverage for {global_params['project-name']}",
            'detail': 'Analyze code coverage.',
            'command': '/usr/bin/cmake',
            'args': coverage_cmd_args[1:],
            'options': options_obj,
            'dependsOn': [make_debug_build_task['label']]
        }

    tasks: VSCodeTasks = {
        'version': "2.0.0",
        'tasks': [configure_debug_build_task, make_debug_build_task, run_test_task]
    }

    if use_coverage:
        tasks['tasks'].append(run_coverage_task)

    return tasks

def make_vscode_tasks_json():
    return json.dumps(make_vscode_tasks_dict(), indent=4)


def make_vscode_properties_dict() -> VSCodeProperties:
    """
    Creates Dictionary object parsable as c_cpp_properties.json by VSCode.
    """

    cc = CMakeCache(global_params['cmake-cache-file'])      # Load CMakeCache.txt
    conan_include_paths = get_conan_include_paths(global_params['conan-output-toolchain'])

    properties: VSCodeProperties = {
        'version': 4,
        'configurations': [
            {
                'name': 'Linux',
                'includePath': [
                    "${workspaceFolder}/**",
                    *conan_include_paths,
                    os.path.join(global_params['build-directory'], 'config', 'include'),
                    os.path.join(global_params['source-directory'], 'include')
                ],
                'defines': [],
                'compilerPath': cc['CMAKE_CXX_COMPILER'],
                'cStandard': 'c17',
                'cppStandard': f"c++{cppstd}",
                'intelliSenseMode': 'linux-gcc-x64',
                'compileCommands': global_params['cmake-compile-commands']
            }
        ]       
    }
    return properties


def make_vscode_properties_json():
    return json.dumps(make_vscode_properties_dict(), indent=4)


def is_project_configured() -> bool:
    """
    Check if CMake has already configured project (though project may be stale).
    Existence of cache file is proof of configuration.
    """
    return bool(os.path.isfile(global_params['cmake-cache-file']))


def is_conan_configured() -> bool:
    """
    Check if Conan has a default profile.
    """
    return bool(os.path.isfile(global_params['conan-profile-file']))


def is_any_from_group_set(args, in_group_list: List[str]) -> bool:
    """
    True if any arguments (i.e. commandline --options) from the group_list
    have been set somehow that can resolve to a boolean True.
    """
    is_single_arg_set = lambda arg: bool(getattr(args, arg))
    return any(map(is_single_arg_set, in_group_list))


def is_requires_targets(args) -> bool:
    """ Return true if any options have been set that will require CMake cache in order to be aware of target names (i.e. project already configured). """
    return is_any_from_group_set(args, global_params['action-requires-targets'])


def is_triggers_guard(args) -> bool:
    """ Return true if any options have been set that must trigger a run of the guard/resolution. """
    return is_any_from_group_set(args, global_params['action-triggers-guard'])


def is_triggers_config(args) -> bool:
    """ Return true if any options have been set that must trigger a CMake config. run. """
    return is_any_from_group_set(args, global_params['action-triggers-config'])


def get_cmake_target_list():
    """
    Query CMake to get all targets (as available in current CMake cache).

    Target types in the list are executable (binary executable), library (binary non-executable), custom (command line invokations), etc.
    
    Will not invoke new run of CMake configuration, so if project not configured yet (or files are deleted/ cleaned),
    this call will print 'Error: could not load cache' twice.
    """

    cmd_string = shell_commands['cmake-build-target-cmd'] + "help"      # <cmake bla bla> --target help
    cmd_list = list(cmd_string.split(' '))                              # subprocess.run() takes command as a list of component parts

    # Run command and extract output as a list of lines
    targets_raw = subprocess.run(cmd_list, stdout=subprocess.PIPE).stdout.decode('UTF-8').split(os.linesep)

    # Filter list to only get lines that are targets (Follows pattern '... <target_name>')
    is_target_filter = lambda line: True if line[:3] == '...' else False
    
    # Then Map transforms each remaining line by removing the '... ' prefix
    remove_dots_map = lambda line: line[4:]

    return list(map(remove_dots_map, filter(is_target_filter, targets_raw)))


def allocate_to_target_types(full_target_list: List[str]) -> None:
    """
    Allocates the full CMake target list (read from current CMakeCache.txt) into types in the global object.
    Types defined per ordered pattern matching (parsing) vs. global object regex. 
    Writes directly to the global targets obj.
    """
    cmake_target_list: List[str] = full_target_list.copy()

    # Iterate over raw list of CMake targets to tie each to a recognized target type
    while (cmake_target_list):
        cmake_target_id = cmake_target_list.pop()

        # Ordered/prioritized parsing for each target-type
        for target_type_params in targets.values():

            # Check if cmake_target_id matches the regex for this target-type
            if re.search(target_type_params['re'], cmake_target_id):
                target_type_params['ids'].append(cmake_target_id)      # Store this cmake target as a recognized type
                break                                                  # Done, succesfully assigned the id to a known type


def populate_targets() -> None:
    """ Updates the global targets dictionary """
    set_targets()                                       # Reset to default
    allocate_to_target_types(get_cmake_target_list())   # Load from CMake


def get_known_targets_of_type(target_type: str) -> List[str]:
    """ Returns the list of ids for a given target type (build, test, etc.). """
    return targets[target_type]['ids']


def is_known_target_of_type(target_type: str, target_id: str) -> bool:
    """ If target-id is in the list of target-ids for this target-type, then it is known. """
    return target_id in get_known_targets_of_type(target_type)


def is_known_target(target_id: str) -> bool:
    """ Checks if target_id can be found in list composed from targets of known types. """
    all_targets = []
    for target_type in targets.keys():
        all_targets += get_known_targets_of_type(target_type)
    return target_id in all_targets


def pretty_print_targets(target_type: str, header: str = "Targets:") -> None:
    """ Prints a list with N items as N lines. """
    print(header)
    [print(t) for t in get_known_targets_of_type(target_type)]


def run_single_target(target_id: str) -> Union[bool, int]:
    """ Order CMake to run/build a single target process declared as <target_name>. """

    # Don't try to execute if target name isn't known to CMake
    if not is_known_target(target_id):
        print("Target not in CMake cache. Check <target_name> or reconfigure CMake project.")
        return False

    cmd = shell_commands['cmake-build-target-cmd'] + target_id      # cmake --build <dir> --target <target_id>
    return os.system(cmd)


def run():
    """ Execute all command line actions """

    # Parse commandline
    p = get_parser()
    args = p.parse_args()

    # Initialize parameters
    set_global_params()

    # Guard and resolve parameters
    if is_triggers_guard(args):
        guard_all_required(args)

    # Compute from global parameters
    # set_targets()
    set_shell_commands()

    # Execute orders
    handle_args(args)


if __name__ == '__main__':
    run()
