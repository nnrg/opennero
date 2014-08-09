from text_viewer import TextViewer
import Tkinter as tk
import threading
import time

Pole1 = '1'
Pole2 = '2'
Pole3 = '3'

BEGIN_QUEUE = [1,5]
ACTIONS = {}
ACTIONS[Pole1, Pole2] = [5,1,4,3,4,1,5,2,]
ACTIONS[Pole2, Pole1] = [3,5,1,4,2,4,1,5,]
ACTIONS[Pole1, Pole3] = [5,1,4,3,4,1,1,5,2,5,1,4,]
ACTIONS[Pole3, Pole1] = [4,1,5,3,5,1,1,4,2,4,1,5,]
ACTIONS[Pole3, Pole2] = [3,4,1,5,2,5,1,4,]
ACTIONS[Pole2, Pole3] = [4,1,5,3,5,1,4,2,]
END_QUEUE = [0,0,0,5,5,1]

class RecursiveSolver:
    def __init__(self, viewer):
        self.num_towers = 3
        self.viewer = viewer

    def move(self, frm, to):
        return ACTIONS[frm, to]

    def dohanoi(self, n, to, frm, using):
        if n == 0: return []
        level = self.num_towers - n
        prefix = ''.join(['\t' for i in range(level)])
        self.viewer.display_text(prefix + "At level {0} goal is to move {1} disks from pole {2} to pole {3}".format(level, n, frm, to))
        if n > 1:
            self.viewer.display_text(prefix + "Decomposing the problem:")
            self.viewer.display_text(prefix + "Move {0} disks from pole {1} to pole {2}".format(n-1, frm, using))
            self.viewer.display_text(prefix + "Then move remaining disk from pole {0} to pole {1}".format(frm, to))
            self.viewer.display_text(prefix + "Then move {0} disks from pole {1} to pole {2}".format(n-1, using, frm))
            self.viewer.display_text(prefix + "Recursing on each subproblem...")
            self.viewer.user_pause('')
        actions1 = self.dohanoi(n-1, using, frm, to)
        self.viewer.display_text(prefix + "ADD ACTION: move remaining disk from {0} to {1}".format(frm, to))
        self.viewer.user_pause('')
        actions2 = self.move(frm, to)
        actions3 = self.dohanoi(n-1, to, using, frm)
        plan = actions1 + actions2 + actions3
        #self.display_text(prefix + "Back at level {0}, the plan is [{1}]".format(level, ', '.join([str(x) for x in plan])))
        return actions1 + actions2 + actions3

    def solve(self):
        time.sleep(0.1)
        for a in self.queue_init():
            continue

    def queue_init(self):
        self.viewer.user_pause('Starting to Solve!')
        actions = self.dohanoi(3, Pole2, Pole1, Pole3)
        self.viewer.user_pause('Problem Solved! Please close the window to continue!')
        return BEGIN_QUEUE + actions + END_QUEUE

def main():
    root = tk.Tk()
    root.title('Problem Decomposition')
    viewer = TextViewer(root)
    solver = RecursiveSolver(viewer)
    worker = threading.Thread(target=solver.solve)
    worker.daemon = True
    worker.start()
    tk.mainloop()
    worker.join()

if __name__ == "__main__":
    main()
