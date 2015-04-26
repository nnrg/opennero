# Gaussian Smoothing Code #

Below is the relevant code from `show_image.py` to perform Gaussian smoothing on an image.

```
# Apply Gaussian filter
sigma = 1
bw_pix = bw.load()
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

```