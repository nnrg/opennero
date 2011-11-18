"""
Script to show an image in all four stages of edge detection.

Author: Wesley Tansey
11/17/2011
"""

import os, sys
import Tkinter
import Image, ImageTk, ImageFilter
import math
import itertools
import numpy as np
import scipy
import classifier

numpy = True

def gaussian(x, y, sigma):
    return (1 / (2*math.pi*sigma**2))*math.exp(-(x**2+y**2)/(2*sigma**2))

"""
Shifts the rows and columns of an array, putting zeros in any empty spaces
and truncating any values that overflow
"""
def push(np_array, rows, columns):
    result = np.zeros((np_array.shape[0],np_array.shape[1]))
    if rows > 0:
        if columns > 0:
            result[rows:,columns:] = np_array[:-rows,:-columns]
        elif columns < 0:
            result[rows:,:columns] = np_array[:-rows,-columns:]
        else:
            result[rows:,:] = np_array[:-rows,:]
    elif rows < 0:
        if columns > 0:
            result[:rows,columns:] = np_array[-rows:,:-columns]
        elif columns < 0:
            result[:rows,:columns] = np_array[-rows:,-columns:]
        else:
            result[:rows,:] = np_array[-rows:,:]
    else:
        if columns > 0:
            result[:,columns:] = np_array[:,:-columns]
        elif columns < 0:
            result[:,:columns] = np_array[:,-columns:]
        else:
            result[:,:] = np_array[:,:]
    return result

np_orientation = np.array([0,315,45,270,90,225,180,135])
def find_orientation(upper_left, upper_center, upper_right, mid_left, mid_right, lower_left, lower_center, lower_right):
    a = np.array([upper_center, upper_left, upper_right, mid_left, mid_right, lower_left, lower_center, lower_right])
    return np_orientation[a.argmax()]

def button_click_exit_mainloop (event):
    event.widget.quit() # this will cause mainloop to unblock.

"""
Start the GUI
"""
root = Tkinter.Tk()
root.bind("<Button>", button_click_exit_mainloop)
root.geometry('+%d+%d' % (100,100))

# Load the file
print 'Opening: ' + sys.argv[1]
split = sys.argv[1].rpartition('color/')
base_dir = split[0]
name = split[2]
image0 = Image.open(sys.argv[1])

# Resize it to a thumbnail size
width = image0.size[0]
height = image0.size[1]
thumbnail_width = width/2
thumbnail_height = height/2
root.geometry('%dx%d' % (thumbnail_width*2,thumbnail_height*2))
image1 = image0.resize((thumbnail_width,thumbnail_height))
tkpi = ImageTk.PhotoImage(image1)
label_image = Tkinter.Label(root, image=tkpi)
label_image.place(x=0,y=0,width=thumbnail_width,height=thumbnail_height)
root.title(sys.argv[1].rpartition('/')[2])

# Convert the image to black and white
bw = image0.convert("L")
bw.save(base_dir + 'bw/' + name, "PNG")
bw_pix = bw.load()
bw1 = bw.resize((thumbnail_width,thumbnail_height))
tkpi_bw = ImageTk.PhotoImage(bw1)
label_image_bw = Tkinter.Label(root, image=tkpi_bw)
label_image_bw.place(x=thumbnail_width,y=0,width=thumbnail_width,height=thumbnail_height)

# Apply Gaussian filter
sigma = 1

# Fast way to do convolution
np_bw = np.array(bw) # convert to numpy
from scipy.ndimage.filters import gaussian_filter
convolution = gaussian_filter(np_bw, sigma)

# Fast way to save an image from a 2-d numpy array
smoothed = Image.fromarray(convolution)

smoothed.save(base_dir + 'smooth/' + name, "PNG")
smoothed1 = smoothed.resize((thumbnail_width,thumbnail_height))
tkpi_smooth = ImageTk.PhotoImage(smoothed1)
label_image_smooth = Tkinter.Label(root, image=tkpi_smooth)
label_image_smooth.place(x=0,y=thumbnail_height,width=thumbnail_width,height=thumbnail_height)

neighbor_offsets = [(-1,-1,225), (-1,0,270), (-1,1,315), (0,1,0), (0,-1,180), (1,0,90), (1,1,45), (1,-1,125)]

# Fast way of doing edge detection
edges = image0.filter(ImageFilter.FIND_EDGES).convert("L")

edges.save(base_dir + 'edges/' + name, "PNG")
edges1 = edges.resize((thumbnail_width,thumbnail_height))
tkpi_edges = ImageTk.PhotoImage(edges1)
label_image_edges = Tkinter.Label(root, image=tkpi_edges)
label_image_edges.place(x=thumbnail_width,y=thumbnail_height,width=thumbnail_width,height=thumbnail_height)

# Fast way to detect pixel orientation
np_edges = np.array(edges)
upper_left = push(np_edges, 1, 1)
upper_center = push(np_edges, 1, 0)
upper_right = push(np_edges, 1, -1)
mid_left = push(np_edges, 0, 1)
mid_right = push(np_edges, 0, -1)
lower_left = push(np_edges, -1, 1)
lower_center = push(np_edges, -1, 0)
lower_right = push(np_edges, -1, -1)
vfunc = np.vectorize(find_orientation)

orientations = vfunc(upper_left, upper_center, upper_right, mid_left, mid_right, lower_left, lower_center, lower_right)

# Create the area to draw the classification results
canvas_height = 50
canvas_width = 150
canvas_offset = '%d,%d' % (thumbnail_width - canvas_width / 2, thumbnail_height - canvas_height / 2)
print canvas_offset
canvas = Tkinter.Canvas(root, offset=canvas_offset, width=canvas_width, height=canvas_height, bg='white')
canvas.pack()

# Call the classifier to detect what kind of object this is
from classifier import ObjectClassifier
classifier = ObjectClassifier()
result = classifier.classify(np_edges, orientations)

canvas.create_text(canvas_width / 2, canvas_height/2, text=str(result) + '!') 

root.mainloop() # wait until user clicks the window
