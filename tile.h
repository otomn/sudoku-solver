//
//  tile.h
//  Sudoku
//
//  Created by Toby on 2018-05-28.
//  Copyright © 2018 Toby. All rights reserved.
//

#ifndef bit_h
#define bit_h

#include <stdio.h>

#endif /* bit_h */

// a struct that stores information about a tile on the sudoku table
struct tile{
    short num; // the number of this tile if known, 0 if unkown
    short cand; // bit representation of the possible numbers (candidates)
    int r; //row
    int c; //column
};

typedef struct tile Tile;

Tile *newTile(void);
void setNum(Tile *, int);
void rmNum(Tile *, int);
int checkNum(Tile *, int);
int getBlock(int, int);
int getIndex(int, int);
