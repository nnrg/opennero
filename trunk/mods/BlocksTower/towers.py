def On(A, B):
    return ("On", A, B)

def Clear(A):
    return ("Clear", A)

def Smaller(A, B):
    return ("Smaller", A, B)

class Towers(object):

    Pole1 = 'Pole1'
    Pole2 = 'Pole2'
    Pole3 = 'Pole3'

    POLES = ['Pole1', 'Pole2', 'Pole3']

    @classmethod
    def Move(cls, STATE, Disk, Source, Dest):
        if Clear(Disk) in STATE and On(Disk, Source) in STATE and Clear(Dest) in STATE and Smaller(Disk, Dest) in STATE:
            STATE.add( On(Disk, Dest) )
            STATE.remove( On(Disk, Source) )
            STATE.remove( Clear( Dest ) )
            STATE.add( Clear( Source ) )
            return True
        else:
            return False
    
    @classmethod
    def UnMove(cls, STATE, Disk, Source, Dest):
        if On(Disk, Dest) in STATE and On(Disk, Source) not in STATE and Clear(Dest) not in STATE and Clear(Source) in STATE:
            STATE.remove( On(Disk, Dest) )
            STATE.add( On(Disk, Source) )
            STATE.add( Clear( Dest ) )
            STATE.remove( Clear( Source ) )
            return True
        else:
            return False
    
    # actions is just a list of pairs of functions to do or undo an action
    # in general we could make things general and check for function arity
    # but currently the code only works with Disk, Source, Dest
#    ACTIONS = [ (Move, UnMove) ]
    @classmethod
    def get_actions(cls):
        return [ (cls.Move, cls.UnMove) ]
    
    @classmethod
    def get_pole(cls, state, disk):
        """ get the pole of the disk given the state """
        if disk in cls.POLES:
            return disk
        for p in state:
            if p[0] == 'On' and p[1] == disk:
                if p[2] in cls.POLES:
                    return p[2]
                else:
                    return cls.get_pole(state - set([p]), p[2])
        return None

    # action primitives
    # move without getting stuff
    MOVES = { \
        (Pole1, Pole2): [4, 1, 5], 
        (Pole1, Pole3): [4, 1, 1, 5], 
        (Pole2, Pole1): [5, 1, 4], 
        (Pole2, Pole3): [4, 1, 5], 
        (Pole3, Pole1): [5, 1, 1, 4], 
        (Pole3, Pole2): [5, 1, 4] 
    }
    
    # move with pick up and put down
    CARRY_MOVES = {}
    for (source, dest) in MOVES:
        CARRY_MOVES[(source, dest)] = [3] + MOVES[(source, dest)] + [2]

class Towers2(Towers):

    Disk1 = 'Disk1'
    Disk2 = 'Disk2'

    Pole1 = 'Pole1'
    Pole2 = 'Pole2'
    Pole3 = 'Pole3'
    
    DISKS = ['Disk1', 'Disk2']
    POLES = ['Pole1', 'Pole2', 'Pole3']
    LITERALS = [Disk1, Disk2, Pole1, Pole2, Pole3]
    
    INIT = set([
        Clear(Disk1),
        On(Disk1, Disk2),
        On(Disk2, Pole1),
        Clear(Pole2),
        Clear(Pole3),
        Smaller(Disk1, Pole1),
        Smaller(Disk1, Pole2),
        Smaller(Disk1, Pole3),
        Smaller(Disk1, Disk2),
        Smaller(Disk2, Pole1),
        Smaller(Disk2, Pole2),
        Smaller(Disk2, Pole3),
    ])
    
    GOAL = set([
        On(Disk1, Disk2),
        On(Disk2, Pole3)
    ])
    

class Towers3(Towers):

    Disk1 = 'Disk1'
    Disk2 = 'Disk2'
    Disk3 = 'Disk3'

    Pole1 = 'Pole1'
    Pole2 = 'Pole2'
    Pole3 = 'Pole3'

    DISKS = ['Disk1', 'Disk2', 'Disk3']
    POLES = ['Pole1', 'Pole2', 'Pole3']
    LITERALS = [Disk1, Disk2, Disk3, Pole1, Pole2, Pole3]

    INIT = set([
        Clear(Disk1),
        On(Disk1, Disk2),
        On(Disk2, Disk3),
        On(Disk3, Pole1),
        Clear(Pole2),
        Clear(Pole3),
        Smaller(Disk1, Pole1),
        Smaller(Disk1, Pole2),
        Smaller(Disk1, Pole3),
        Smaller(Disk1, Disk2),
        Smaller(Disk1, Disk3),
        Smaller(Disk2, Pole1),
        Smaller(Disk2, Pole2),
        Smaller(Disk2, Pole3),
        Smaller(Disk2, Disk3),
        Smaller(Disk3, Pole1),
        Smaller(Disk3, Pole2),
        Smaller(Disk3, Pole3),
    ])
    
    GOAL = set([
        On(Disk1, Disk2),
        On(Disk2, Disk3),
        On(Disk3, Pole3)
    ])

