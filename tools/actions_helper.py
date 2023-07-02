"""
Various helpers for the actions.py script
Janus, 2023
"""

import os
from typing import Dict, List, Optional, TypedDict

class VSCodeLaunchSetupCommand(TypedDict):
    """Defines the object(s) that go into the configurations[setupCommands] sublist in a launch.json file."""
    description: str
    text: str
    ignoreFailures: bool


class VSCodeLaunchConfiguration(TypedDict):
    """Defines the object(s) that go into the configurations list in a launch.json file."""
    name: str
    cwd: str
    type: str
    request: str
    program: str
    args: List[str]
    stopAtEntry: bool
    environment: List[str]
    externalConsole: bool
    MIMode: str
    setupCommands: List[VSCodeLaunchSetupCommand]
    preLaunchTask: str
    miDebuggerPath: str


class VSCodeLaunch(TypedDict):
    """Defines the launch.json object structure."""
    version: str
    configurations: List[VSCodeLaunchConfiguration]


class VSCodeTaskGroup(TypedDict):
    """Defines the object that goes inside the group attribute in a task in tasks.json file."""
    kind: str
    isDefault: bool


class VSCodeTaskOption(TypedDict):
    """Defines the object that goes inside the options attribute in a task in a tasks.json file."""
    cwd: str


class VSCodeTask(TypedDict):
    """Defines the task object that goes inside the tasks list in a tasks.json file."""
    type: str
    label: str
    detail: str
    command: str
    args: List[str]
    options: VSCodeTaskOption 
    group: VSCodeTaskGroup
    dependsOn: List[str]


class VSCodeTasks(TypedDict):
    """Defines the tasks.json object structure."""
    version: int
    tasks: List[VSCodeTask]


class VSCodePropertyConfiguration(TypedDict):
    """Defines a named configuration that goes inside the configurations list in the c_cpp_properties.json file."""
    name: str
    includePath: List[str]
    defines: List[str]
    compilerPath: str
    cStandard: str
    cppStandard: str
    intelliSenseMode: str
    compileCommands: str


class VSCodeProperties(TypedDict):
    """Defines the property object for C/C++ in the c_cpp_properties.json file."""
    version: str
    configurations: List[VSCodePropertyConfiguration]


def is_on_str(text: str) -> str:
    """ "ON" if string is some "true", "TRUE", "True", "on", "ON", "On". Otherwise "OFF" """
    if text is None:
        return "OFF"
    return "ON" if text.upper() in ["TRUE", "ON"] else "OFF"


def guard_single(global_params, fallback, env_var: str, param_name: str, override_with_flag: bool = False, override_value: str = None, verbose: bool = True):
    """
    Check that env_var has a valid value. 
    If not, set it to the global fallback value, and keep global_params sync'ed.
    Key name in global_params and fallback dicts must match.
    Verbose: print what is being done. 
    """
    
    if verbose:
        old_value = os.getenv(env_var)

    # If overriding
    if override_with_flag:

        # Override and sync
        global_params[param_name]   = override_value
        os.environ[env_var]         = override_value

    # Elif missing (None) or empty ("")
    elif not os.getenv(env_var):

        # Fix and sync
        global_params[param_name]   = fallback[param_name]
        os.environ[env_var]         = fallback[param_name]
        
    if verbose:
        print(f"Using value {os.environ[env_var]} for ENV:{env_var} (was {old_value}) and global_params['{param_name}'].")
