from OpenNero import * #imports vector3f
from common import * #imports addObject
from CaptureTheFlag.NeroEnvironment import *
from CaptureTheFlag.RTNEATAgent import *
import common.gui as gui
from CaptureTheFlag.inputConfig import *
from CaptureTheFlag.constants import *
from math import *
from CaptureTheFlag.module import getMod, delMod
from common.fitness import Fitness
from common import *
import time
from CaptureTheFlag.client import *

#for sydney
#from CaptureTheFlag.environment import *
#from CaptureTheFlag.agent import *

def ModMain():
	print "Hello, Mod!"
	
	#initialize camera
	camRotateSpeed = 10
    	camMoveSpeed = 500
    	camZoomSpeed = 50
    	cam = getSimContext().addCamera(camRotateSpeed, camMoveSpeed, camZoomSpeed)
   	cam.setFarPlane(5000)
    	cam.setEdgeScroll(False)
    	cam.setPosition(Vector3f(5, 5, 5))
    	cam.setTarget(Vector3f(0, 0, 1))
	
	
	
	#create environment for the agent
	#env = NeroEnvironment()
	#set_environment(env)
	
	#for hand driving sydney for debugging purposes
	env = NeroEnvironment()
	set_environment(env)
	#enable_ai()
	#addObject("data/character/SydneyAgent.xml", Vector3f(0, 0, 4.5))
    
    	#enable_collision()
	
	
	#startScript('Nero/menu.py')

	# create key binding to control the agent
	ioMap = createInputMapping()
	getSimContext().setInputMapping(ioMap)

	# enable AI and create the agent
	
	#add terrain at the given X-Y-Z coordinates
	addObject("data/terrain/GrassyTerrain.xml", Vector3f(-50,-50,0))
	addObject("data/terrain/GrassyTerrain.xml", Vector3f(-50,-143,0))
	
	#add the left and right walls
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(-50, -49.5, 0), Vector3f(0, 0, 90), Vector3f(9.4, 1, 1), type = OBSTACLE)
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(44, -49.5, 0), Vector3f(0, 0, 90), Vector3f(9.4, 1, 1), type = OBSTACLE)
	
	#add the top and bottom walls
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(-3, -143, 0), Vector3f(0, 0, 0), Vector3f(4.7, 1, 1), type = OBSTACLE)
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(-3, 44, 0), Vector3f(0, 0, 0), Vector3f(4.7, 1, 1), type = OBSTACLE)
	
	#add the right side square
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(-29, -59.5, 0), Vector3f(0, 0, 0), Vector3f(2.15, 1, 1), type = OBSTACLE)
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(-29, -39.5, 0), Vector3f(0, 0, 0), Vector3f(2.15, 1, 1), type = OBSTACLE)
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(2, -49.5, 0), Vector3f(0, 0, 90), Vector3f(1.05, 1, 1), type = OBSTACLE)
	
	#add the left side square
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(23, -59.5, 0), Vector3f(0, 0, 0), Vector3f(2.15, 1, 1), type = OBSTACLE)
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(23, -39.5, 0), Vector3f(0, 0, 0), Vector3f(2.15, 1, 1), type = OBSTACLE)
	addObject("data/shapes/wall/BrickWall.xml", Vector3f(-8, -49.5, 0), Vector3f(0, 0, 90), Vector3f(1.05, 1, 1), type = OBSTACLE)
	
	#add a cube in center
	addObject("data/shapes/cube/GreenCube.xml", Vector3f(-3, -49.5, 0))
	
	#add a cube on blue teams side
	addObject("data/shapes/cube/BlueCube.xml", Vector3f(-3, -135, 0))
	
	#add a cube on red teams side
	addObject("data/shapes/cube/RedCube.xml", Vector3f(-3, 36, 0))
	
	#add sky and background
	addSkyBox("data/sky/irrlicht2")
	addObject("data/terrain/Sea.xml", Vector3f(-3000,-3000,-20))
	
	# setup the gui
    	CreateGui(getGuiManager())
		
