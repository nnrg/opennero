import fileinput
import re
import sys
import Tkinter as tk
import time
from tree_viewer import TreeViewer

viewer = None
if __name__ == "__main__":
    root = tk.Tk()
    root.title('STRIPS linear planner')
    viewer = TreeViewer(root)

def join_list(l):
    return ", ".join([str(s) for s in l])

def weak_contains(items, target):
    for item in items:
        if weak_match(item, target):
            return True
    return False

def weak_find(items, target):
    for item in items:
        if weak_match(item, target):
            return item
    return None

def weak_match(ground1, ground2):
    """
    Matches a grounded condition if it has the same name and literals
    but ignores the truth value
    """
    if ground1.predicate != ground2.predicate:
        return False
    if len(ground1.literals) != len(ground2.literals):
        return False
    for i, j in zip(ground1.literals, ground2.literals):
        if i != j:
            return False
    return True

def strong_find(items, condition):
    for item in items:
        if strong_match(item, condition):
            return item

def strong_match(ground1, ground2):
    """
    Matches a grounded conditions if it is a weak match and is the same truth value
    """
    return ground1.truth == ground2.truth and weak_match(ground1, ground2)

class World:
    def __init__(self):
        self.state = dict()
        self.goals = set()
        self.known_literals = set()
        self.actions = dict()
    def is_true(self, predicate, literals):
        if predicate not in self.state:
            return False
        return literals in self.state[predicate]
    def is_false(self, predicate, literals):
        return not self.is_true(predicate, literals)
    def set_true(self, predicate, literals):
        if predicate not in self.state:
            self.state[predicate] = set()
        self.state[predicate].add(literals)
    def set_false(self, predicate, literals):
        if predicate in self.state:
            self.state[predicate].remove(literals)
    def add_goal(self, predicate, literals, truth=True):
        g = GroundedCondition(predicate, literals, truth)
        self.goals.add(g)
    def add_literal(self, literal):
        self.known_literals.add(literal)
    def add_action(self, action):
        if action.name not in self.actions:
            self.actions[action.name] = action
    def goal_reached(self):
        for g in self.goals:
            if not g.reached(self):
                return False
        return True

class Condition:
    def __init__(self, predicate, params, truth=True):
        self.predicate = predicate
        self.params = params
        self.truth = truth

    def ground(self, args_map):
        args = list()
        for p in self.params:
            if p in args_map:
                args.append(args_map[p])
            else:
                args.append(p)
        return GroundedCondition(self.predicate, tuple(args), self.truth)

    def __str__(self):
        name = self.predicate
        if not self.truth:
            name = "!" + name
        return "{0}({1})".format(name, join_list(self.params))

class GroundedCondition:
    def __init__(self, predicate, literals, truth=True):
        self.predicate = predicate
        self.literals = literals
        self.truth = truth

    def reached(self, world):
        return world.is_true(self.predicate, self.literals) == self.truth

    def __str__(self):
        name = self.predicate
        if not self.truth:
            name = "!" + name
        return "{0}({1})".format(name, join_list(self.literals))

class Action:
    def __init__(self, name, params, preconditions, postconditions):
        self.name = name
        self.params = params
        self.pre = preconditions
        self.post = postconditions
    def generate_groundings(self, world):
        self.grounds = []
        cur_literals = []
        self.groundings_helper(world.known_literals, cur_literals, self.grounds)
    def groundings_helper(self, all_literals, cur_literals, g):
        if len(cur_literals) == len(self.params):
            args_map = dict(zip(self.params, cur_literals))
            grounded_pre = [p.ground(args_map) for p in self.pre]
            grounded_post = [p.ground(args_map) for p in self.post]
            g.append(GroundedAction(self, cur_literals, grounded_pre, grounded_post))
            return
        for literal in all_literals:
            if literal not in cur_literals:
                self.groundings_helper(all_literals, cur_literals + [ literal ], g)
    def print_grounds(self):
        i = 0
        for g in self.grounds:
            print "Grounding " + str(i)
            print g
            print ""
            i = i + 1
    def __str__(self):
        return "{0}({1})\nPre: {2}\nPost: {3}".format(self.name, join_list(self.params), join_list(self.pre), join_list(self.post))

class GroundedAction:
    def __init__(self, action, literals, pre, post):
        self.action = action
        self.literals = literals
        self.pre = pre
        self.post = post
        # If the precondition specifies some requirement that is not changed in the post condition,
        # then we add that together with the post conditions and call it the "complete" post conditions
        self.complete_post = list(post)
        for p in pre:
            if not weak_contains(self.complete_post, p):
                self.complete_post.append(p)
    def __str__(self):
        return "{0}({1})\nPre: {2}\nPost: {3}".format(self.action.name, join_list(self.literals), join_list(self.pre), join_list(self.post))
    def simple_str(self):
        return "{0}({1})".format(self.action.name, join_list(self.literals))

class ParseState:
    INITIAL=1
    GOAL=2
    ACTIONS=3
    ACTION_DECLARATION=4
    ACTION_PRE=5
    ACTION_POST=6

class InfiniteLoopGuard(object):
    activation_count = 0
    MAX_ACTIVATION_COUNT = 100

    @classmethod
    def reset(cls):
        cls.activation_count = 0

    @classmethod
    def can_continue(cls):
        cls.activation_count += 1
        if cls.activation_count > cls.MAX_ACTIVATION_COUNT:
            return False
        else:
            return True

def create_world(filename):
    w = World()
    predicateRegex = re.compile('(!?[A-Z][a-zA-Z_]*) *\( *([a-zA-Z0-9_, ]+) *\)')
    initialStateRegex = re.compile('init(ial state)?:', re.IGNORECASE)
    goalStateRegex = re.compile('goal( state)?:', re.IGNORECASE)
    actionStateRegex = re.compile('actions:', re.IGNORECASE)
    precondRegex = re.compile('pre(conditions)?:', re.IGNORECASE)
    postcondRegex = re.compile('post(conditions)?:', re.IGNORECASE)
    pstate = ParseState.INITIAL
    cur_action = None
    if filename is None:
        filename = sys.argv[1]

    # Read file
    with open(filename) as f:
        for line in f:
            if line.strip() == "" or line.strip()[:2] == "//":
                continue

            if pstate == ParseState.INITIAL:
                # Get initial state
                m = initialStateRegex.match(line)

                # Check the declaring syntax
                if m is None:
                    raise Exception("Initial state not specified correctly. Line should start with 'Initial state:' or 'init:' but was: " + line)

                # Get the initial state
                preds = re.findall(predicateRegex, line[len(m.group(0)):].strip())

                for p in preds:
                    # get the name of the predicate
                    name = p[0]
                    literals = tuple([s.strip() for s in p[1].split(",")])
                    for literal in literals:
                        w.add_literal(literal)

                    # Note that this is a closed-world assumption, so the only reason to have a negative initial
                    # state is if you have some literals that need to be declared
                    if name[0] == '!':
                        name = name[1:]
                        w.set_false(name, literals)
                    else:
                        w.set_true(name, literals)

                pstate = ParseState.GOAL

            elif pstate == ParseState.GOAL:
                # Get goal state declaration
                m = goalStateRegex.match(line)

                # Check the declaring syntax
                if m is None:
                    raise Exception("Goal state not specified correctly. Line should start with 'Goal state:' or 'goal:' but line was: " + line)

                # Get the goal state
                preds = re.findall(predicateRegex, line[len(m.group(0)):].strip())

                for p in preds:
                    # get the name of the predicate
                    name = p[0]
                    literals = tuple([s.strip() for s in p[1].split(",")])
                    for literal in literals:
                        w.add_literal(literal)

                    # Check if this is a negated predicate
                    truth = name[0] != '!'

                    # If it's negated, update the name
                    if not truth:
                        name = name[1:]

                    # Add the goal condition
                    w.add_goal(name, literals, truth)

                pstate = ParseState.ACTIONS
            elif pstate == ParseState.ACTIONS:
                # Get goal state declaration
                m = actionStateRegex.match(line)

                # Check the declaring syntax
                if m is None:
                    raise Exception("Actions not specified correctly. Line should start with 'Actions:' but line was: " + line)

                pstate = ParseState.ACTION_DECLARATION
            elif pstate == ParseState.ACTION_DECLARATION:

                # Action declarations look just like predicate declarations
                m = predicateRegex.match(line.strip())

                if m is None:
                    raise Exception("Action not specified correctly. Expected action declaration in form Name(Param1, ...) but was: " + line)

                name = m.group(1)
                params = tuple([s.strip() for s in m.group(2).split(",")])

                cur_action = Action(name, params, [], [])

                pstate = ParseState.ACTION_PRE
            elif pstate == ParseState.ACTION_PRE:

                # Precondition declarations look just like state declarations but with a different starting syntax
                m = precondRegex.match(line.strip())

                # Check the declaring syntax
                if m is None:
                    raise Exception("Preconditions not specified correctly. Line should start with 'Preconditions:' or 'pre:' but was: " + line)

                # Get the preconditions
                preds = re.findall(predicateRegex, line[len(m.group(0)):].strip())

                for p in preds:
                    # get the name of the predicate
                    name = p[0]

                    params = tuple([s.strip() for s in p[1].split(",")])

                    # conditions can have literals that have yet to be declared
                    for p in params:
                        if p not in cur_action.params:
                            w.add_literal(p)

                    # Check if this is a negated predicate
                    truth = name[0] != '!'

                    # If it's negated, update the name
                    if not truth:
                        name = name[1:]

                    cur_action.pre.append(Condition(name, params, truth))

                pstate = ParseState.ACTION_POST
            elif pstate == ParseState.ACTION_POST:
                # Precondition declarations look just like state declarations but with a different starting syntax
                m = postcondRegex.match(line.strip())

                # Check the declaring syntax
                if m is None:
                    raise Exception("Postconditions not specified correctly. Line should start with 'Postconditions:' or 'post:' but was: " +line)

                # Get the postconditions
                preds = re.findall(predicateRegex, line[len(m.group(0)):].strip())

                for p in preds:
                    # get the name of the predicate
                    name = p[0]

                    params = tuple([s.strip() for s in p[1].split(",")])

                    # conditions can have literals that have yet to be declared
                    for p in params:
                        if p not in cur_action.params:
                            w.add_literal(p)

                    # Check if this is a negated predicate
                    truth = name[0] != '!'

                    # If it's negated, update the name
                    if not truth:
                        name = name[1:]

                    cur_action.post.append(Condition(name, params, truth))

                # Add this action to the world
                w.add_action(cur_action)

                pstate = ParseState.ACTION_DECLARATION

    for k, v in w.actions.iteritems():
        v.generate_groundings(w)

    return w

debug = True

def linear_solver(world):
    state = []

    # the world state is a dictionary from predicate names to true grounded args of that predicate
    for predicate in world.state:
        for literals in world.state[predicate]:
            state.append(GroundedCondition(predicate, literals, True))

    goals = list(world.goals)
    
    InfiniteLoopGuard.reset()

    return linear_solver_helper(world, state, goals, [])

def linear_solver_helper(world, state, goals, current_plan, depth = 0):
    padding = "".join(["++" for x in range(0,len(current_plan))]) + " "
    plan = []

    #setup debagging info
    if debug:
        this_subgoal_action_list = -1
    """
    print "Current Plan: {0}".format("\n".join([x.simple_str() for x in current_plan]))
    print ""
    print "---------------------------"
    print ""
    """

    if len(goals) == 0:
        return plan

    if depth > 15:
        return None

    if not InfiniteLoopGuard.can_continue():
        viewer.display_text("%s recursive calls reached." % InfiniteLoopGuard.MAX_ACTIVATION_COUNT)
        viewer.display_text("Aborting search to prevent infinite loop.")
        viewer.user_pause("")
        return None

    #display the end goal
    if debug:
        if (depth == 0):
            this_level_subgoal_view = viewer.number_item_viewers
            viewer.add_item_viewer("Goal", [str(x) for x in goals], -1, range(1, len(goals)))
        else:
            this_level_subgoal_view = viewer.number_item_viewers            
            viewer.add_item_viewer("Preconditions", [str(x) for x in goals], -1, range(1, len(goals)))

    i = 0
    while i < len(goals):
        goal = goals[i]

        if debug:
            #viewer.add_item_viewer("State", [str(x) for x in state], 0, [1,3])
            viewer.remove_hidden_index(i, this_level_subgoal_view)
            viewer.set_active_index(i, this_level_subgoal_view)
            viewer.display_text(padding + "Current Plan: {0}".format(" -> ".join([x.simple_str() for x in current_plan])))
            viewer.display_text(padding + "Subgoal: {0}".format(goal))
            viewer.display_text(padding + "Other Goals: {0}".format(", ".join([str(x) for x in goals[i+1:]])))
            viewer.display_text(padding + "State: {0}".format(", ".join([str(s) for s in state])))
            viewer.user_pause("")

        if satisfied(state, goal):
            # recurse
            if debug:
                viewer.add_completed_index(i, this_level_subgoal_view)
                viewer.user_pause(padding + "Satisfied already")
                viewer.display_text("")
            i += 1
            continue

        possible_actions = sorted(get_possible_grounds(world, goal), key=lambda c: initial_state_distance(state, c.pre))

        # otherwise, we need to find a subgoal that will get us to the goal
        # find all the grounded actions which will satisfy the goal
        if debug:
            this_subgoal_action_list = viewer.number_item_viewers
            viewer.add_item_viewer("Actions", [x.simple_str() for x in possible_actions])
            viewer.display_text(padding + "List of possible actions that satisfy {0}:".format(goal))
            viewer.display_text("\n".join([padding + x.simple_str() for x in possible_actions]))
            #viewer.user_pause("")
            action_index = 0

        found = False

        for action in possible_actions:

            if debug:
                viewer.set_active_index(action_index, this_subgoal_action_list)
                viewer.display_text(padding + "Trying next action to satisfy {0}:".format(goal))
                viewer.display_text(padding + str(action).replace("\n", "\n" + padding))
                viewer.user_pause("")

            # check if there is at least 1 action for each precondition which satisfies it
            if not preconditions_reachable(world, action):
                if debug:
                    viewer.add_hidden_index(action_index, this_subgoal_action_list)
                    viewer.display_text(padding + "Some preconditions not reachable by any possible action. Skipping...")
                    action_index += 1                    
                    viewer.user_pause("")
                continue

            # check if the action directly contradicts another goal
            if contains_contradiction(goals, action):
                if debug:
                    viewer.add_hidden_index(action_index, this_subgoal_action_list)                    
                    viewer.display_text(padding + "Action violates another goal state. Skipping...")
                    action_index += 1                    
                    viewer.user_pause("")
                continue

            # if we can't obviously reject it as unreachable, we have to recursively descend.
            if debug:
                viewer.display_text(padding + "Action cannot be trivially rejected as unreachable. Descending...")
                viewer.user_pause("")

            temp_state = list(state)

            subgoals = list(action.pre)

            current_plan.append(action)

            solution = linear_solver_helper(world, temp_state, subgoals, current_plan, depth = depth + 1)

            # we were unable to find
            if solution is None:
                if debug:
                    viewer.add_hidden_index(action_index, this_subgoal_action_list)                    
                    viewer.display_text(padding + "No solution found with this action. Skipping...")
                    action_index += 1
                current_plan.pop()
                continue

            if debug:
                viewer.add_completed_index(i, this_level_subgoal_view)                
                viewer.display_text(padding + "Possible solution found!")
                #viewer.user_pause("")


            # update the state to incorporate the post conditions of our selected action
            for post in action.post:
                update_state(temp_state, post)

            """We need to check if the state deleted any of the previous goals. Three options how to handle this:
            1) Give up
            2) Protect it from happening by backtracking all the way (requires fine-grained tracking of states)
            3) Re-introduce any goal which was deleted
            We choose #3 here, because it actually solves the problem eventually"""
            clobbered = [x for x in goals[0:i] if x != goal and not satisfied(temp_state, x)]
            clob_len = len(clobbered)
            if len(clobbered) > 0:
                """if debug:
                    print padding + "Path satisfies {0} but clobbers other goals: {1}".format(goal, ", ".join([str(x) for x in clobbered]))
                    print padding + "Skipping..."
                current_plan.pop()
                continue"""
                if debug:
                    [viewer.add_item_to_viewer(str(x), this_level_subgoal_view) for x in clobbered]
                    viewer.display_text(padding + "Path satisfies {0} but clobbers other goals: {1}".format(goal, ", ".join([str(x) for x in clobbered])))
                    viewer.display_text(padding + "Re-adding the clobbered goals to the end of the list")
                    viewer.user_pause("")
                [goals.remove(x) for x in clobbered]
                [goals.append(x) for x in clobbered]
                i -= clob_len
                if debug:
                    viewer.display_text(padding + "New goals: {0}".format(", ".join([str(x) for x in goals])))
                    viewer.user_pause("")


            # add the subplan to the plan
            plan.extend(solution)

            # accept the temporary state as valid
            del state[:]
            state.extend(temp_state)
            #state = temp_state

            # add this action to the plan
            plan.append(action)

            if debug:
                viewer.display_text(padding + "New State: " + ", ".join([str(x) for x in state]))
                #viewer.user_pause("")

            i += 1
            found = True
            break

        if not found:
            if debug:
                viewer.display_text("")
                viewer.user_pause("++" + padding + "No actions found to satisfy this subgoal. Backtracking...")
                viewer.display_text("")
                #check and see if we have an action list to get rid of (i.e. if we had to deal with unmet preconditions)
                if not(this_subgoal_action_list == -1):
                    viewer.remove_last_item_viewer(this_subgoal_action_list)
            #current_plan.pop()
            return None
            
        #if we are done looking at this subgoal clean up any old action lists before moving on
        if debug:
            if not(this_subgoal_action_list == -1):
                #there is an old action list we need to get rid of
                viewer.remove_last_item_viewer(this_subgoal_action_list)
                this_subgoal_action_list = -1        
        ############################
        #End while goals
        ###########################
    
    #Get rid of all the planning widgets we created, unless it was the initial goal panel
    if debug:
        #check and see if we have an action list to get rid of (i.e. if we had to deal with unmet preconditions)
        if not(this_subgoal_action_list == -1):
            viewer.remove_last_item_viewer(this_subgoal_action_list)
        if (depth > 0):
            viewer.remove_last_item_viewer(this_level_subgoal_view)
    return plan

def contains_contradiction(state, action):
    for post in action.post:
        m = weak_find(state, post)
        if m != None and m.truth != post.truth:
            return True
    return False


def initial_state_distance(state, preconds):
    count = 0
    for p in preconds:
        if not satisfied(state, p):
            count += 1
    return count

def satisfied(state, goal):
    condition = weak_find(state, goal)

    # we only keep track of positive literals (closed world assumption), so if it's here, it's true
    if goal.truth == True:
        return condition != None

    # if it's not here, we assume it's false
    return condition == None

def preconditions_reachable(world, action):
    for p in action.pre:
        if not precondition_reachable(world, p):
            return False

    return True

def precondition_reachable(world, pre):
    """ Checks if there is any way that this precondition can be satisfied, ever """
    if pre.reached(world):
        return True

    for key,action in world.actions.iteritems():
        for ground in action.grounds:
            for p in ground.post:
                if strong_match(p, pre):
                    return True
    return False

def update_state(state, post):
    # look for the condition (positive or negative) in our state
    condition = weak_find(state, post)

    # if the condition doesn't exist and it's a positive statement, add it
    if post.truth == True:
        if condition is None:
            state.append(post)
    # if the condition exists and it's a negative statement, remove it (closed world assumption)
    elif condition != None and post.truth is False:
        state.remove(condition)

# Gets all grounded actions which have a post condition that includes the goal
def get_possible_grounds(world, goal):
    results = []
    for key,action in world.actions.iteritems():
        for ground in action.grounds:
            for p in ground.post:
                if strong_match(p, goal):
                    results.append(ground)
                    break
    return results

def print_plan(plan):
    viewer.display_text('Plan:')
    viewer.display_text('')
    for x in plan:
        viewer.display_text(x.simple_str())
        print ' '.join(x.literals)
    viewer.display_text('')
    viewer.display_text('Click Execute Plan or close the window to continue!')

def run():
    time.sleep(0.1)

    w = create_world(None)

    # Did someone start us at the goal?
    already_solved = w.goal_reached()

    if not already_solved:
        solution = linear_solver(w)
        if solution is None:
            viewer.user_pause("No solution found :(")
        else:
            viewer.display_text("Solved!")
            print_plan(solution)

def main():
    import threading
    worker = threading.Thread(target=run)
    worker.daemon = True
    worker.start()
    tk.mainloop()
    worker.join()

if __name__ == "__main__":
    main()
