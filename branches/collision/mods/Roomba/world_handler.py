import random
import re
import string
import os
world = "world_config.txt"  # the file we're operating on

class Pellet:
    def __init__(self, reward, mode = "manual", other_info = None):
        self.reward = reward
        self.info = other_info
        self.mode = mode
        self.placement(mode)
    def placement(self, mode):
        if (mode == "manual"):
            # requires 2 values: x and y
            if (self.info == None or len(self.info) == 0):
                self.x = 0
                self.y = 0
            else:
                self.x = self.info[0]
                if (len(self.info) >= 1):
                    self.y = self.info[1]
                else:
                    self.y = self.x
        if (mode == "random"):
            # requires 4 values: size_x, size_y, offset_x, offset_y
            #if self.info == None or len(self.info) == 0:
            #   size_x, size_y = 100, 100
            #   offset_x, offset_y = 0, 0
            #else:
            size_x = self.info[0]
            size_y = self.info[1]
            offset_x = self.info[2]
            offset_y = self.info[3]
            self.x = random.random()*size_x + offset_x
            self.y = random.random()*size_y + offset_y
        if (mode == "cluster"):
            # requires 4 values: center_x, center_y, spread_x, spread_y
            center_x = self.info[0]
            center_y = self.info[1]
            spread_x = self.info[2]
            spread_y = self.info[3]
            self.x = random.normalvariate(center_x, spread_x) 
            self.y = random.normalvariate(center_y, spread_y)
    def __len__(self):
        return 2 # number of dimensions for kdtree
    def __getitem__(self, key):
        if key == 0:
            return self.x
        elif key == 1:
            return self.y
        else:
            raise IndexError
    def __str__(self):
        return 'Pellet(%.2f, %.2f, reward = %f)' % (self.x, self.y, self.reward)

def add_random_pellets(num_pellets, file_name = world):
    "Add a specified number of random pellets into the world"
    try:
        out_file = open(file_name, 'a')
        for i in range (0, num_pellets):
            out_file.write("random\t")
            out_file.write("random\t")
            out_file.write("1\t")
            out_file.write("1\n")
        out_file.close()
        return True
    except IOError:
        print "ALERT! Cannot open: " + file_name + ". File does not exists."
        return False
    
def write_pellets(pellets, file_name = world):
    try:
        out_file = open(file_name, 'a')
        
        if (type(pellets) == type([])):
            for pellet in pellets:
                out_file.write("\n" + str(pellet.reward) + "\t")
                out_file.write(pellet.mode + "\t")
                for item in pellet.info:
                    out_file.write(str(item) + "\t")        
            out_file.close()
            return True            
        elif (type(pellets) == Pellet):
            out_file.write("\n" + str(pellet.reward) + "\t")
            out_file.write(pellet.mode + "\t")
            for item in pellet.info:
                out_file.write(str(item) + "\t")
            out_file.close()
            return True
    except IOError:
        print "ALERT! Cannot open: " + file_name + ". File does not exists."
        return False
    

def clear_map(file_name = world):
    try:
        # Read Dimensions
        in_file = open(file_name, "r")
        data = {}
        # Skip 1 lines (filler)
        in_file.readline()
        
        for i in range(3):
            line = in_file.readline()
            info = line.split("\t")
            data[info[0]] = info[1][0:-1]
        in_file.close()
        
        # write Dimensions
        out_file = open(file_name, "w")
        out_file.write("Dimensions\n")
        for elt in ("X_DIM", "Y_DIM", "Z_DIM"):
            out_file.write(elt + "\t")
            out_file.write(data[elt] + "\n") 
        out_file.write("\nPellets\n")       
        out_file.close()
        return data
    except IOError:
        print "ALERT! Cannot open: " + file_name + ". File does not exists."
        return False

def read_world(file_name = world):
    """ Read the pellet specs in the world file and return a pellet matrix """
    try:
        in_file = open(file_name, "r")
        data = {}
        # Skip 1 lines
        in_file.readline()

        # Read Dimensions
        data['dims'] = []
        for i in range(3):
            line = in_file.readline()
            info = line.split("\t")
            data['dims'].append(int(info[1]))
        data['dims'] = tuple(data['dims'])
        
        # Skip 3 lines
        for i in range(3):
            line = in_file.readline()

        # Read Pellets
        data['pellets'] = []
        for line in in_file:
            info = line.split("\t")
            reward = info[0]
            mode = info[1]
            other_info = info[2:len(info)-1]
            pellet = Pellet(reward, mode, other_info)
            data['pellets'].append(pellet)            
        in_file.close()
        return data
    except IOError:
        print "ALERT! Cannot open: " + file_name + ". File does not exists."
        return False
    
def pattern_random(file_name = world, size_x = 100, size_y = 100, offset_x = 0, offset_y = 0):
    data = read_world(file_name)
    if (data != None):
        crumbs = {}
        for info in data['pellets']:
            pellet_x, pellet_y = info[0], info[1]
            pellet_pres, pellet_rew = info[2], info[3]
            if (pellet_x == "random"):
                pellet_x = random.random()*size_x + offset_x
            if (pellet_y == "random"):
                pellet_y = random.random()*size_y + offset_y
            location = (float(pellet_x), float(pellet_y))
            stats = (int(pellet_pres), int(pellet_rew))
            crumbs[location] = stats
        return crumbs
   
def pattern_cluster(num_pellets, file_name = world):
    sizes = clear_map(file_name)
    size_x, size_y = int(sizes["X_DIM"]), int(sizes["Y_DIM"])

    mode = "cluster"
    reward = 1
    centers = []
    for i in range(10):
        center_x = random.random()*size_x
        center_y = random.random()*size_y
        spread_x = random.random()*30
        #spread_y = random.random()*30
        if (spread_x + center_x > size_x):
            spread_x = size_x - center_x
        spread_y = spread_x
        centers.append((center_x, center_y, spread_x, spread_y))
                
    list_pellet = []
    for i in range(num_pellets):
        j = random.randint(0, len(centers)-1)
        info = centers[j]
        pellet = Pellet(reward, mode, info)
        list_pellet.append(pellet)
    
    write_pellets(list_pellet, file_name)    
    return list_pellet

def main():
    clear_map()
    list_pellet = pattern_cluster(200)
    for pellet in list_pellet:
        add_pellet(pellet)
    #add_random_pellets(500, 'world_config.txt')
    #list_crumb = read_pellets()
    #print list_crumb 

#main()
