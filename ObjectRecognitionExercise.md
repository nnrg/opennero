# Object Recognition #

In this assignment you will create a Computer Vision (CV) system to classify different objects in OpenNERO. To create this system, you will build a training set of snapshots, design custom features, and implement a naive Bayes classifier. You will use the first-person NERO mod in OpenNERO (i.e. the [Edge-Detection Demo](EdgeDetection.md)) to take a series of training, testing, and validation images containing objects to classify. Your resulting CV system will be able to walk around the NERO world and identify objects.

## 1. Install the vision extension ##

The first step is to install all the components necessary to run the vision extension. See the [vision installation instructions](VisionMod.md) for more information.

## 2. Create training, testing, and validation sets in OpenNERO ##

Load the vision mod in OpenNERO and click the `First Person Agent` button. This enables you to now move around in first person and take snapshots of the different characters and objects in the world. Each time you take a snapshot, it will be processed using a similar method to the edge detection algorithm is described in section 24.2.1 of your AIMA (third edition) textbook. The results window shows you each of the four steps of edge detection: original (upper left), black and white (upper right), smoothed with a Gaussian filter (lower left), and finally edge pixels detected (lower right). Each intermediate image is saved into the `snapshots` folder with a timestamped filename. Although each of the images is saved, you should **only use the images saved in `snapshots/edges` in your classifier**.

Take at least 40 training, 20 testing, and 20 validation pictures in this world, with a good mix of each of the objects. You want to be able to classify:

  * Steve, the robot agent from the maze world.
  * Sydney, a human agent.
  * a tree.
  * a green cube.

Make sure you have a few of each class of objects in each of your datasets. Also, since the agents are spawned randomly in the world, make sure you gather your samples from multiple runs of the environment; otherwise, your classifier may be learning characteristics about the specific instance of the environment (e.g., mountains and walls in the background) rather than identifying the objects.

## 3. Create Your Object Classifier ##
You will be using a naive Bayes classifier and a set of custom features to classify objects.

### 3a. Implement a naive Bayes classifer ###
As a reminder, a naive Bayes classifier works under the (naive) assumption that all feature probabilities are independent. If we are trying to determine the probability that a certain image should be classified as type C, given that it has a certain collection of features {f1, f2, ..., fn}, then we can write that as P(C | f1, f2, ..., fn). Since we are assuming each of our features is independent given C, we can simplify this to be P(C | f1, f2, ..., fn) = P(C | f1) `*` P(C | f2) `*` ... `*` P(C | fn). To create a classifier, you simply calculate the probability score for each possible value of C (i.e., C = Sydney, C = Steve, C = Tree, C = Cube) and choose the C with the highest score.

In the `classifier.py` file, you will notice a skeleton for your classifier:

```
"""
This is your object classifier. You should implement the train and
classify methods for this assignment.
"""
class ObjectClassifier():
    labels = ['Tree', 'Sydney', 'Steve', 'Cube']
    
    """
    Everytime a snapshot is taken, this method is called and
    the result is displayed on top of the four-image panel.
    """
    def classify(self, edge_pixels, orientations):
        return random.choice(self.labels)
    
    """
    This is your training method. Feel free to change the
    definition to take a directory name or whatever else you
    like. The load_image (below) function may be helpful in
    reading in each image from your datasets.
    """
    def train(self):
        pass
```

Your classifier should implement the `classify` and `train` methods. Each time a snapshot is taken, your `classify` method will be invoked and the results will be displayed at the top of the four-image panel. You probably want to train your model _offline_, then you can simply load your trained model in your constructor.

### 3b. Invent a set of features ###
Each edge detected image is a black image with a white pixel for each edge. Along with information about whether each pixel is an edge, it is also useful to know the orientation of the edges. This information is derived by looking at the maximum gradient from the edge pixel to any of its neighbors. Thus, each call to `classify` also passes an array of orientation values for each pixel, with the following values:

<img src='http://opennero.googlecode.com/svn/wiki/OpenNERO-pixel-orientation.png' />

For instance, if the maximum change in pixel intensity was from the target pixel to the pixel immediately to its right, the orientation of the target pixel would be 90 degrees. See section 24.2.1 of your textbook for more information on how this process works.

Your features should be based on the detected edges you receive from the edge detection algorithm. For example:

  * Number of edge pixels > 100
  * Percentage of pixels that are edge pixels > 20%
  * Number of edge pixels in the top half of the image that are oriented upward (315, 0, or 45) > 50

For each feature, calculate the probability of each class containing that feature. For example, if half of the Sydney examples have more than 100 edge pixels, you would calculate P(C = Sydney | Number of edge pixels > 100) = 0.5. The result will be a probability matrix where each column is a different class and each row is a different feature; the individual cells correspond to the probability that an example is in that class given it has that feature.

Use the training set to calculate your probability matrix, use the testing set to gauge your accuracy and determine whether you need to add more features or if you are starting to overfit, and use the validation set as a hold-out set that you classify **only at the very end of the experiment, once you have decided you are finished**.

Remember, the more features you create and the more dimensions each feature has, the more powerful your classifier can be. However, you will also need more training samples and be in danger of overfitting. Start with a small number of simple binary features at first, and gradually expand until you are happy with your classification error.

## Debugging ##

If you run into any bugs in your program, you can find the error log file for OpenNERO at one of the following locations:

  * **Linux or Mac:** `~/.opennero/nero_log.txt`
  * **Windows:** `"AppData\Local\OpenNERO\nero_log.txt"` or `"Local Settings\Application Data\OpenNERO\nero_log.txt"` depending on the version of Windows you have.