#import all client and server scripts
import NERO.module as module
import NERO.client
import NERO.agent
import NERO.constants as constants
import common
import common.menu_utils
import OpenNero

def ModMain(mode = ""):
    NERO.client.ClientMain()

def ModTick(dt):
    # don't start the external menu if we are running in headless mode!
    if OpenNero.getAppConfig().rendertype == 'null':
        return
    script_server = module.getServer()
    data = script_server.read_data()
    while data:
        module.parseInput(data.strip())
        #script_server.write_data(data)
        data = script_server.read_data()

def StartEvolving():
    mod = NERO.module.getMod()
    mod.deploy('rtneat')
