from OpenNero import * # import the OpenNERO C++ API, such as Vector3f
from math import *

class TestEnvironment(Environment):
    def __init__(self):
        """
        Create the environment
        """
        Environment.__init__(self)

        # define the action space with two actions
        action_info = FeatureVectorInfo()
        action_info.add_discrete(-1, 1) # -1, 0, or 1 (first action)
        action_info.add_discrete(-1, 1) # -1, 0, or 1 (second action)

        # define the observation space
        observation_info = FeatureVectorInfo()
        observation_info.add_continuous(0, 1) # between 0 and 100

        # define the reward space
        reward_info = FeatureVectorInfo()        
        reward_info.add_continuous(0,100) # between 0 and 100

        self.agent_info = AgentInitInfo(observation_info, action_info, reward_info)
        print 'Initialized TestEnvironment'
        print self.agent_info
        
    def reset(self, agent):
        """
        Reset the environment to its initial state
        """
        return True

    def get_agent_info(self, agent):
        return self.agent_info

    def step(self, agent, actions):
        """
        Adjust the world and the agent based on its action
        @return the agent's reward
        """
        # If either action is non-zero, then change the position and orientation
        # of the agent.
        if actions[0] != 0 or actions[1] != 0:
            agent.state.position = Vector3f(agent.state.position.x + actions[0],
                                          agent.state.position.y + actions[1],
                                          agent.state.position.z)
            agent.state.rotation = Vector3f(agent.state.rotation.x,
                                          agent.state.rotation.y,
                                          degrees(atan2(actions[1], actions[0])))
        return 0

    def sense(self, agent):
        """
        Figure out what the agent's input (observation) is
        @return the agent's observation
        """
        default_observation = self.agent_info.sensors.get_instance()
        return default_observation

    def is_active(self, agent):
        """
        Return whether the agent should act or not
        """
        return True

    def is_episode_over(self, agent):
        """
        Figure out if the agent should 'die' and 'respawn'
        """
        return False

    def cleanup(self):
        """
        Perform any last-minute cleanup tasks
        """
        pass
