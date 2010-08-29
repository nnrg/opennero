import sys

# OpenNero imports
from OpenNero import getSimContext, Vector3f

def getGuiManager():
    return getSimContext().getGuiManager()

def addObject(templateFile, position, rotation = Vector3f(0,0,0), scale = Vector3f(1,1,1), label = "", type = 0):
    return getSimContext().addObject(templateFile, position, rotation, scale, label, type)

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
