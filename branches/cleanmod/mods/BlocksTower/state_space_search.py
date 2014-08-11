from towers import Towers3
from canvas_viewer import demo_planner

__doc__ = """This is an example planning algorithm for Towers of Hanoi"""
__author__ = "Igor Karpov <ikarpov@cs.utexas.edu>"
__version__ = "0.1"

DEPTH = 7

def solve(start, goal, actions, depth=DEPTH, plan=[], viewer = None ):
    """
    solve( start, goal, actions, depth=DEPTH )

    A STRIPS-like planner

    Arguments:

    start -- a set of tuples that make up the starting state
    goal -- a set of tuples that make up the goal state
    actions -- a list of (Do, Undo) pairs that operate on the state
        (currently we assume that the signature is
        Do(Disk, Source, Dest) -> {True, False})
    depth -- the maximum depth of recursion to allow the algorithm to search for a plan

    Returns:

    A plan of actions (represented as tuples with function followed by their arguments) which will,
    starting at the start state, reach the goal state
    """
    # if the goal is a subset of the starting state
    if goal.issubset(start):
        # this is already solved!
        if viewer:
            viewer.show_state(start, DEPTH-depth, plan)
            viewer.plan_found(plan)
        return plan
    # if we are below depth, stop
    elif depth <= 0:
        return None
    # otherwise we actually need to do some work
    else:
        # we will need to modify the state so we make a copy of it
        state = set(start)
        if viewer:
            viewer.show_state(state, DEPTH-depth, plan)
        # try all actions with all parameters
        # WARNING: this could get big quickly!
        for (do, undo) in actions:
            for Disk in Towers3.DISKS:
                for Source in set(Towers3.LITERALS) - set([Disk]):
                    for Dest in set(Towers3.LITERALS) - set([Disk, Source]):
                        action = (do, Disk, Source, Dest)
                        if do(state, Disk, Source, Dest):
                            new_state = frozenset(state)
                            # yield all of the states and plans encountered below us
                            sln = solve(new_state, goal, actions, depth = depth - 1, plan = plan + [action], viewer=viewer)
                            if sln is not None:
                                return sln
                            else:
                                # undo the action if it didn't work (backtrack)
                                undo(state, Disk, Source, Dest)
        return None

def print_plan(plan):
    for action in plan:
        print "%s %s" % (action[0].__name__, ' '.join(action[1:]))

if __name__ == "__main__":
    def planner(viewer):
        solve(Towers3.INIT, Towers3.GOAL, Towers3.get_actions(), viewer=viewer)
    plan = demo_planner(planner)
    if plan is not None:
        print_plan(plan)
