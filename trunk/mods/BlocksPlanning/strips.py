import fileinput
import re
import sys

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
        return "{0}({1})\nPre:{2}\nPost: {3}".format(self.action.name, join_list(self.literals), join_list(self.pre), join_list(self.post))
    def simple_str(self):
        return "{0}({1})".format(self.action.name, join_list(self.literals))

class ParseState:
    INITIAL=1
    GOAL=2
    ACTIONS=3
    ACTION_DECLARATION=4
    ACTION_PRE=5
    ACTION_POST=6



# Solve
def solve(world):
    plan = []
    preconds = []
    subgoals = list(world.goals)
    return solve_helper(world, subgoals, preconds, plan)

def solve_helper(world, subgoals, preconds, plan):
    # Check if we're at the goal state
    if len(subgoals) == 0:
        return plan

    if len(plan) > 0 and state_distance(world,preconds) == 0:
        return plan

    padding = ""
    for p in plan:
        padding = padding + " + "

    # Find a goal that we have not currently reached
    for g in subgoals:
        # if we already reached this part of the goal, then do nothing
        if g.reached(world):
            continue

        # get all the grounds which will reach the goal
        candidates = get_possible_grounds(world, g)

        # remove any that would alter the state to violate our preconditions
        # for our subsequent actions. This happens in one of two ways:
        # 1) You can have a postcondition which will directly violate a future precondition
        # 2) You can have a precondition which is not altered in the postcondition and
        #    thus will directly violate a future precondition
        candidates = filter_grounded_actions(candidates, preconds)

        # if we have not reached the goal state and we have no where that can help get us there
        # then we need to back up and try a new plan
        if len(candidates) == 0:
            return None

        # sort them by the minimum distance from the intial state
        # try each one in order, with the precondition as the new subgoal
        for candidate in sorted(candidates, key=lambda c: state_distance(world, c.pre)):

            # merge the existing preconditions with the candidate preconditions
            candpre = merge_preconditions(candidate, preconds)

            # determine the new set of subgoals
            candgoals = merge_goals(world, candidate, subgoals)

            # add the candidate to the list of possibilities
            plan.append(candidate)

            # recursive descent. try adding another subgoal and see if it gets us farther
            result = solve_helper(world, candgoals, candpre, plan)

            # if we found a working plan, then return it
            if result != None:
                return result

            # if the candidate did not work, remove it and try the next guy
            plan.pop()



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

def filter_grounded_actions(grounds, preconds):
    results = []
    # Get the grounded actions for this action
    for ga in grounds:
        if valid_subgoal_action(ga, preconds):
            results.append(ga)
    return results

# Checks if we will make the goal unreachable by using this grounded action
def valid_subgoal_action(grounded_action, preconds):
    # Look at each of our required preconditions for the next action
    for pre in preconds:
        post = weak_find(grounded_action.complete_post, pre)
        if post != None and post.truth != pre.truth:
            return False
    return True

# Calculates the minimum number of changes that must be made to a state to satisfy a
# given set of preconditions
def state_distance(world, preconds):
    count = 0
    for p in preconds:
        if not p.reached(world):
            count += 1
    return count

# Takes a collection of required future preconditions and merges it with a grounded
# action to take before. Requires that the ground has already passed the neceeary filters.
def merge_preconditions(grounded_action, preconds):
    result = list(preconds)
    # look at each precondition in the grounded action
    for p in grounded_action.pre:
        # check if this precondition is already in the list
        m = weak_find(result, p)
        # if it is, we need to remove if it's set to a different truth value
        if m != None:
            if m.truth != p.truth:
                result.remove(m)
        else:
            result.append(p)
    return result

def merge_goals(world, grounded_action, goals):
    result = list(goals)

    # remove any goals that are reached by this action
    for p in grounded_action.post:
        m = weak_find(result, p)
        if m != None and m.truth == p.truth:
            result.remove(m)

    # add new goals for each precondition to the action
    for p in grounded_action.pre:
        m = weak_find(result, p)
        if m is None and world.is_true(p.predicate, p.literals) != p.truth:
            result.append(p)

    return result

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

def main():
    w = create_world(None)

    # Did someone start us at the goal?
    already_solved = w.goal_reached()
    print "Goal already solved? {0}".format(already_solved)

    if not already_solved:
        print "Solving..."
        solution = solve(w)
        if solution is None:
            print "No solution found :("
        else:
            print "Solved! Plan: {0}".format(" -> ".join(reversed([x.simple_str() for x in solution])))
            from show_strips import show_solution
            show_solution(solution)

if __name__ == "__main__":
    main()
