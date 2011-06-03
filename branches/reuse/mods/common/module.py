import sys
import subprocess
from menu_utils import GetScriptServer
from math import *

# OpenNero imports
from OpenNero import getSimContext, Vector3f

def getGuiManager():
    return getSimContext().getGuiManager()

def addObject(templateFile, position, rotation = Vector3f(0,0,0), scale = Vector3f(1,1,1), label = "", type = 0, collision = 0):
    return getSimContext().addObject(templateFile, position, rotation, scale, label, collision, type)

def removeObject(ID):
    """ remove an object from the simulation """
    getSimContext().removeObject(ID)

def addSkyBox(prefix, extension = None):
    """ add a skybox starting with the prefix and ending with the extension """
    if extension:
        getSimContext().addSkyBox(prefix, extension)
    else:
        getSimContext().addSkyBox(prefix)
    
def openWiki(page):
    import webbrowser
    def closure():
        print 'openWiki:', page
        webbrowser.open('http://code.google.com/p/opennero/wiki/' + page)
    return closure

def wrap_degrees(a, da):
    """ add da to a, but make sure that the resulting angle stays within the -180 to 180 range """
    a2 = a + da
    if a2 > 180:
        a2 = -180 + (a2 % 180)
    elif a2 < -180:
        a2 = 180 - (abs(a2) % 180)
    return a2

def mix_angles(h1, h2, f):
    """
    Combine two angles carefully
    @return the angle that is a mixture of h1 and h2
    @param h1 first angle in degrees
    @param h2 second angle in degrees
    @param f the mixing constant - 0 is 100% h1 and 1 is 100% h2
    """
    # combine headings carefully to avoid a 'flip'
    rh1,rh2 = radians(h1), radians(h2)
    # average direction of the two headings
    hy = sin(rh1) * (1-f) + sin(rh2) * f
    hx = cos(rh1) * (1-f) + cos(rh2) * f
    # angle of that direction in degrees
    return degrees(atan2(hy, hx))

opennero_sub_procs = {}

def startScript(script):
    """
    start the named script unless it is already running
    """
    global opennero_sub_procs    
    if script not in opennero_sub_procs:
        subproc = subprocess.Popen(['python', script])
        opennero_sub_procs[script] = subproc
    elif opennero_sub_procs[script].poll():
        del opennero_sub_procs[script]
        subproc = subprocess.Popen(['python', script])
        opennero_sub_procs[script] = subproc

def killScript(script):
    """
    If the named script was started, kill it and cleanup the handle
    """
    global opennero_sub_procs
    if script in opennero_sub_procs:
        subproc = opennero_sub_procs[script]
        del opennero_sub_procs[script]
        subproc.kill()
