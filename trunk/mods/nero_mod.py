#!/usr/bin/env python
__help__ = """
OpenNERO mod manipulation routines.

These routines can be used both from command line and from within OpenNERO.
A mod is basically a directory with Python scripts and associated resources that "take over"
OpenNERO. For example, a mod can be a menu, a game, or an AI experiment.

Usage: nero_mod.py COMMAND ARGS

Where the parameters can be
  create MODNAME \t copy the special mod TEMPLATE into a new mod MODNAME and change internal references accordingly. This is equivalent to "copy TEMPLATE MODNAME".
  delete MODNAME \t delete the mod MODNAME
  copy FROM TO \t\t copy the mod named FROM to the one named TO, updating internal references as necessary
  move FROM TO \t\t copy the mod named FROM to the one named TO and then delete the source mod
"""

import shutil
import os
import sys

TEMPLATE = 'TEMPLATE'
SPECIAL_MOD_NAMES = ['TEMPLATE', 'common', 'hub']
SPECIAL_MOD_PREFIX = '_'
MOD_PATH = '.'

def mod_path(name):
    " get the path of the mod "
    return os.path.join(MOD_PATH, name)

def is_special(name):
    " True if the mod name is reserved "
    return name in SPECIAL_MOD_NAMES or name.startswith(SPECIAL_MOD_PREFIX)

def mod_exists(name):
    " True iff the mod exists "
    return os.path.exists(name) and os.path.isdir(name)

def mod_is_base(name):
    " True iff the mod is actually a mod base "
    return os.path.exists(name) and os.path.isdir(name) and name.startswith(SPECIAL_MOD_PREFIX)

def create_mod(name):
    if is_special(name):
        print "mod name '%s' is reserved - cannot create mod with this name" % name
        return False
    if mod_exists(name):
        print "mod '%s' already exists - cannot create mod with this name" % name
        return False
    print "creating mod '%s'" % name
    shutil.copytree(TEMPLATE, name)
    return True

def delete_mod(name):
    if is_special(name):
        print "cannot delete '%s' - reserved name" % name
        return False
    if not mod_exists(name):
        print "mod '%s' does not exist or is not a mod" % name
        return False
    print "deleting mod '%s'" % name
    shutil.rmtree(name)
    return True

def copy_mod(name, new_name):
    if name == new_name:
        print "request to copy '%s' to '%s' - nothing to do" % (name, new_name)
        return False
    if not mod_exists(name):
        print "cannot copy mod '%s' - mod does not exist" % name
        return False
    if is_special(new_name):
        print "mod name '%s' is reserved - cannot create mod with this name" % new_name
        return False
    if mod_exists(new_name):
        print "mod '%s' already exists - cannot create mod with this name" % new_name
        return False
    print "copying mod '%s' to '%s'" % (name, new_name)
    shutil.copytree(name, new_name)
    return True

def list_mods():
    " list all available mods "
    mods = []
    files = os.listdir(MOD_PATH)
    for f in files:
        if mod_exists(f) and not is_special(f):
            mods.append(f)
    return mods

def list_bases():
    " list all available bases (directories starting with an underscore) "
    bases = []
    files = os.listdir(MOD_PATH)
    bases = [f for f in files if mod_is_base(f)]
    return bases

def move_mod(name, new_name):
    return copy_mod(name, new_name) and delete_mod(name)

def error(msg):
    print >> sys.stderr, msg
    print >> sys.stderr, __help__
    sys.exit(1)

def main():
    if len(sys.argv) <= 1:
        error("Not enough arguments")
    command = sys.argv[1]
    if command == "copy":
        if len(sys.argv) != 4:
            error("Wrong number of arguments")
        copy_mod(sys.argv[2], sys.argv[3])
    elif command == "create":
        if len(sys.argv) != 3:
            error("Wrong number of arguments")
        create_mod(sys.argv[2])
    elif command == "delete":
        if len(sys.argv) != 3:
            error("Wrong number of arguments")
        delete_mod(sys.argv[2])
    elif command == "move":
        if len(sys.argv) != 4:
            error("Wrong number of arguments")
        move_mod(sys.argv[2], sys.argv[3])
    else:
        error("Unknown command '%s'" % command)

if __name__ == "__main__":
    main()
