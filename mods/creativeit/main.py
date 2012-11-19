#import all client and server scripts
import common
import common.menu_utils
import creativeit.agent
import creativeit.client as client
import creativeit.constants as constants
import creativeit.module as module
import OpenNero

def ModMain():
    client.ClientMain()

script_server = None

#def ModTick(dt):
#    global script_server
#    if OpenNero.getAppConfig().rendertype == 'null':
#        return
#    if script_server is None:
#        script_server = common.menu_utils.GetScriptServer()
#        common.startScript("creativeit/menu.py")
#    data = script_server.read_data()
#    while data:
#        module.parseInput(data.strip())
#        data = script_server.read_data()
