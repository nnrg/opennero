## Introcution ##

The first step in many vision systems is to detect the edges in the image  (see section 24.2.1 of <a href='http://aima.cs.berkeley.edu'>AIMA</a>). Given an image, an edge detection algorithm determines which pixels lay along the boundary of two different elements, such as an agent's head and a distant mountain. The edge maps and histograms can then be classified into different objects. In this demo you will drive a NERO agent around the environment and take snapshots of different objects in it. Each snapshot will then be processed through smoothing and edge detection; building a system that labels these objects is left for an exercise.

The video below shows how it works; you can also run OpenNERO yourself
to test it interactively.

<a href='http://www.youtube.com/watch?feature=player_embedded&v=PlrKywV5ep8' target='_blank'><img src='http://img.youtube.com/vi/PlrKywV5ep8/0.jpg' width='425' height=344 /></a>

## Four-Panel Window Results ##

Each time you take a snapshot, it will be processed using a method similar to that in section 24.2.1 of the AIMA (third edition) textbook. That is, first the brightness is extracted from the original color image, resulting in a gray-scale image. Next, the image is convolved with a Gaussian filter, resulting in a smoothed image. The smoothing step is necessary to detected edges more reliably. The last step is to calculate the brightness gradients for each pixel in the smoothed image. When the gradient is high, there is an edge throught that pixel. The gradients are thresholded, resulting in a map of edges in the image.

The results of these four processing steps are displayed together in a four-panel window that pops up. Below is a description of each panel.

### Color ###
The upper left panel displays the raw, color snapshot taken.

### Black and White ###
The upper right panel displays the color snapshot converted to black and white.

### Smoothed ###
The lower left panel displays the black and white image after having applied a Gaussian filter to smooth out the edges and prepare it for edge detection. The code to perform this smoothing is found in `show_image.py` and [excerpted here](GaussianSmoothingCode.md).

### Edges ###
The lower right panel displays the edge-detected image. The code to perform this process is found in `show_image.py` and [excerpted here](EdgeDetectionCode.md).

## Next Steps ##

Now that you have seen how edge detection works, you can build an object recognition system on top of it. One very basic approach to object recognition is to train a machine learning classifier on a training set of edge-detected images. (Each snapshot that you take and its processed versions are saved into the <tt>snapshots</tt> folder with a timestamped filename, allowing you to easily collect such a training set.)
To implement your classifier, see the [Object Recognition Exercise page](ObjectRecognitionExercise.md).
After you have done this Exercise, the result of the classification is also displayed in the four-panel window: You can then walk around the environment, labeling objects that you see!