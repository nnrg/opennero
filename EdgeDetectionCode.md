# Edge Detection Code #

Below is the relevant code from `show_image.py` to perform edge detection on a smoothed image.

```
neighbor_offsets = [(-1,-1,225), (-1,0,270), (-1,1,315), (0,1,0), (0,-1,180), (1,0,90), (1,1,45), (1,-1,125)]

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
```