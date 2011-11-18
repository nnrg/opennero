"""
Script to show an image.

Courtesy of Noah Spurrier 2007
http://code.activestate.com/recipes/521918-pil-and-tkinter-to-display-images/
"""

import os, sys
import Tkinter
import Image, ImageTk

def button_click_exit_mainloop (event):
    event.widget.quit() # this will cause mainloop to unblock.

root = Tkinter.Tk()
root.bind("<Button>", button_click_exit_mainloop)
root.geometry('+%d+%d' % (100,100))
print os.getcwd()
print 'Opening: ' + sys.argv[1]
image1 = Image.open(sys.argv[1])
root.geometry('%dx%d' % (image1.size[0],image1.size[1]))
tkpi = ImageTk.PhotoImage(image1)
label_image = Tkinter.Label(root, image=tkpi)
label_image.place(x=0,y=0,width=image1.size[0],height=image1.size[1])
root.title(sys.argv[1].rpartition('/')[2])
root.mainloop() # wait until user clicks the window
