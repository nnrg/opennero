#import all client and server scripts
import NERO.module
import NERO.client
import NERO.agent
import common
import common.menu_utils
import OpenNero

def ModMain():
    NERO.client.ClientMain()

script_server = None

def ModTick(dt):
    global script_server
    # don't start the external menu if we are running in headless mode!
    if OpenNero.getAppConfig().rendertype == 'null':
        return
    if script_server is None:
        script_server = common.menu_utils.GetScriptServer()
        common.startScript("NERO/menu.py")
    data = script_server.read_data()
    while data:
        NERO.module.parseInput(data.strip())
        data = script_server.read_data()

def StartEvolving():
    mod = NERO.module.getMod()
    mod.deploy('rtneat')
