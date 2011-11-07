import random
from math import *

__author__ = "Igor V. Karpov (ikarpov@cs.utexas.edu)"
__date__ = "October 29, 2007"

"""
Automatic maze generation. For for more information, see:
http://en.wikipedia.org/wiki/Maze_generation_algorithm
"""

class Maze:
    """
    a 2-d grid world maze
    """
    def __init__(self, rows, cols, walls, dx, dy):
        assert(rows > 0)
        assert(cols > 0)
        assert(dx > 0)
        assert(dy > 0)
        self.rows = rows
        self.cols = cols
        self.walls = walls
        self.dx = dx
        self.dy = dy

    @staticmethod
    def from_data(maze_data):
        assert(len(maze_data) == 5)
        rows = maze_data[0]
        cols = maze_data[1]
        walls = maze_data[2]
        dx = maze_data[3]
        dy = maze_data[4]
        return Maze(rows, cols, walls, dx, dy)

    def get_data(self):
        return (self.rows, self.cols, self.walls, self.dx, self.dy)

    @staticmethod
    def generate(rows, cols, dx, dy):
        """
        Randomized Kruskal's algorithm for automatic maze generation
        """
        walls = set([]) # a list of walls, where each wall is ((r1,c1),(r2,c2))
        cells = Grouper() # disjoint sets of cells
        for r in xrange(rows):
            for c in xrange(cols):
                # for each cell, add west and north walls
                walls.add(((r - 1, c), (r, c))) # north wall
                walls.add(((r, c - 1), (r, c))) # west wall
                cells.join((r, c)) # add the cell
        for r in xrange(rows):
            walls.add(((r, cols - 1), (r, cols))) # east wall
        for c in xrange(cols):
            walls.add(((rows - 1, c), (rows, c))) # south wall
        wall_list = list(walls)
        random.shuffle(wall_list) # randomly order the walls
        for (cell1, cell2) in wall_list:
            if not cells.joined(cell1, cell2):
                walls.remove((cell1, cell2))
                cells.join(cell1, cell2)
        return Maze(rows, cols, walls, dx, dy)

    def rc_goal(self, r, c):
        "check if r,c is the goal state"
        return r == self.rows - 1 and c == self.cols - 1

    def rc_bounds(self, r, c):
        "check in bounds row col"
        return r >= 0 and c >= 0 and r < self.rows and c < self.cols

    def xy_bounds(self, x, y):
        "check in bounds x y"
        (r,c) = self.xy2rc(x,y)
        return self.rc_bounds(r,c)

    def xy2rc(self, x, y):
        "convert x y to row col"
        return (int(round(x/self.dx))-1, int(round(y/self.dy))-1)

    def rc2xy(self, r, c):
        "convert row, col to x,y"
        return ((r+1) * self.dx, (c+1) * self.dy)

    def xy_limits(self):
        "return ( (xmin, ymin), (xmax, ymax) )"
        (xmin, ymin) = self.rc2xy(0,0)
        (xmax, ymax) = self.rc2xy(self.rows - 1, self.cols - 1)
        xmin -= 0.5 * self.dx
        ymin -= 0.5 * self.dy
        xmax += 0.5 * self.dx
        ymax += 0.5 * self.dy
        return ( (xmin, ymin), (xmax, ymax) )

    def xy_valid(self, x1, y1, x2, y2):
        (r1,c1) = self.xy2rc(x1,y1)
        (r2,c2) = self.xy2rc(x2,y2)
        return self.rc_valid(r1,c1,r2-r1,c2-c1)

    def rc_valid(self, r, c, dr, dc):
        return not self.is_wall(r,c,dr,dc)

    def is_wall(self,r,c,dr,dc):
        """
        return true iff there is a wall between r,c and r+dr,c+dc
        """
        assert(self.rc_bounds(r,c))
        if dr != 0 and dc != 0:
            return True # can't move diagonally
        if abs(dr) > 1 or abs(dc) > 1:
            return True # can't teleport
        if not self.rc_bounds(r+dr,c+dc):
            return True
        elif ((r,c),(r+dr,c+dc)) in self.walls:
            return True
        elif ((r+dr,c+dc),(r,c)) in self.walls:
            return True
        else:
            return False

    def __str__(self):
        """
        print the maze for debugging
        """
        s = ''
        for c in xrange(self.cols):
            if ((-1, c), (0, c)) in self.walls:
                s += ' _'
            else:
                s += '  '
        s += '\n'
        for r in xrange(self.rows):
            for c in xrange(self.cols):
                if ((r, c - 1), (r, c)) in self.walls:
                    s += '|'
                else:
                    s += ' '
                if ((r, c), (r + 1, c)) in self.walls:
                    s += '_'
                else:
                    s += ' '
                if ((r, self.cols - 1), (r, self.cols)) in self.walls:
                    s += '|'
            s += '\n'
        return s

class Grouper(object):
    """
    Borrowed from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/387776
    Author: Michael Droettboom
    This class provides a lightweight way to group arbitrary objects
    together into disjoint sets when a full-blown graph data structure
    would be overkill.
    """
    def __init__(self, init=[]):
        mapping = self._mapping = {}
        for x in init:
            mapping[x] = [x]

    def join(self, a, *args):
        """
        Join given arguments into the same set.
        Accepts one or more arguments.
        """
        mapping = self._mapping
        set_a = mapping.setdefault(a, [a])

        for arg in args:
            set_b = mapping.get(arg)
            if set_b is None:
                set_a.append(arg)
                mapping[arg] = set_a
            elif set_b is not set_a:
                if len(set_b) > len(set_a):
                    set_a, set_b = set_b, set_a
                set_a.extend(set_b)
                for elem in set_b:
                    mapping[elem] = set_a

    def joined(self, a, b):
        """Returns True if a and b are members of the same set."""
        mapping = self._mapping
        try:
            return mapping[a] is mapping[b]
        except KeyError:
            return False

    def __iter__(self):
        """Returns an iterator returning each of the disjoint sets as a list."""
        seen = set()
        for elem, group in self._mapping.iteritems():
            if elem not in seen:
                yield group
                seen.update(group)

if __name__ == "__main__":
    print Maze.generate(10, 10, 20, 20)

