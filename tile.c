//
//  tile.c
//  Sudoku
//
//  Created by Toby on 2018-05-28.
//  Copyright © 2018 Toby. All rights reserved.
//

#include "tile.h"
#include <stdlib.h>
#define allOnes 511 // (1 << 9) - 1 i.e. 9 consecutive 1's

// create a new tile with cand set to all ones
Tile *newTile(){
    Tile *tile = malloc(sizeof(Tile));
    tile->cand = allOnes;
    tile->num = 0;
    return tile;
}

// set the number of this tile
void setNum(Tile *tile, int num){
    tile->num = num;
}

// remove a candidate number
void rmNum(Tile *tile, int num){
    tile->cand &= allOnes - (1 << (num - 1)); // set num bit to 0
}

// check if num is set (i.e. is a candidate)
int checkNum(Tile *tile, int num){
    return tile->cand & (1 << (num - 1));
}

// get the block number of this tile
int getBlock(int r, int c){
    return c / 3 + r / 3 * 3;
}

// get the index of this tile in its block
int getIndex(int r, int c){
    return c % 3 + r % 3 * 3;
}
