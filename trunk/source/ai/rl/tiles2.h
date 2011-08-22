#ifndef _TILES2_H_
#define _TILES2_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

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
	int tiles[],               ///< provided array contains returned tiles (tile indices)
	int num_tilings,           ///< number of tile indices to be returned in tiles       
    int memory_size,           ///< total number of possible tiles
	float floats[],            ///< array of floating point variables
    int num_floats,            ///< number of floating point variables
    int ints[],				   ///< array of integer variables
    int num_ints);             ///< number of integer variables

/// hash collision table
class collision_table {
public:
    collision_table(int,int);
    ~collision_table();
    long m;
    long *data;
    int safe;
    long calls;
    long clearhits;
    long collisions;
    void reset();
    int usage();
};

	
void GetTiles(
	int tiles[],               ///< provided array contains returned tiles (tile indices)
	int num_tilings,           ///< number of tile indices to be returned in tiles       
    collision_table *ctable,   ///< total number of possible tiles
	float floats[],            ///< array of floating point variables
    int num_floats,            ///< number of floating point variables
    int ints[],				   ///< array of integer variables
    int num_ints);             ///< number of integer variables

int hash_UNH(int *ints, int num_ints, long m, int increment);
int hashfunction(int *ints, int num_ints, collision_table *ctable);

// no ints
void GetTiles(int tiles[],int nt,int memory,float floats[],int nf);
void GetTiles(int tiles[],int nt,collision_table *ct,float floats[],int nf);

// one int
void GetTiles(int tiles[],int nt,int memory,float floats[],int nf,int h1);
void GetTiles(int tiles[],int nt,collision_table *ct,float floats[],int nf,int h1);

// two ints
void GetTiles(int tiles[],int nt,int memory,float floats[],int nf,int h1,int h2);
void GetTiles(int tiles[],int nt,collision_table *ct,float floats[],int nf,int h1,int h2);

// three ints
void GetTiles(int tiles[],int nt,int memory,float floats[],int nf,int h1,int h2,int h3);
void GetTiles(int tiles[],int nt,collision_table *ct,float floats[],int nf,int h1,int h2,int h3);

// one float, no ints
void GetTiles1(int tiles[],int nt,int memory,float f1);
void GetTiles1(int tiles[],int nt,collision_table *ct,float f1);

// one float, one int
void GetTiles1(int tiles[],int nt,int memory,float f1,int h1);
void GetTiles1(int tiles[],int nt,collision_table *ct,float f1,int h1);

// one float, two ints
void GetTiles1(int tiles[],int nt,int memory,float f1,int h1,int h2);
void GetTiles1(int tiles[],int nt,collision_table *ct,float f1,int h1,int h2);

// one float, three ints
void GetTiles1(int tiles[],int nt,int memory,float f1,int h1,int h2,int h3);
void GetTiles1(int tiles[],int nt,collision_table *ct,float f1,int h1,int h2,int h3);

// two floats, no ints
void GetTiles2(int tiles[],int nt,int memory,float f1,float f2);
void GetTiles2(int tiles[],int nt,collision_table *ct,float f1,float f2);

// two floats, one int
void GetTiles2(int tiles[],int nt,int memory,float f1,float f2,int h1);
void GetTiles2(int tiles[],int nt,collision_table *ct,float f1,float f2,int h1);

// two floats, two ints
void GetTiles2(int tiles[],int nt,int memory,float f1,float f2,int h1,int h2);
void GetTiles2(int tiles[],int nt,collision_table *ct,float f1,float f2,int h1,int h2);

// two floats, three ints
void GetTiles2(int tiles[],int nt,int memory,float f1,float f2,int h1,int h2,int h3);
void GetTiles2(int tiles[],int nt,collision_table *ct,float f1,float f2,int h1,int h2,int h3);

void GetTilesWrap(
	int tiles[],               ///< provided array contains returned tiles (tile indices)
	int num_tilings,           ///< number of tile indices to be returned in tiles       
    int memory_size,           ///< total number of possible tiles
	float floats[],            ///< array of floating point variables
    int num_floats,            ///< number of floating point variables
    int wrap_widths[],         ///< array of widths (length and units as in floats)
    int ints[],				   ///< array of integer variables
    int num_ints);             ///< number of integer variables
	
void GetTilesWrap(
	int tiles[],               ///< provided array contains returned tiles (tile indices)
	int num_tilings,           ///< number of tile indices to be returned in tiles       
    collision_table *ctable,   ///< total number of possible tiles
	float floats[],            ///< array of floating point variables
    int num_floats,            ///< number of floating point variables
    int wrap_widths[],         ///< array of widths (length and units as in floats)
    int ints[],				   ///< array of integer variables
    int num_ints);             ///< number of integer variables

// no ints
void GetTilesWrap(int tiles[],int num_tilings,int memory_size,float floats[],       
    int num_floats,int wrap_widths[]);           

#endif

