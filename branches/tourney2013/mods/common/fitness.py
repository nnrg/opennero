import random

def is_number(x):
    return type(x) in (int, float, long, complex)

class Fitness:
    """
    Multi-objective fitness
    """
    STAND_GROUND = "Stand ground"
    STICK_TOGETHER = "Stick together"
    APPROACH_ENEMY = "Approach enemy"
    APPROACH_FLAG = "Approach flag"
    HIT_TARGET = "Hit target"
    AVOID_FIRE = "Avoid fire"
    dimensions = [STAND_GROUND, STICK_TOGETHER, APPROACH_ENEMY, APPROACH_FLAG, HIT_TARGET, AVOID_FIRE]
    def __init__(self):
        self.data = {}
        for d in Fitness.dimensions:
            self.data[d] = 0
    def random(self):
        for d in Fitness.dimensions:
            self.data[d] = random.random()
    def randint(self, a, b):
        for d in Fitness.dimensions:
            self.data[d] = float(random.randint(a,b))
    def __repr__(self): return repr(self.data)
    def __str__(self): return '\t'.join([str(self.data[k]) for k in Fitness.dimensions])
    def __len__(self): return self.data
    def __getitem__(self, key): return self.data[key]
    def __setitem__(self, key, value): self.data[key] = value
    def __contains__(self, item): return item in self.data
    def __iter__(self): return [self.data[d] for d in Fitness.dimensions].__iter__()
    def sum(self): return sum(self.data.values())
    def __add__(self, other):
        result = Fitness()
        if is_number(other):
            for d in Fitness.dimensions:
                result[d] = self[d] + other
        else:
            for d in Fitness.dimensions:
                result[d] = self[d] + other[d]
        return result
    def __sub__(self, other):
        result = Fitness()
        if is_number(other):
            for d in Fitness.dimensions:
                result[d] = self[d] - other
        else:
            for d in Fitness.dimensions:
                result[d] = self[d] - other[d]
        return result
    def __mul__(self, other):
        result = Fitness()
        if is_number(other):
            for d in Fitness.dimensions:
                result[d] = self[d] * other
        else:
            for d in Fitness.dimensions:
                result[d] = self[d] * other[d]
        return result
    def __div__(self, other):
        result = Fitness()
        if is_number(other):
            for d in Fitness.dimensions:
                result[d] = self[d] / other
        else:
            for d in Fitness.dimensions:
                if other[d] != 0:
                    result[d] = self[d] / other[d]
        return result
    def __pow__(self, other):
        result = Fitness()
        if is_number(other):
            for d in Fitness.dimensions:
                result[d] = self[d] ** other
        else:
            for d in Fitness.dimensions:
                result[d] = self[d] ** other[d]
        return result
    def __abs__(self):
        result = Fitness()
        for d in Fitness.dimensions:
            result[d] = abs(self[d])
        return result

class FitnessStats:
    """
    Online fitness mean and variance calculator
    """
    def __init__(self):
        self.n = 0 # current number of samples
        self.mean = Fitness() # current mean estimate
        self.nvar = Fitness() # sum of squares of differences from the (current) mean
    def add(self, f):
        self.n += 1 # update n
        delta = f - self.mean
        self.mean += delta/self.n # update mean
        self.nvar += delta * (f - self.mean) # use new value of mean
    def var(self):
        return self.nvar / self.n
    def stddev(self):
        return self.var() ** 0.5
    def zscore(self, f):
        return sum( (f - self.mean)/self.stddev() )

if __name__ == "__main__":
    flist = []
    stats = FitnessStats()
    for i in range(10):
        x = Fitness()
        x.randint(-5,5)
        flist.append(x)
        stats.add(x)
        print x
    for f in flist:
        print stats.zscore(f)
    print 'n:', stats.n
    print 'mean:', stats.mean
    print 'var:', stats.var()
    print 'stddev:', stats.stddev()
