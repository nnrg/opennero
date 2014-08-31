import Tkinter as tk
from towers import Towers3
from pprint import pprint
import threading

DH = 10
X = {'Pole1': 25, 'Pole2': 75, 'Pole3': 125}
W = {'Disk1': 25, 'Disk2': 30, 'Disk3': 35}
C = {'Disk1': 'red', 'Disk2': 'green', 'Disk3': 'blue'}

class StripsStateViewer:
    def __init__(self, lock = None):
        self.master = tk.Tk()
        self.master.title('Towers of Hanoi planning state')
        self.canvases = []
        self.handles = {}

        self.fTop = tk.Frame(self.master)

        self.fTop.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        self.fBottom = tk.Frame(self.master)

        self.return_button = tk.Button(self.fBottom, text='Execute Plan')
        self.return_button.config(command=self.return_results)
        self.return_button.pack(side=tk.RIGHT)

        self.continue_button = tk.Button(self.fBottom, text='Skip Rest')
        self.continue_button.config(command=self.user_continue)
        self.continue_button.pack(side=tk.RIGHT)

        self.next_button = tk.Button(self.fBottom, text='Next Step')
        self.next_button.config(command=self.user_unpause)
        self.next_button.pack(side=tk.RIGHT)

        self.fBottom.pack(side=tk.BOTTOM)

        self.continued = tk.BooleanVar()
        self.continued.set(False)

        #self.bind('<<display-text>>', self.display_text_handler)
        #self.bind('<<user-pause>>', self.user_pause_handler)

        #self.message = Queue.Queue()
        #self.callback = Queue.Queue() # wait on items in this queue to continue
 
        for i in range(8):
            self.push_canvas()

        self.plan = None
        self.lock = lock
        if self.lock:
            self.master.after(100, self.update_state)

        def window_close_handler():
            self.return_results()

        self.master.protocol("WM_DELETE_WINDOW", window_close_handler)


    def push_canvas(self):
        canvas = tk.Canvas(self.fTop, width=150, height=100)
        canvas.grid(row=0,column=len(self.canvases))
        canvas.create_rectangle(45/2, 50/2, 55/2, 190/2, fill="grey")
        canvas.create_rectangle(145/2, 50/2, 155/2, 190/2, fill="grey")
        canvas.create_rectangle(245/2, 50/2, 255/2, 190/2, fill="grey")
        canvas.create_rectangle(10/2, 130/2, 290/2, 190/2, fill = "grey")
        for disk in Towers3.DISKS:
            self.handles[(canvas, disk)] = None
        self.canvases.append(canvas)

    def clear_canvas(self,canvas):
        for obj in Towers3.DISKS:
            h = self.handles[(canvas, obj)]
            if h:
                canvas.delete(h)
                self.handles[(canvas, h)] = None

    def update_state(self):
        #if not self.plan:
        #    self.master.after(50, self.update_state)
        #else:
        #    self.master.after(1000, self.quit)
        if not self.plan:
            self.master.after(50, self.update_state)
        if self.continued.get():
            if self.lock and self.lock.locked():
                self.lock.release()

    def quit(self):
        self.master.quit()

    def show_wes_state(self, state, depth = 0, plan = []):
        # convert the state
        state = set([ tuple([c.name()] + list(c.literals)) for c in state ])
        pprint(state)
        self.show_state(state, depth, plan)

    def show_state(self, state, depth = 0, plan = []):
        """ show the state evaluated at the depth and the specified plan"""
        if self.lock:
            self.lock.acquire()
        while depth > len(self.canvases) - 1:
            self.push_canvas()
        for i in range(depth, len(self.canvases)):
            self.clear_canvas(self.canvases[i])
        canvas = self.canvases[depth]
        for disk in Towers3.DISKS:
            if self.handles[(canvas,disk)]:
                canvas.delete(self.handles[(canvas,disk)])
                self.handles[(canvas,disk)] = None
            h = get_height(state, disk) * DH
            pole = Towers3.get_pole(state, disk)
            if pole is not None:
                x = X[pole]
                color = C[disk]
                width = W[disk]
                self.handles[(canvas,disk)] = \
                    canvas.create_rectangle(x-width/2, 150/2-h, x+width/2, 130/2 - h, fill = color)

    def plan_found(self, plan):
        self.plan = plan

    def run(self):
        tk.mainloop()

    def user_unpause(self):
        if self.lock and self.lock.locked():
            self.lock.release()

    def user_continue(self):
        self.continued.set(True)

    def return_results(self):
        self.continued.set(True)
        if not self.plan:
            self.master.after(1000, self.return_results)
        else:
            self.master.quit()

def show_state(state = Towers3.INIT):
    """ show the current state in a window """
    viewer = StripsStateViewer()
    viewer.show_state(state)
    tk.mainloop()

def demo_planner(planner):
    """ demonstrate the planner with the "thought bubble" of the state stack """
    lock = threading.Lock()
    viewer = StripsStateViewer(lock)
    #viewer.show_state(Towers3.INIT)
    thread = threading.Thread(target=planner, args=[viewer,])
    thread.start()
    tk.mainloop()
    thread.join()
    return viewer.plan

def get_height(state, disk):
    """ get the height of the disk given the state """
    for p in state:
        if p[0] == 'On' and p[1] == disk:
            return 1 + get_height(state - set([p]), p[2])
    return 0

if __name__ == "__main__":
    show_state()
