"""
Script to show an image in all four stages of edge detection.

Author: Wesley Tansey
11/17/2011
"""

import os, sys
import Tkinter
import Image, ImageTk
import math
import itertools

def gaussian(x, y, sigma):
    return (1 / (2*math.pi*sigma**2))*math.exp(-(x**2+y**2)/(2*sigma**2))

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
convolution = [ [0 for col in range(bw.size[1])] for row in range(bw.size[0])]
# Loop over every pixel in the image and apply the Gaussian filter
for (x,y) in itertools.product(range(bw.size[0]), range(bw.size[1])):
    gauss_totals = 0
    # For each neighboring pixel
    for(u, v) in itertools.product(range(-3*sigma + x,3*sigma+1+x),range(-3*sigma + y,3*sigma+1+y)):
        if(u < 0 or v < 0 or u >= width or v >= height):
            continue
        gauss_totals += gaussian(x-u,y-v,sigma)
        convolution[x][y] += (bw_pix[u,v]) * gaussian(x-u,y-v,sigma)

    convolution[x][y] /= gauss_totals

smoothed = Image.new(bw.mode, bw.size)
smoothed_pix = smoothed.load()
for x, xval in enumerate(convolution):
    for y, yval in enumerate(convolution[x]):
        smoothed_pix[x,y] = int(convolution[x][y])
smoothed.save(base_dir + 'smooth/' + name, "PNG")
smoothed1 = smoothed.resize((thumbnail_width,thumbnail_height))
tkpi_smooth = ImageTk.PhotoImage(smoothed1)
label_image_smooth = Tkinter.Label(root, image=tkpi_smooth)
label_image_smooth.place(x=0,y=thumbnail_height,width=thumbnail_width,height=thumbnail_height)

gradient_threshold = 10

neighbor_offsets = [(-1,-1,225), (-1,0,270), (-1,1,315), (0,1,0), (0,-1,180), (1,0,90), (1,1,45), (1,-1,125)]

gradients = [ [(0,0) for col in range(bw.size[1])] for row in range(bw.size[0])]

# Loop over every pixel in the image and calculate the largest gradient
for (x,y) in itertools.product(range(bw.size[0]), range(bw.size[1])):
    pixel = convolution[x][y]
    # For each immediately neighboring pixel
    for (uidx, vidx, angle) in neighbor_offsets:
        u = x + uidx
        v = y + vidx
        if u < 0 or y < 0 or u >= width or v >= height:
            continue
        gradient = abs(convolution[u][v] - pixel)
        if gradient > gradients[x][y][0]:
            gradients[x][y] = (gradient, angle)

# Draw the edges
edges = bw.point(lambda i: 255)
edges_pix = edges.load()

results = [ [(False,0) for col in range(bw.size[1])] for row in range(bw.size[0])]

# Loop over every pixel in the image and determine if it's an edge pixel
for (x,y) in itertools.product(range(bw.size[0]), range(bw.size[1])):
    gradient = gradients[x][y]
    # the gradient must be greater than some threshold set by the user
    if gradient[0] < gradient_threshold:
        continue
    is_max = True
    # determine if the gradient is a local maximum
    for (uidx, vidx, angle) in neighbor_offsets:
        u = x + uidx
        v = y + vidx
        if u < 0 or y < 0 or u >= width or v >= height:
            continue
        if gradients[u][v][0] > gradient[0]:
            is_max = False
            break
    if is_max:
        edges_pix[x,y] = 0
        results[x][y] = (True,angle)

edges.save(base_dir + 'edges/' + name, "PNG")
edges1 = edges.resize((thumbnail_width,thumbnail_height))
tkpi_edges = ImageTk.PhotoImage(edges1)
label_image_edges = Tkinter.Label(root, image=tkpi_edges)
label_image_edges.place(x=thumbnail_width,y=thumbnail_height,width=thumbnail_width,height=thumbnail_height)

root.mainloop() # wait until user clicks the window
