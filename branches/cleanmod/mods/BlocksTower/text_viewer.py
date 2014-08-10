import Tkinter as tk
import time
import threading
import Queue

class TextViewer(tk.Frame):
    def __init__(self, master, **options):
        tk.Frame.__init__(self, master, **options)
        self.master = master

        self.fTop = tk.Frame(self)

        self.text = tk.Text(self.fTop)
        self.text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.scroll = tk.Scrollbar(self.fTop)
        self.scroll.pack(side=tk.RIGHT, fill=tk.Y)

        self.text.config(font="Courier 12", yscrollcommand=self.scroll.set)
        self.scroll.config(command=self.text.yview)
        
        self.fTop.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        self.fBottom = tk.Frame(self)

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
        
        self.paused = tk.BooleanVar()
        self.paused.set(False)

        self.continued = tk.BooleanVar()
        self.continued.set(False)

        self.bind('<<display-text>>', self.display_text_handler)
        self.bind('<<user-pause>>', self.user_pause_handler)
        
        self.message = Queue.Queue()
        self.callback = Queue.Queue() # wait on items in this queue to continue
        
        self.pack(fill=tk.BOTH, expand=True)

        def window_close_handler():
            self.return_results()

        self.master.protocol("WM_DELETE_WINDOW", window_close_handler)

    def display_text(self, s):
        self.message.put(s)
        self.event_generate('<<display-text>>')

    def display_text_handler(self, event=None):
        s = self.message.get()
        self.text.insert(tk.END, s)
        self.text.insert(tk.END, '\n')
        self.text.yview(tk.END)

    def user_pause(self, s):
        self.display_text(s)
        self.event_generate('<<user-pause>>')
        # blocks until the callback queue is filled
        self.callback.get()

    def user_pause_handler(self, event=None):
        if self.continued.get():
            self.callback.put('go ahead')
        # now check back every now and then until 
        self.paused.set(True)
        self.after(100, self.check_if_unpaused)

    def check_if_unpaused(self, event=None):
        if self.continued.get() or not self.paused.get():
            self.callback.put('go ahead 2')
        else:
            self.after(100, self.check_if_unpaused)
 
    def user_unpause(self):
        self.paused.set(False)

    def user_continue(self):
        self.paused.set(False)
        self.continued.set(True)

    def return_results(self):
        if self.continued.get() or not self.paused.get():
            self.master.quit()
        else:
            # Viewer is paused. Continuing before returning...
            self.user_continue()
            self.master.after(1000, self.master.quit)

if __name__ == "__main__":
    viewer = TextViewer()
    tk.mainloop()
