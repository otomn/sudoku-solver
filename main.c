//
//  main.c
//  Sudoku
//
//  Created by Toby on 2018-05-28.
//  Copyright © 2018 Toby. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tile.h"
#define REC_MAX_LVL 3

Tile ***rows;
Tile ***cols;
Tile ***blcs; // blocks
Tile ****sws; // sort ways
int known; // known tiles
int lastKnown; // knwon tiles in the last cycle

// make a sudoku table
void makeTable(){
    sws = malloc(sizeof(Tile ***) * 3);
    // make a 9 * 9 reference table
    for (int b = 0; b < 3; b++){
        sws[b] = malloc(sizeof(Tile **) * 9);
        for (int i = 0; i < 9; i++){
            sws[b][i] = malloc(sizeof(Tile *) * 9);
        }
    }
    rows = sws[0];
    cols = sws[1];
    blcs = sws[2];
    
    // make 81 tiles and set up the references
    for (int r = 0; r < 9; r++){
        for (int c = 0; c < 9; c++){
            rows[r][c] = newTile();
            rows[r][c]->r = r;
            rows[r][c]->c = c;
            cols[c][r] = rows[r][c];
            blcs[getBlock(r, c)][getIndex(r, c)] = rows[r][c];
        }
    }
}

//print table without candidate numbers
void printTable(){
    printf("printing table\n");
    for (int r = 0; r < 9; r++){
        for (int c = 0; c < 9; c++){
            if (rows[r][c]->num == 0){
                printf("  ");
            } else {
                printf("%d ", rows[r][c]->num);
            }
            if (c == 2 || c == 5) printf("| ");
        }
        if (r == 2 || r == 5) printf("\n- - - | - - - | - - -");
        printf("\n");
    }
}

// print i if i is a candidate for tile, else print space
void printCand(Tile *tile, int num){
    if (checkNum(tile, num)) {
        printf("%d", num);
    } else {
        printf(" ");
    }
}

//print table with candidate numbers
void printTableDetailed(){
    printf("printing table\n");
    for (int r = 0; r < 9; r++){
        // print 1st subrow of this row
        for (int c = 0; c < 9; c++){
            if (rows[r][c]->num != 0){
                printf("*** ");
            } else {
                printCand(rows[r][c], 1);
                printCand(rows[r][c], 2);
                printCand(rows[r][c], 3);
                printf(" ");
            }
            if (c == 2 || c == 5) printf("| ");
        }
        printf("\n");
        // print 2nd subrow of this row
        for (int c = 0; c < 9; c++){
            if (rows[r][c]->num != 0){
                printf("*%d* ", rows[r][c]->num);
            } else {
                printCand(rows[r][c], 4);
                printCand(rows[r][c], 5);
                printCand(rows[r][c], 6);
                printf(" ");
            }
            if (c == 2 || c == 5) printf("| ");
        }
        printf("\n");
        // print 3rd subrow of this row
        for (int c = 0; c < 9; c++){
            if (rows[r][c]->num != 0){
                printf("*** ");
            } else {
                printCand(rows[r][c], 7);
                printCand(rows[r][c], 8);
                printCand(rows[r][c], 9);
                printf(" ");
            }
            if (c == 2 || c == 5) printf("| ");
        }
        printf("\n");
        if (r == 2 || r == 5)
            printf("--- --- --- | --- --- --- | --- --- ---\n");
    }
}

// fill in sudoku table data from input
void readTable(){
    for (int r = 0; r < 9; r++){
        for (int c = 0; c < 9; c++){
            scanf("%hd", &rows[r][c]->num);
        }
    }
    printTable();
}

// used when a tile's number is known
// removes that number from the candidates of all tiles in the same row/col/block
void clearCand(int r, int c){
    known++;
    int num = rows[r][c]->num;
    rows[r][c]->cand = 0;
    for (int i = 0; i < 9; i++){
        rmNum(rows[r][i], num);
        rmNum(cols[c][i], num);
        rmNum(blcs[getBlock(r, c)][i], num);
    }
}

// begin strategies

// if there is only one candidate, lock in that number
void checkTile(){
    for (int r = 0; r < 9; r++){
        for (int c = 0; c < 9; c++){
            for (int n = 0; n < 9; n++){
                if (rows[r][c]->cand == 1 << n){
                    rows[r][c]->num = n + 1;
                    clearCand(r, c);
                }
            }
        }
    }
}

// check fot the only empty tile in a section (a row/col/block)
void checkSection(){
    for (int n = 1; n <= 9; n++){ //number
        for (int s = 0; s < 9; s++){ //section
            int row = 0, col = 0;
            for (int sw = 0; sw < 3; sw++){ //sort way
                int count = 0;
                for (int i = 0; i < 9; i++){
                    Tile *tile = sws[sw][s][i];
                    if (checkNum(tile, n)) {
                        row = tile->r; //set last seen location
                        col = tile->c;
                        count++;
                    }
                }
                if (count == 1){ //only one is seen in the section
                    rows[row][col]->num = n;
                    clearCand(row, col);
                }
            }
        }
    }
}

// if n only shows up as candidate on line l in block b, it cannot be on the
// same line in other blocks
void pointing(){
    for (int n = 1; n <= 9; n++){
        for (int b = 0; b < 9; b++){
            int row = 9, col = 9;
            for (int i = 0; i < 9; i++){
                Tile *tile = blcs[b][i];
                if (checkNum(tile, n)){
                    if (row == 9) { //initial state
                        row = tile->r;
                    } else if (row != tile->r){
                        row = 10; // state for number n shows up on multiple lines
                    }
                    if (col == 9) {
                        col = tile->c;
                    } else if (col != tile->c){
                        col = 10;
                    }
                }
            }
            if (row < 9){
                // number n shows up only on this row
                // remove n from candidates in all tiles on the same row that
                // are in different blocks
                for (int j = 0; j < 9; j++){
                    if (getBlock(row, j) != b) rmNum(rows[row][j], n);
                }
            }
            if (col < 9){
                for (int j = 0; j < 9; j++){
                    if (getBlock(j, col) != b) rmNum(cols[col][j], n);
                }
            }
        }
    }
}

// end strategies

// 0 if two tiles in the same section has the same number
// or a tile has no number and no candidate
// 1 if is valid
// 2 if all tiles have a number
int validateSw(Tile ***sw){
    int result = 2;
    Tile *log; // log for each subsection
    int num;
    for (int s = 0; s < 9; s++){
        log = newTile();
        for (int i = 0; i < 9; i++){
            if ((num = sw[s][i]->num)){
                // tile is set
                if (checkNum(log, num)){
                    rmNum(log, num);
                } else {
                    return 0; // same number appeared before
                }
            } else {
                // check candidates
                if (!sw[s][i]->cand) return 0;
            }
        }
        if (log->cand) result = 1;
    }
    return result;
}

// 0 if two tiles on the same row/col/block has the same number
// or a tile has no number and no candidate
// 1 if is valid
// 2 if all tiles have a number
int validate(){
    int result = 2;
    for (int i = 0; i < 3; i++){
        if (validateSw(sws[i]) == 0) return 0;
        if (validateSw(sws[i]) == 1) result = 1;
    }
    return result;
}

int tryOnError(int level);
void solve(void);

void tryOnErrorChild(int level){
    solve();
    int code = validate();
    
    if (code == 0) exit(0);
    if (code == 2) exit(1);
    if (level == REC_MAX_LVL) exit(2);
    code = tryOnError(level + 1);
    exit(code);
}

// ultimate solver that picks a candidate for an unknown tile
// 0 if no solution
// 1 if has a solution
// 2 no solution is found within max depth
int tryOnError(int level){
    int code = validate();
    if (code == 0) return 0;
    if (code == 2) return 1;
    Tile *tile = NULL;
    for (int r = 0; r < 9; r++){
        for (int c =0; c < 9; c++){
            tile = rows[r][c];
            if (!tile->num) {
                r = 9;
                break;
            }
        }
    }
    int pid = 0;
    int count = 0;
    int *pids = malloc(sizeof(int) * 9);
    for (int n = 1; n <= 9; n++){
        if (checkNum(tile, n)){
            pid = fork();
            count++;
            if (pid) {
                pids[n - 1] = pid;
            } else {
                tile->num = n;
                clearCand(tile->r, tile->c);
                tryOnErrorChild(level);
            }
        }
    }
    int status;
    for (int i = 0; i < count; i++){
        pid = wait(&status);
        if (pid == -1) {
            fprintf(stderr, "%d Error in wait\n", i);
            exit(1);
        }
        int num = 0;
        for (int n = 0; n < 9; n++){
            if (pids[n] == pid){
                num = n + 1;
                break;
            }
        }
        if (!WIFEXITED(status)) {
            fprintf(stderr, "%d exited abnormally\n", pid);
            exit(1);
        }
        if (WEXITSTATUS(status) == 1) {
            tile->num = num;
            clearCand(tile->r, tile->c);
            solve();
            return tryOnError(level);
        }
        if (WEXITSTATUS(status) == 0) {
            rmNum(tile, num);
        }
    }
    if (tile->cand) return 2; //has unremoved candidates
    return 0; // no candidate
}

// solve the sudoku with given data
void solve(){
    while (known < 81 && known > lastKnown){
        lastKnown = known;
        
        // begin strategies
        checkTile();
        checkSection();
        pointing();
        // end strategies
        
//        printTable();
    }
}

int main(int argc, const char * argv[]) {
    known = 0;
    lastKnown = 0;
    makeTable();
    readTable();
    for (int i = 0; i < 9; i++){
        for (int j = 0; j < 9; j++){
            if (rows[i][j]->num) clearCand(i, j);
        }
    }
    solve();
    printTableDetailed();
    int code = tryOnError(0);
    printTableDetailed();
    printTable();
    switch (code) {
        case 0:
            printf("sudoku has no solution\n");
            break;
        case 1:
            printf("sudoku solved\n");
            break;
        case 2:
            printf("no solution is found within max depth\n");
            break;
        default:
            break;
    }
    return 0;
}
