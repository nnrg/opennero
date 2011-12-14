/**
@file
CMAC tile coding implementation from Rich Sutton

External documentation and recommendations on the use of this code is
available at http://rlai.net.
 
This is an implementation of grid-style tile codings, based originally on 
the UNH CMAC code (see http://www.ece.unh.edu/robots/cmac.htm ). 
Here we provide a procedure, "GetTiles", that maps floating-point and integer
variables to a list of tiles. This function is memoryless and requires no
setup. We assume that hashing colisions are to be ignored. There may be
duplicates in the list of tiles, but this is unlikely if memory-size is
large. 
 
The floating-point input variables will be gridded at unit intervals, so generalization
will be by 1 in each direction, and any scaling will have 
to be done externally before calling tiles.  There is no generalization
across integer values.
 
It is recommended by the UNH folks that num-tilings be a power of 2, e.g., 16. 
 
We assume the existence of a function "rand()" that produces successive
random integers, of which we use only the low-order bytes.
*/

#include <iostream>
#include "tiles2.h"
#include "stdlib.h"
#include "math.h"

/// n mod k
int mod(int n, int k) {return (n >= 0) ? n%k : k-1-((-n-1)%k);}

void GetTiles(
    std::vector<int>& tiles,          ///< provided array contains returned tiles (tile indices)
    int memory_size,                  ///< total number of possible tiles
    const std::vector<float>& floats, ///< array of floating point variables
    const std::vector<int>& ints)     ///< array of integer variables
{
    int i,j;
    int qstate[MAX_NUM_VARS];
    int base[MAX_NUM_VARS];
    std::vector<int> coordinates;
    coordinates.resize(floats.size() + ints.size() + 1);

    for (int i=0; i<ints.size(); i++)
        coordinates[floats.size()+1+i] = ints[i];

    /* quantize state to integers (henceforth, tile widths == tiles.size()) */
    for (i = 0; i < floats.size(); i++)
    {
        qstate[i] = (int) floor(floats[i] * tiles.size());
        base[i] = 0;
    }

    /*compute the tile numbers */
    for (j = 0; j < tiles.size(); j++)
    {

        /* loop over each relevant dimension */
        for (i = 0; i < floats.size(); i++)
        {

            /* find coordinates of activated tile in tiling space */
            coordinates[i] = qstate[i] - mod(qstate[i]-base[i], tiles.size());

            /* compute displacement of next tiling in quantized space */
            base[i] += 1 + (2 * i);
        }
        /* add additional indices for tiling and hashing_set so they hash differently */
        coordinates[i] = j;

        tiles[j] = hash_UNH(coordinates, memory_size, 449);
    }
    return;
}


/// Takes an array of integers and returns the corresponding tile after hashing 
int hash_UNH(const std::vector<int>& ints, long m, int increment)
{
    static unsigned int rndseq[2048];
    static int first_call =  1;
    int i,k;
    long index;
    long sum = 0;

    /* if first call to hashing, initialize table of random numbers */
    if (first_call)
    {
        for (k = 0; k < 2048; k++)
        {
            rndseq[k] = 0;
            for (i=0; i < (int)sizeof(int); ++i)
                rndseq[k] = (rndseq[k] << 8) | (rand() & 0xff);
        }
        first_call = 0;
    }

    for (i = 0; i < ints.size(); i++)
    {
        /* add random table offset for this dimension and wrap around */
        index = ints[i];
        index += (increment * i);
        index %= 2048;
        while (index < 0)
            index += 2048;

        /* add selected random number to sum */
        sum += (long)rndseq[(int)index];
    }
    index = (int)(sum % m);
    while (index < 0)
        index += m;

    return(index);
}
