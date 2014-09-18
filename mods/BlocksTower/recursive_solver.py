from tree_viewer import TreeViewer
import Tkinter as tk
import threading
import time

NUM_DISKS = 3

Pole1 = '1'
Pole2 = '2'
Pole3 = '3'

BEGIN_QUEUE = [1,5]
MOVES = {}
MOVES[Pole1, Pole2] = [5,1,4,3,4,1,5,2,]
MOVES[Pole2, Pole1] = [3,5,1,4,2,4,1,5,]
MOVES[Pole1, Pole3] = [5,1,4,3,4,1,1,5,2,5,1,4,]
MOVES[Pole3, Pole1] = [4,1,5,3,5,1,1,4,2,4,1,5,]
MOVES[Pole3, Pole2] = [3,4,1,5,2,5,1,4,]
MOVES[Pole2, Pole3] = [4,1,5,3,5,1,4,2,]
END_QUEUE = [0,0,0,5,5,1]

class RecursiveSolver:
    def __init__(self, viewer):
        self.num_disks = NUM_DISKS
        self.viewer = viewer

    def move(self, frm, to):
        return MOVES[frm, to]

    def dohanoi(self, n, to, frm, using):
        if n == 0: return []
        level = self.num_disks - n
        prefix = ''.join(['\t' for i in range(level)])
        self.viewer.display_text(prefix + "At level {0} goal is to move {1} disks from pole {2} to pole {3}".format(level, n, frm, to))
        self.viewer.user_pause('')
        if n > 1:
            self.viewer.display_text(prefix + "Decomposing the problem:")
            self.viewer.display_text(prefix + "Move {0} disks from pole {1} to pole {2}".format(n-1, frm, using))
            self.viewer.display_text(prefix + "Then move remaining disk from pole {0} to pole {1}".format(frm, to))
            self.viewer.display_text(prefix + "Then move {0} disks from pole {1} to pole {2}".format(n-1, using, to))
            subgoals = ['Move %s disks from %s to %s' % (n-1, frm, using),
                        'Move disk from %s to %s' % (frm, to),
                        'Move %s disks from %s to %s' % (n-1, using, to)]
            self.viewer.add_item_viewer("Subgoals", subgoals, -1, [])
            self.viewer.user_pause('')

            self.viewer.set_active_index(0, level + 1)
            self.viewer.display_text(prefix + "Recursing on first subgoal...")
            self.viewer.user_pause('')
            actions1 = self.dohanoi(n-1, using, frm, to)
    
            self.viewer.add_completed_index(0, level + 1)
            self.viewer.set_active_index(1, level + 1)
            self.viewer.display_text(prefix + "Handling second subgoal...")
            self.viewer.display_text(prefix + "Adding action: Move remaining disk from {0} to {1}".format(frm, to))
            self.viewer.user_pause('')
            actions2 = self.move(frm, to)
    
            self.viewer.add_completed_index(1, level + 1)
            self.viewer.set_active_index(2, level + 1)
            self.viewer.display_text(prefix + "Recursing on third subgoal...")
            self.viewer.user_pause('')
            actions3 = self.dohanoi(n-1, to, using, frm)
    
            self.viewer.add_completed_index(2, level + 1)
            plan = actions1 + actions2 + actions3
            #self.display_text(prefix + "Back at level {0}, the plan is [{1}]".format(level, ', '.join([str(x) for x in plan])))
            self.viewer.display_text(prefix + "Subgoals are completed...")
            self.viewer.user_pause('')
            self.viewer.remove_last_item_viewer(level + 1)
            return actions1 + actions2 + actions3
        else:
            self.viewer.display_text(prefix + "Entering base case...".format(frm, to))
            self.viewer.display_text(prefix + "Adding action: Move single disk from {0} to {1}".format(frm, to))
            actions2 = self.move(frm, to)
            self.viewer.user_pause('')
            return actions2


    def solve(self):
        time.sleep(0.1)
        for a in self.queue_init():
            continue

    def queue_init(self):
        self.viewer.add_item_viewer("Goal", ['Move %s disks from %s to %s' % (self.num_disks, Pole1, Pole3)], -1, [])
        self.viewer.display_text('Starting to Solve!')
        self.viewer.user_pause('')
        self.viewer.set_active_index(0, 0)
        actions = self.dohanoi(self.num_disks, Pole3, Pole1, Pole2)
        self.viewer.add_completed_index(0, 0)
        self.viewer.display_text('Problem Solved! Please click Execute Plan or close the window to continue!')
        self.viewer.user_pause('')
        return BEGIN_QUEUE + actions + END_QUEUE

def main():
    root = tk.Tk()
    root.title('Problem Decomposition')
    viewer = TreeViewer(root)
    solver = RecursiveSolver(viewer)
    worker = threading.Thread(target=solver.solve)
    worker.daemon = True
    worker.start()
    tk.mainloop()
    worker.join()

if __name__ == "__main__":
    main()
