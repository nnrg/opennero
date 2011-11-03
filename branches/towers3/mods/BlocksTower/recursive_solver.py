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
        if frm == 'a' and to == 'b': return self.atob
        if frm == 'a' and to == 'c': return self.atoc
        if frm == 'b' and to == 'a': return self.btoa
        if frm == 'b' and to == 'c': return self.btoc
        if frm == 'c' and to == 'a': return self.ctoa
        if frm == 'c' and to == 'b': return self.ctob
    def dohanoi(self, n, to, frm, using):
        if n == 0: return
        prefix = ''.join(['   ' for i in range(n)])
        strn = prefix + "Moving depth {n} from {frm} to {to} using {using}".format(n=n, frm=frm, to=to, using=using)
        for a in self.dohanoi(n-1, using, frm, to):
            yield a
        self.user_pause(strn)
        for a in self.move(frm, to):
            self.display_text(prefix + ' primitive action: ' + str(a))
            yield a
        self.user_pause(prefix + 'Higher level action complete!')
        for a in self.dohanoi(n-1, to, using, frm):
            yield a
    def solve(self):
        for a in self.queue_init():
            continue
    def queue_init(self):
        self.user_pause('Starting to Solve!')
        for a in self.init_queue:
            yield a
        for a in self.dohanoi(3, 'b', 'a', 'c'):
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
