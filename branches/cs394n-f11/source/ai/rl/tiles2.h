#ifndef _TILES2_H_
#define _TILES2_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>

/**
 * @file tiles2.h
 * @brief Rich Sutton's tile coding software
 * 
 * <a href="http://www.cs.ualberta.ca/~sutton/">Rich Sutton</a> CMAC/tile coding software. More details
 * can be found <a href="http://www.cs.ualberta.ca/~sutton/tiles2.html">here</a>.
 */

#define MAX_NUM_VARS 20        ///< Maximum number of variables in a grid-tiling
#define MAX_NUM_COORDS 100     ///< Maximum number of hashing coordinates      
#define MaxLONGINT 2147483647  ///< Maximum long integer

void GetTiles(
    std::vector<int>& tiles,          ///< provided array contains returned tiles (tile indices)
    int memory_size,                  ///< total number of possible tiles
	const std::vector<float>& floats, ///< array of floating point variables
    const std::vector<int>& ints);    ///< array of integer variables

int hash_UNH(const std::vector<int>& ints, long m, int increment);

#endif

