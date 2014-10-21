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
SPECIAL_MOD_NAMES = ['TEMPLATE', 'common', 'hub', 'share']
SPECIAL_MOD_PREFIX = '_'
MOD_PATH = '.'
SHARE_PATH = 'share'
BUILTIN_MOD_IDS = ['Maze:Search', 'Maze:RL', 'TowerofHanoi:Planning', 'TowerofHanoi:NLP', 'Roomba', 'NERO', 'NERO_Battle']
MOD_TITLES = {'Maze:Search': 'Search',
              'Maze:RL': 'Reinforcement Learning',
              'TowerofHanoi:Planning': 'Planning',
              'TowerofHanoi:NLP': 'Natural Language Processing',
              'Roomba': 'Neuroevolution',
              'NERO': 'Multi-agent Systems Training',
              'NERO_Battle': 'Multi-agent Systems Battle'}

MOD_DEPS = {'Hw1':'Maze', 'Hw2':'TowerofHanoi', 'Hw3':'Maze', 'Hw4':'TowerofHanoi', 'Hw5':'NERO'}

#MOD_IDS_OF_TITLES = dict((v, k) for k, v in MOD_TITLES.iteritems())

def extract_mod_name(mod_id):
    " Get the real mod name from a name:mode string"
    return mod_id.split(":")[0]

def extract_mod_mode(mod_id):
    " Get the mode part from a name:mode string"
    if ":" in mod_id:
        return mod_id.split(":")[1]
    else:
        return ""

def compile_mod_path(mod_name):
    " compile a list of all paths the mod uses, looking at dependencies and share folders "
    # create the path where to look for mod content
    mod_path = mod_name
    # find and append any bases that look like this mod
    for share_name in list_shares():
        if mod_name.startswith(share_name):
            mod_path += ':' + os.path.join(SHARE_PATH, share_name)
    # look in the dependency list to see if this mod has dependencies 
    if mod_name in MOD_DEPS:
        sub_path = compile_mod_path(MOD_DEPS[mod_name])
        mod_path += ':' + sub_path
    # always append the common directory
    if ":common" not in mod_path:
        mod_path += ":common"
    return mod_path

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
    global BUILTIN_MOD_IDS
    " list all available mods "
    mod_ids = []
    mod_names = []
    # first list Maze, NERO, TowerofHanoi and Roomba in order
    for mod_id in BUILTIN_MOD_IDS:
        mod_name = extract_mod_name(mod_id)
        if mod_exists(mod_name) and not is_special(mod_name):
            mod_ids.append(mod_id)
            mod_names.append(mod_name)
    # now add any other mods
    #BUILTIN_MOD_IDS = set(BUILTIN_MOD_IDS)
    user_mods = []
    files = sorted(os.listdir(MOD_PATH), reverse=True)
    for f in files:
        if f not in mod_names and mod_exists(f) and not is_special(f):
            user_mods.append(f)
    return user_mods + mod_ids

def list_bases():
    " list all available bases (directories starting with an underscore) "
    bases = []
    files = os.listdir(MOD_PATH)
    bases = [f for f in files if mod_is_base(f)]
    return bases

def list_shares():
    " list all available shares (directories inside FOLDER_SHARE) "
    shares = []
    files = os.listdir(SHARE_PATH)
    shares = [f for f in files]
    return shares 

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
