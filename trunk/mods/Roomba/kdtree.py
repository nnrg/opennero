import os

class Node:
    def __init__(self):
        self.location = None
        self.left = None
        self.right = None
    def points(self):
        l = []
        l.append(self.location)
        if self.left:
            l.extend(self.left.points())
        if self.right:
            l.extend(self.right.points())
        return l
    def __str__(self, offset = ''):
        res = offset + str(self.location)
        if self.left:
            res = res + os.linesep + offset + self.left.__str__(offset + '  ')
        if self.right:
            res = res + os.linesep + offset + self.right.__str__(offset + '  ')
        return res
 
def kdtree(pointList, depth=0):
    if not pointList:
        return
 
    # Select axis based on depth so that axis cycles through all valid values
    k = len(pointList[0]) # assumes all points have the same dimension
    axis = depth % k
 
    # Sort point list and choose median as pivot element
    pointList.sort(key=lambda point: point[axis])
    median = len(pointList)/2 # choose median
 
    # Create node and construct subtrees
    node = Node()
    node.location = pointList[median]
    node.left = kdtree(pointList[0:median], depth+1)
    node.right = kdtree(pointList[median+1:], depth+1)
    return node

# this method seems to be buggy so we use the rebuild method instead
# def kdremove(here, point, depth = 0):
#     if not here:
#         return None
#     if here.location == point:
#         return kdtree(here.points()[1:])
#     k = len(here.location) # dimension
#     axis = depth % k
#     if point[axis] < here.location[axis]:
#         here.left = kdremove(here.left, point, depth+1)
#     else:
#         here.right = kdremove(here.right, point, depth+1)
#     return here

def kdremove(here, point):
    if not here:
        return None
    point_list = here.points()
    point_list.remove(point)
    return kdtree(point_list)

def kddistance(point1, point2):
    return sum([(x-y)**2 for x,y in zip(point1, point2)])

def kdsearchnn(here, point, best=None, depth = 0):
    if not here:
        return best
    #print 'kdsearchnn',here.location,point,best,depth
    k = len(here.location) # dimension
    axis = depth % k
    if not best:
        best = here.location
    if kddistance(here.location,point) < kddistance(best,point):
        best = here.location
    # figure out the "near" branch and the "away" branch
    near_child = here.left
    away_child = here.right
    if point[axis] > here.location[axis]:
        near_child = here.right
        away_child = here.left
    # search the near branch first - greed is good
    best = kdsearchnn( near_child, point, best, depth + 1 )
    # search the far branch, if needed
    if kddistance(here.location,point) < kddistance(best,point):
        best = kdsearchnn( child_away, point, best, depth + 1 )
    return best

def main():
    pointList = [(2,3), (5,4), (9,6), (4,7), (8,1), (7,2)]

    print 'KD-tree:'
    tree = kdtree(pointList)
    print tree
    print 'Points in it:', tree.points()

    print 'nearest-neighbor to (5,4):', kdsearchnn(tree, (5,4))
    print 'nearest-neighbor to (0,0):', kdsearchnn(tree, (0,0))
    print 'nearest-neighbor to (10,10):', kdsearchnn(tree, (10,10))

    print 'KD-tree after removing (5,4):'
    tree = kdremove(tree, (5,4))
    print tree
    print 'Points in it:', tree.points(), 'length:'

    print 'KD-tree after removing (8,1):'
    tree = kdremove(tree, (8,1))
    print tree
    print 'Points in it:', tree.points(), 'length:'

if __name__ == "__main__":
    main()
