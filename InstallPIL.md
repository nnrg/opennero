# Installing PIL #

To install PIL, download the latest version (1.1.7) for your OS (<a href='http://www.pythonware.com/products/pil/'>linux, windows</a>, <a href='http://pythonmac.org/packages/py25-fat/'>MacOSX</a>).

In linux, install with:
```
tar xzf Imaging-1.1.7.tar.gz
cd Imaging-1.1.7
python setup.py build
python setup.py install
```

To test your installation of PIL, download a PNG image file such as <a href='hw5_stapler.png'>this stapler</a>:

```
import Image
im = Image.open("hw5_stapler.png")
print im.format, im.size, im.mode
# >> PNG (600, 468) RGBA
bw = im.convert("L")
bw.save("hw5_bw_stapler.png", "PNG")
```

The resulting file should look like <a href='hw5_stapler_bw.png'>this black and white version of the stapler</a>.

If you want to create a new, blank image with the same dimensions and color mode as the black and white photo but perhaps with some custom values based on calculations stored in an array (e.g., if you have an array of edge pixels and want to create an image with them):

```
im = Image.new(screen.mode, screen.size)
im_pixels = im.load()
# iterate over your array and set the image pixels with im_pixels[x,y] = ...
im.save("image.png", "PNG")
```

Keep in mind that in a black and white photo, the values for each pixel are in the range [0,255]. For more information about using PIL with the Image class, see the <a href='http://www.pythonware.com/library/pil/handbook/image.htm'>online handbook</a>.