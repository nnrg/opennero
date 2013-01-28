# load standard libraries #
import sys
import random
import math

# load C-side code #
import OpenNero

# load Python functional scripts #
import constants
import common
import world_handler
from agent_handler import AgentState, AgentInit

# load agent script
from roomba import RoombaBrain
from RTNEATAgent import RTNEATAgent

class SandboxMod:

    def __init__(self):
        """
        initialize the sandbox server
        """
        self.marker_map = {} # a map of cells and markers so that we don't have more than one per cell
        self.environment = None
        self.agent_ids = []
    
    def mark(self, x, y, marker):
        """ Mark a position (x, y) with the specified color """
        # remove the previous object, if necessary
        self.unmark(x, y)
        # add a new marker object
        id = common.addObject(marker, OpenNero.Vector3f(x, y, -1), OpenNero.Vector3f(0,0,0), OpenNero.Vector3f(0.5,0.5,0.5), type = constants.OBJECT_TYPE_MARKER)
        # remember the ID of the object we are about to create
        self.marker_map[(x, y)] = id
	    
    def mark_blue(self, x, y):
        self.mark(x, y,"data/shapes/cube/BlueCube.xml")
    
    def mark_green(self, x, y):
        self.mark(x, y,"data/shapes/cube/GreenCube.xml")
    
    def mark_yellow(self, x, y):
        self.mark(x, y,"data/shapes/cube/YellowCube.xml")
    
    def mark_white(self, x, y):
        self.mark(x, y,"data/shapes/cube/WhiteCube.xml")
    
    def unmark(self, x, y):
        if (x, y) in self.marker_map:
            common.removeObject(self.marker_map[(x, y)])
            del self.marker_map[(x, y)]
            return True
        else:
            return False
    
    def setup_sandbox(self):
        """
        setup the sandbox environment
        """
        OpenNero.getSimContext().delay = 0.0
        self.environment = RoombaEnvironment(constants.XDIM, constants.YDIM)
        OpenNero.set_environment(self.environment)
    
    def reset_sandbox(self=None):
        """
        reset the sandbox and refill with stuff to vacuum
        """
        for id in self.marker_map.values():
            common.removeObject(id)  # delete id from Registry, not from dict
        self.marker_map = {}
        for id in self.agent_ids:
            common.removeObject(id)  # delete id from Registry, not from list
        self.agent_ids = []
        OpenNero.reset_ai()

    def remove_bots(self):
        """ remove all existing bots from the environment """
        OpenNero.disable_ai()
        for id in self.agent_ids:
            common.removeObject(id)  # delete id from Registry, not from list
        self.agent_ids = []

    def distribute_bots(self, num_bots, bot_type):
        """distribute bots so that they don't overlap"""
        # make a number of tiles to stick bots in
        N_TILES = 10
        tiles = [ (r,c) for r in range(N_TILES) for c in range(N_TILES)]
        random.shuffle(tiles)
        bots_to_add = num_bots
        while bots_to_add > 0:
            (r,c) = tiles.pop() # random tile
            x, y = r * constants.XDIM / float(N_TILES), c * constants.YDIM / float(N_TILES) # position within tile
            x, y = x + random.random() * constants.XDIM * 0.5 / N_TILES, y + random.random() * constants.YDIM * 0.5 / N_TILES # random offset
            if in_bounds(x,y):
                agent_id = common.addObject(bot_type, OpenNero.Vector3f(x, y, 0), scale=OpenNero.Vector3f(1, 1, 1), type = constants.OBJECT_TYPE_ROOMBA, collision = constants.OBJECT_TYPE_ROOMBA)
                self.agent_ids.append(agent_id)
                bots_to_add -= 1
            else:
                pass # if a tile caused a bad starting point, we won't see it again
        

    def add_bots(self, bot_type, num_bots):
        OpenNero.disable_ai()
        num_bots = int(num_bots)
        if bot_type.lower().find("script") >= 0:
            self.distribute_bots(num_bots, "data/shapes/roomba/Roomba.xml")
            OpenNero.enable_ai()
            return True
        elif bot_type.lower().find("rtneat") >= 0:
            self.start_rtneat(num_bots)
            return True
        else:
            return False

    def start_rtneat(self, pop_size):
        " start the rtneat learning demo "
        OpenNero.disable_ai()
        #self.environment = RoombaEnvironment(constants.XDIM, constants.YDIM, self)
        #set_environment(self.environment)
        #self.reset_sandbox()
        # Create RTNEAT object
        rbound = OpenNero.FeatureVectorInfo()
        rbound.add_continuous(-sys.float_info.max, sys.float_info.max)
        rtneat = OpenNero.RTNEAT("data/ai/neat-params.dat", 7, 2, pop_size, 1.0, rbound)
        rtneat.set_weight(0,1)
        OpenNero.set_ai("rtneat",rtneat)
        OpenNero.enable_ai()
        self.distribute_bots(pop_size, "data/shapes/roomba/RoombaRTNEAT.xml")

def furniture_collide_all(x,y):
    """
    this is a manual list of collisions with furniture in the room
    TODO: replace with actual collision
    """
    for (cx,cy) in constants.CHAIR_LIST:
        if chair_collide(x,y,cx,cy):
            return True
    for (fx,fy) in constants.FURNITURE_LIST:
        if furniture_collide(x,y,fx,fy):
            return True
    return False
    
def chair_collide(x,y,cx,cy):
    (cx,cy) = furniture(cx,cy)
    return math.hypot(x-cx,y-cy) < constants.ROOMBA_RAD*2

def furniture_collide(x,y,fx,fy):
    (fx,fy) = furniture(fx,fy)
    return math.hypot(x-fx, y-fy) < constants.ROOMBA_RAD

def furniture(x,y):
    """
    convert from furniture space to roomba space
    """
    return (constants.XDIM * x, constants.YDIM * (1-y))    
    

def in_bounds(x,y):
    return (x > constants.ROOMBA_RAD and
            x < constants.XDIM - constants.ROOMBA_RAD and
            y > constants.ROOMBA_RAD and
            y < constants.YDIM - constants.ROOMBA_RAD and
            (x > constants.XDIM * 0.174 or y < constants.YDIM * (1.0 - 0.309)))

#################################################################################        
class RoombaEnvironment(OpenNero.Environment):
    """
    Sample Environment for the Sandbox
    """
    def __init__(self, xdim, ydim):
        """
        Create the environment
        """
        OpenNero.Environment.__init__(self) 
        
        self.XDIM = xdim
        self.YDIM = ydim
        self.max_steps = 500       
        self.states = {} # dictionary of agent states
        self.crumbs = world_handler.pattern_cluster(500, "Roomba/world_config.txt")
        # only keep crumbs that are inside the walls
        self.crumbs = [c for c in self.crumbs if in_bounds(c.x,c.y)]

        self.init_list = AgentInit()
        self.init_list.add_type("<class 'Roomba.roomba.RoombaBrain'>")
        self.init_list.add_type("<class 'Roomba.RTNEATAgent.RTNEATAgent'>")
        #print self.init_list.types

        roomba_abound = self.init_list.get_action("<class 'Roomba.roomba.RoombaBrain'>")
        roomba_sbound = self.init_list.get_sensor("<class 'Roomba.roomba.RoombaBrain'>")
        roomba_rbound = self.init_list.get_reward("<class 'Roomba.roomba.RoombaBrain'>")
        rtneat_abound = self.init_list.get_action("<class 'Roomba.RTNEATAgent.RTNEATAgent'>")
        rtneat_sbound = self.init_list.get_sensor("<class 'Roomba.RTNEATAgent.RTNEATAgent'>")
        rtneat_rbound = self.init_list.get_reward("<class 'Roomba.RTNEATAgent.RTNEATAgent'>")

        ### Bounds for Roomba ###
        # actions
        roomba_abound.add_continuous(-math.pi, math.pi) # amount to turn by
        
        # sensors
        roomba_sbound.add_discrete(0,1)    # wall bump
        roomba_sbound.add_continuous(0,xdim)   # self.x
        roomba_sbound.add_continuous(0,ydim)   # self.y
        roomba_sbound.add_continuous(0,xdim)   # closest.x
        roomba_sbound.add_continuous(0,ydim)   # closest.y
        
        # rewards
        roomba_rbound.add_continuous(-100,100) # range for reward

        ### End Bounds for Roomba ####

        ### Bounds for RTNEAT ###
        # actions
        rtneat_abound.add_continuous(-math.pi, math.pi) # amount to turn by
        
        # sensors
        rtneat_sbound.add_continuous(-1, 1)
        rtneat_sbound.add_continuous(-1, 1)
        rtneat_sbound.add_continuous(-1, 1)
        rtneat_sbound.add_continuous(-1, 1)
        rtneat_sbound.add_continuous(-1, 1)
        rtneat_sbound.add_continuous(-1, 1)
    
        # rewards
        rtneat_rbound.add_continuous(-1, 1)
        ### End Bounds for RTNEAT ###

        # set up shop
        # Add Wayne's Roomba room with experimentally-derived vertical offset to match crumbs.
        common.addObject("data/terrain/RoombaRoom.xml", OpenNero.Vector3f(xdim/2,ydim/2, -1), OpenNero.Vector3f(0,0,0), OpenNero.Vector3f(xdim/245.0, ydim/245.0, constants.HEIGHT/24.5), type = constants.OBJECT_TYPE_WALLS)

        # OpenNero.getSimContext().addAxes()
        self.add_crumbs()
        for crumb in self.crumbs:
            self.add_crumb_sensors(roomba_sbound)        

    def get_state(self, agent):
        if agent in self.states:
            return self.states[agent]
        else:
            print "new state created"
            pos = agent.state.position
            rot = agent.state.rotation
            self.states[agent] = AgentState(pos, rot)
            return self.states[agent]
        
    def randomize(self):
        self.crumbs = world_handler.read_pellets()
        # only keep crumbs that are inside the walls        
        self.crumbs = [c for c in self.crumbs if in_bounds(c.x,c.y)]

    def add_crumb_sensors(self, roomba_sbound):
        """Add the crumb sensors, in order: x position of crumb (0 to XDIM,
        continuous), y position of crumb (0 to XDIM, continuous), whether
        crumb is present at said position or has been vacced (0 or 1), and
        reward for vaccing said crumb."""
        roomba_sbound.add_continuous(0, self.XDIM)    # crumb position X
        roomba_sbound.add_continuous(0, self.YDIM)    # crumb position Y
        roomba_sbound.add_discrete(0, 1)       # crumb present/ not present
        roomba_sbound.add_discrete(1, 5)       # reward for crumb

    def add_crumbs(self):
        for pellet in self.crumbs:
            if not (pellet.x, pellet.y) in getMod().marker_map:
                getMod().mark_blue(pellet.x, pellet.y)

    def reset(self, agent):
        """ reset the environment to its initial state """
        state = self.get_state(agent)
        state.reset()
        agent.state.position = state.initial_position
        agent.state.rotation = state.initial_rotation
        agent.state.velocity = state.initial_velocity
        state.episode_count += 1
        self.add_crumbs()
        print "Episode %d complete" % state.episode_count
        return True

    def get_agent_info(self, agent):
        """ return a blueprint for a new agent """
        return self.init_list.get_info(str(type(agent)))

    def num_sensors(self):
        """
        Return total number of sensors
        """
        return (len(getMod().marker_map)*4 + constants.N_FIXED_SENSORS)
    
    def step(self, agent, action):
        """
        A step for an agent
        """
        state = self.get_state(agent) # the agent's status
        if (state.is_out == True):
            getMod().unmark(agent.state.position.x, agent.state.position.y)
        else:
            assert(self.get_agent_info(agent).actions.validate(action)) # check if the action is valid
            if (str(type(agent)) == "<class 'Roomba.roomba.RoombaBrain'>"):
                angle = action[0] # in range of -pi to pi
                degree_angle = math.degrees(angle)
                delta_angle = degree_angle - agent.state.rotation.z
                delta_dist = constants.MAX_SPEED
            else:
                angle = action[0] # in range of -pi to pi
                degree_angle = math.degrees(angle)
                delta_angle = degree_angle - agent.state.rotation.z
                delta_dist = constants.MAX_SPEED
            reward = self.update_position(agent, delta_dist, delta_angle)
        state.reward += reward
        return reward

    # delta_angle (degrees) is change in angle
    # delta_dist is change in distance (or velocity, since unit of time unchanged)
    def update_position(self, agent, delta_dist, delta_angle):
        """
        Updates position of the agent and collects pellets.
        """
        state = self.get_state(agent)
        state.step_count += 1

        position = agent.state.position
        rotation = agent.state.rotation

        # posteriori collision detection
        rotation.z = common.wrap_degrees(rotation.z, delta_angle)
        position.x += delta_dist*math.cos(math.radians(rotation.z))
        position.y += delta_dist*math.sin(math.radians(rotation.z))

        # check if one of 4 out-of-bound conditions applies
        # if yes, back-track to correct position
        if (position.x) < 0 or (position.y) < 0 or \
           (position.x) > self.XDIM or (position.y) > self.YDIM:
            # correct position
            if (position.x) < 0:
                position.x -= 2 * delta_dist*math.cos(math.radians(rotation.z))
            if (position.y) < 0:
                position.y -= 2 * delta_dist*math.sin(math.radians(rotation.z))
            if (position.x) > self.XDIM:
                position.x -= 2 * delta_dist*math.cos(math.radians(rotation.z))
            if (position.y) > self.YDIM:
                position.y -= 2 * delta_dist*math.sin(math.radians(rotation.z))
        elif position.x < self.XDIM * 0.174 and position.y > self.YDIM * (1.0 - 0.309):
            position.x -= 2 * delta_dist*math.cos(math.radians(rotation.z))
            position.y -= 2 * delta_dist*math.sin(math.radians(rotation.z))
        elif furniture_collide_all(position.x, position.y):
            position.x -= 2 * delta_dist*math.cos(math.radians(rotation.z))
            position.y -= 2 * delta_dist*math.sin(math.radians(rotation.z))
                
        # register new position
        state.position = position
        state.rotation = rotation
        agent.state.position = position
        agent.state.rotation = rotation
        
        reward = 0
        
        # remove all crumbs within ROOMBA_RAD of agent position
        pos = (position.x, position.y)
        for crumb in self.crumbs:
            if (crumb.x, crumb.y) in getMod().marker_map:
                distance = math.hypot(crumb[0] - pos[0], crumb[1] - pos[1])
                if distance < constants.ROOMBA_RAD:
                    getMod().unmark(crumb.x, crumb.y)
                    reward += crumb.reward
                
        # check if agent has expended its step allowance
        if (self.max_steps != 0) and (state.step_count >= self.max_steps):
            state.is_out = True    # if yes, mark it to be removed
        return reward            
    
    def sense(self, agent, sensors):
        """ figure out what the agent should sense """
        state = self.get_state(agent)
        if (str(type(agent)) == "<class 'Roomba.roomba.RoombaBrain'>"):
            if state.bumped:
                sensors[0] = 1
                state.bumped = False
            else:
                sensors[0] = 0

            # get agent's position
            pos = agent.state.position
            sensors[1] = pos.x
            sensors[2] = pos.y
            
            # describe every other crumb on the field!
            self.sense_crumbs(sensors, constants.N_S_IN_BLOCK, constants.N_FIXED_SENSORS, agent)

        else:
            """ Copied over from creativeit branch """
            sensors[0] = constants.MAX_DISTANCE
            sensors[1] = constants.MAX_DISTANCE
            sensors[2] = constants.MAX_DISTANCE
            sensors[3] = constants.MAX_DISTANCE
            sensors[4] = -1
            sensors[5] = constants.MAX_DISTANCE
            
            # The first four sensors detect the distance to the nearest cube in each of the
            # four quadrants defined by the coordinate frame attached to the agent.  The
            # positive X axis of the coordinate frame is oriented in the forward direction
            # with respect to the agent.  The fifth sensor detects the minimum angular
            # distance between the agent and the nearest cubes detected by the other sensors.
            # All sensor readings are normalized to lie in [-1, 1].
            
            for cube_position in getMod().marker_map:
                
                distx = cube_position[0] - agent.state.position.x
                disty = cube_position[1] - agent.state.position.y
                dist = math.hypot(distx, disty)
                angle = math.degrees(math.atan2(disty, distx)) - agent.state.rotation.z  # range [-360, 360]
                if angle > 180: angle = angle - 360
                if angle < -180: angle = angle + 360
                angle = angle/180 # range [-1, 1]
                if angle >= -1 and angle < -0.5:
                    if dist < sensors[0]:
                        sensors[0] = dist
                        if math.fabs(angle) < math.fabs(sensors[4]): sensors[4] = angle
                elif angle >= -0.5 and angle < 0:
                    if dist < sensors[1]:
                        sensors[1] = dist
                        if math.fabs(angle) < math.fabs(sensors[4]): sensors[4] = angle
                elif angle >= 0 and angle < 0.5:
                    if dist < sensors[2]:
                        sensors[2] = dist
                        if math.fabs(angle) < math.fabs(sensors[4]): sensors[4] = angle
                else:
                    if dist < sensors[3]:
                        sensors[3] = dist
                        if math.fabs(angle) < math.fabs(sensors[4]): sensors[4] = angle
                                
            # Any distance sensor that still has the value MAX_DISTANCE is set to -1.
            for i in range(0, 6):
                if i != 4 and sensors[i] >= constants.MAX_DISTANCE:
                    sensors[i] = -1

            # Invert and normalize the remaining distance sensor values to [0, 1]
            maxval = max(sensors[0], sensors[1], sensors[2], sensors[3], sensors[5])
            if maxval > 0:
                for i in range(0, 6):
                    if i != 4 and sensors[i] > 0:
                        sensors[i] = 1 - (sensors[i]/maxval)

            # Now, sensors that do not detect any cubes/wall will have the value -1,
            # sensors that detect cubes/wall at maxval distance will have the value 0,
            # and sensors that detect cubes/wall at zero distance will have value 1.
        return sensors

    def sense_crumbs(self, sensors, num_sensors, start_sensor, agent):
        """
        Generate a (big) array of observations, num_sensors for each crumb
        and store them inside sensors starting at start_sensor.
        Each crumb is stored as: (x,y,exists?,reward)
        """
        i = start_sensor
        closest = None
        closest_distance = None
        pos = agent.state.position
        pos = (pos.x, pos.y)
        for pellet in self.crumbs:
            sensors[i] = pellet.x
            sensors[i+1] = pellet.y
            if (pellet.x, pellet.y) in getMod().marker_map:
                sensors[i+2] = 1
                distance = math.hypot(pellet.x - pos[0], pellet.y - pos[1])
                if closest is None or distance < closest_distance:
                    closest = pellet
                    closest_distance = distance
            else:
                sensors[i+2] = 0 
            sensors[i+3] = pellet.reward
            i = i + num_sensors
        if closest is not None:
            # freebie for scripted agents: tell agent the closest crumb!
            sensors[3] = closest.x
            sensors[4] = closest.y
        return True
                     
    def is_episode_over(self, agent):
        """ is the current episode over for the agent? """
        state = self.get_state(agent)
        if self.max_steps != 0 and state.step_count >= self.max_steps:
            return True
        return False
    
    def cleanup(self):
        """
        cleanup the world
        """
        self.environment = None
        return True

gMod = None

def delMod():
    global gMod
    gMod = None

def getMod():
    global gMod
    if not gMod:
        gMod = SandboxMod()
    return gMod

def ServerMain():
    print "Starting Sandbox Environment"
