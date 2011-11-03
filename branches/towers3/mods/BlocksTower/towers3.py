Disk1 = 'Disk1'
Disk2 = 'Disk2'
Disk3 = 'Disk3'
Pole1 = 'Pole1'
Pole2 = 'Pole2'
Pole3 = 'Pole3'

DISKS = ['Disk1', 'Disk2', 'Disk3']
POLES = ['Pole1', 'Pole2', 'Pole3']
LITERALS = [Disk1, Disk2, Disk3, Pole1, Pole2, Pole3]

def On(A,B):
    return ("On", A, B)

def Clear(A):
    return ("Clear", A)

def Smaller(A, B):
    return ("Smaller", A, B)

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

# ACTIONS

def Move(STATE, Disk, Source, Dest):
    if Clear(Disk) in STATE and On(Disk, Source) in STATE and Clear(Dest) in STATE and Smaller(Disk, Dest) in STATE:
        STATE.add( On(Disk, Dest) )
        STATE.remove( On(Disk, Source) )
        STATE.remove( Clear( Dest ) )
        STATE.add( Clear( Source ) )
        return True
    else:
        return False

def UnMove(STATE, Disk, Source, Dest):
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
ACTIONS = [ (Move, UnMove) ]
