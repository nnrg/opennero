import os
import re
import sys
import gzip
import random
import tempfile
import xml.etree.ElementTree as ET
import json

import common
import constants
import environment
import OpenNero

from team import TeamEncoder

class NeroModule:
    def __init__(self):
        self.set_speedup(constants.DEFAULT_SPEEDUP)
        self.environment = None

    def set_speedup(self, speedup):
        OpenNero.getSimContext().delay = 1.0 - (speedup / 100.0)
        print 'speedup delay', OpenNero.getSimContext().delay

    def setup_map(self):
        """
        setup the test environment
        """
        OpenNero.disable_ai()

        if self.environment:
            error("Environment already created")
            return

        # create the environment - this also creates the rtNEAT object
        self.environment = environment.NeroEnvironment()
        OpenNero.set_environment(self.environment)
        self.environment.setup()

        return True

    def remove_flag(self):
        if self.environment:
            self.environment.remove_flag()

    def change_flag(self, loc):
        if self.environment:
            self.environment.change_flag(loc)

    def place_basic_turret(self, loc):
        if self.environment:
            self.environment.place_basic_turret(loc)

    def set_weight(self, key, value):
        self.environment.set_weight(key, value)

    #The following is run when one of the Deploy buttons is pressed
    def deploy(self, team_ai='rtneat', agent_ai='neat', team_type=constants.OBJECT_TYPE_TEAM_0):
        if self.environment:
            self.environment.deploy(team_ai, agent_ai, team_type)

    #The following is run when the Save button is pressed
    def save_team(self, location, team_type=constants.OBJECT_TYPE_TEAM_0):
        if not self.environment:
            return
        team = self.environment.teams[team_type]
        if not team:
            return
        with open(location, 'w') as f:
            json.dump(team, f, cls=TeamEncoder, indent=4, separators=(',', ': '))

    #The following is run when the Load button is pressed
    def load_team(self, location, team_type=constants.OBJECT_TYPE_TEAM_0):
        # OpenNero.disable_ai()
        # team = self.teams[team_type]
        # if team:
        #     team.load(location)
        # OpenNero.enable_ai()
        pass

    def set_spawn(self, x, y, team_type=constants.OBJECT_TYPE_TEAM_0):
        if self.environment:
            self.environment.set_spawn(x, y)


gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = NeroModule()
    return gMod

script_server = None

def getServer():
    global script_server
    if script_server is None:
        script_server = common.menu_utils.GetScriptServer()
        common.startJava(constants.MENU_JAR, constants.MENU_CLASS)
    return script_server

def parseInput(strn):
    """
    Parse input from the remote control training interface
    """
    mod = getMod()
    root = ET.fromstring(strn)

    if root.tag == 'message':
        for content in root:
            msg_type = content.attrib['class'].split('.')[-1]
            print 'Message received:', msg_type

            if msg_type == 'FitnessWeights':
                parseInputFitness(content)
                
            if msg_type == 'Command':
                parseInputCommand(content)

def parseInputFitness(content):
    """
    Parse fitness/reward related input from training window
    """
    mod = getMod()
    for entry in content.findall('entry'):
        dim, val = entry.attrib['dimension'], float(entry.text)
        key = getattr(constants, 'FITNESS_' + dim, None)
        if key:
            mod.set_weight(key, val / 100.0)
    
def parseInputCommand(content):
    """
    Parse commands from training window
    """
    mod = getMod()
    command, arg = content.attrib['command'], content.attrib['arg']
    # first word is command rest is filename
    if command.isupper():
        vali = int(arg)
    if command == "save1": mod.save_team(arg, constants.OBJECT_TYPE_TEAM_0)
    if command == "load1": mod.load_team(arg, constants.OBJECT_TYPE_TEAM_0)
    if command == "rtneat": mod.deploy('rtneat', 'neat')
    if command == "qlearning": mod.deploy('none', 'qlearning')
    if command == "pause": OpenNero.disable_ai()
    if command == "resume": OpenNero.enable_ai()

def ServerMain():
    print "Starting mod NERO"
