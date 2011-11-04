from strips import TextViewer
import Tkinter as tk
import threading

class RecursiveSolver(TextViewer):
    def __init__(self):
        TextViewer.__init__(self, 'Recursive Solver Text Output')
        self.init_queue = [1,5]
        self.atob = [5,1,4,3,4,1,5,2,]
        self.btoa = [3,5,1,4,2,4,1,5,]
        self.atoc = [5,1,4,3,4,1,1,5,2,5,1,4,]
        self.ctoa = [4,1,5,3,5,1,1,4,2,4,1,5,]
        self.btoc = [3,4,1,5,2,5,1,4,]
        self.ctob = [4,1,5,3,5,1,4,2,]
        self.end_queue = [0,0,0,5,5,1]
        self.num_towers = 3

    def move(self, frm, to):
        if frm == 'Pole1' and to == 'Pole2': return self.atob
        if frm == 'Pole1' and to == 'Pole3': return self.atoc
        if frm == 'Pole2' and to == 'Pole1': return self.btoa
        if frm == 'Pole2' and to == 'Pole3': return self.btoc
        if frm == 'Pole3' and to == 'Pole1': return self.ctoa
        if frm == 'Pole3' and to == 'Pole2': return self.ctob

    def dohanoi(self, n, to, frm, using):
        if n == 0: return []
        level = self.num_towers - n
        prefix = ''.join(['   ' for i in range(level)])
        self.display_text(prefix + "At level {0} goal is to move {1} disks from pole {2} to pole {3}".format(level, n, frm, to))
        if n > 1:
            self.display_text(prefix + "Decomposing the problem:")
            self.display_text(prefix + "Move {0} disks from pole {1} to pole {2}".format(n-1, frm, using))
            self.display_text(prefix + "Then move remaining disk from pole {0} to pole {1}".format(frm, to))
            self.display_text(prefix + "Then move {0} disks from pole {1} to pole {2}".format(n-1, using, frm))
            self.display_text(prefix + "Recursing on each subproblem...")
            self.user_pause('')
        actions1 = self.dohanoi(n-1, using, frm, to)
        self.display_text(prefix + "\tADD ACTION: move remaining disk from {0} to {1}".format(frm, to))
        actions2 = self.move(frm, to)
        actions3 = self.dohanoi(n-1, to, using, frm)
        plan = actions1 + actions2 + actions3
        self.display_text(prefix + "Back at level {0}, the plan is [{1}]".format(level, ', '.join([str(x) for x in plan])))
        return actions1 + actions2 + actions3

    def solve(self):
        for a in self.queue_init():
            continue

    def queue_init(self):
        self.user_pause('Starting to Solve!')
        for a in self.init_queue:
            yield a
        for a in self.dohanoi(3, 'Pole2', 'Pole1', 'Pole3'):
            yield a
        self.user_pause('Problem Solved! Please close the window to continue!')
        for a in self.end_queue:
            yield a

def main():
    solver = RecursiveSolver()
    worker = threading.Thread(target=solver.solve)
    worker.start()
    tk.mainloop()
    worker.join()

if __name__ == "__main__":
    main()
