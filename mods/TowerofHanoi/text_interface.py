import Tkinter as tk
import sys


class MyDialog:
    def __init__(self, parent):

        top = self.top = parent #tk.Toplevel(parent)

        tk.Label(top, text="Please enter English sentence:").pack()
        tk.Label(top, text="Example: move disk1 from pole1 to pole2").pack()
        tk.Label(top, text="Example: pick up disk1 from pole1").pack()
        tk.Label(top, text="Example: put down disk1 on pole2").pack()

        self.e = tk.Entry(top, width=50)
        self.e.pack(padx=5)

        self.fBottom = tk.Frame(top)

        c = tk.Button(self.fBottom, text="Close", command=self.close)
        c.pack(side = tk.RIGHT)

        b = tk.Button(self.fBottom, text="Execute", command=self.ok)
        b.pack(side = tk.RIGHT)
        self.value = ""

        self.fBottom.pack(side=tk.BOTTOM)

    def ok(self):
        self.value = self.e.get()
        print self.value
        self.top.destroy()

    def close(self):
        self.value = "close"
        print self.value
        self.top.destroy()


def main():
    root = tk.Tk()
    root.title('Enter Command')
    d = MyDialog(root)
    root.wait_window(d.top)
   

if __name__ == "__main__":
        main()
