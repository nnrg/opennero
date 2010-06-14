import cv
image = cv.LoadImage("full-image.jpg", 0)
edges = cv.CreateImage((image.width, image.height), image.depth, image.nChannels)
cv.Canny(image, edges, 500, 600, 3)
cv.SaveImage("edges.png", edges)
