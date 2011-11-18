import numpy as np
import random

class ObjectClassifier():
    labels = ['Tree', 'Sydney', 'Steve', 'Nothing']
    def classify(self, edge_pixels, orientations):
        return random.choice(self.labels)
