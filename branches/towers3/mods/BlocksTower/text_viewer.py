import Tkinter as tk
import time

class TextViewer:
    def __init__(self, title = 'STRIPS planner output'):
        self.root = tk.Tk()
        self.root.title(title)
        self.frame = tk.Frame()
        self.text = tk.Text(self.frame)
        self.scroll = tk.Scrollbar(self.frame)
        self.text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        self.frame2 = tk.Frame()
        self.frame2.pack(side=tk.BOTTOM)
        self.continue_button = tk.Button(self.frame2, text='Skip Rest')
        self.continue_button.pack(side=tk.RIGHT)
        self.next_button = tk.Button(self.frame2, text='Next Step')
        self.next_button.pack(side=tk.RIGHT)

        self.text.focus_set()

        self.scroll.config(command=self.text.yview)
        self.text.config(font="Courier 12", yscrollcommand=self.scroll.set)
        self.next_button.config(command=self.user_unpause)
        self.continue_button.config(command=self.user_continue)

        self.continued = False
        self.paused = False

    def display_text(self, s):
        self.text.insert(tk.END, s)
        self.text.insert(tk.END, '\n')
        self.text.yview(tk.END)

    def user_pause(self, s):
        self.display_text(s)
        if self.continued:
            return
        self.paused = True
        while self.paused and not self.continued:
            time.sleep(0.1)

    def user_unpause(self):
        self.paused = False

    def user_continue(self):
        self.continued = True
