# The Vision Environment #

<img src='http://opennero.googlecode.com/svn/wiki/OpenNERO-vision.png' height='300' width='400></img'>

To run the demo,<br>
<br>
<ol><li>Download the Vision Environment for OpenNERO.<br>
</li><li>Start the Vision Experiment.<br>
</li><li>Click the First Person Agent button.<br>
</li><li>Position your agent with the W, A, S, and D keys.<br>
</li><li>When you have an object in sight, press the Snapshot button.</li></ol>

Each snapshot will automatically be processed by OpenNERO and the results will be displayed in a four-panel window. Note that the default version included in the demo is a faster, more efficient implementation of the canonical edge detection algorithm; it should complete its analysis in about 10 seconds and requires <code>numpy</code> and <code>scipy</code> to be installed. If you prefer to use the slower, more canonical implementation of the edge detection algorithm, or if you are not able to install <code>numpy</code> or <code>scipy</code>, you can checkout the cs343vision2 branch.<br>
<br>
<h2>Downloading the Vision Environment</h2>

Follow the steps below to download and install the vision environment in OpenNERO.<br>
<br>
Download the <a href='http://opennero.googlecode.com/svn/wiki/Hw5.tar.gz'>vision environment files</a>.  Extract the archive inside your OpenNERO installation folder.<br>
<br>
<h2>2. Install the Python Imaging Library</h2>

We are going to be using the <a href='http://www.pythonware.com/products/pil/'>Python Imaging Library</a> to help with loading, saving, and manipulating the images we take in OpenNERO. If you do not already have it installed, see the <a href='InstallPIL.md'>install instructions</a>.<br>
<br>
<h2>3. Install <code>numpy</code> and <code>scipy</code></h2>

You will need to <a href='http://www.scipy.org/Installing_SciPy'>install numpy and scipy</a> if you are not already able to import them.