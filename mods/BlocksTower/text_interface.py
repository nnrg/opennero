import Tkinter as tk
import sys
from text_viewer import TextViewer


class MyDialog:
    def __init__(self, parent):

        top = self.top = parent #tk.Toplevel(parent)

        tk.Label(top, text="Please enter English sentence:").pack()

        self.e = tk.Entry(top)
        self.e.pack(padx=5)

        b = tk.Button(top, text="Enter", command=self.ok)
        b.pack(pady=5)
        self.value = ""

        c = tk.Button(top, text="Close", command=self.close)
        c.pack(pady=5)

    def ok(self):
        self.value = self.e.get()
        print self.value
        self.top.destroy()

    def close(self):
        self.top.destroy()


def main():
    root = tk.Tk()
    root.title('Enter Command')
    d = MyDialog(root)
    root.wait_window(d.top)
   

if __name__ == "__main__":
        main()
