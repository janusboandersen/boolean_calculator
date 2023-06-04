"""
Various helpers for the actions.py script
Janus, 2023
"""

import os

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
