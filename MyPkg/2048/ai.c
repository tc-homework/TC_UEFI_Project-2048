#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <Library/UefiBootServicesTableLib.h>

#include "board.h"

unsigned count = 0;

/* Weight map of cells */
static const unsigned cellWeight[4][4] = {
    {10,  9,  8,  8},
    {11,  9,  8,  8},
    {13, 10,  9,  9},
    {17, 13, 11, 10}
};


unsigned Try(struct board board, char dert) {
    unsigned afterWeight = 0;
    int v;
    if (dert == 'w'){
        v = board_move_up(&board);
    } else if (dert == 's'){
        v = board_move_down(&board);
    } else if (dert == 'a'){
        v = board_move_left(&board);
    } else if (dert == 'd'){
        v = board_move_right(&board);
    }
    if(v == 0) {
        //Print(L"%c%d|", dert, 0);
        return 0;
    }
    unsigned i, j;
    for(i = 0; i < BOARD_ROWS; i++) {
        for(j = 0; j < BOARD_COLUMNS; j++) {
            afterWeight += (cellWeight[i][j] * board.tiles[i][j]);
        }
    }
    //Print(L"%c%d|", dert, afterWeight);
    return afterWeight;
}

char GetAINextMove(struct board board) {
    gBS->Stall(300000);
    unsigned wScore = Try(board, 'w');
    unsigned sScore = Try(board, 's');
    unsigned aScore = Try(board, 'a');
    unsigned dScore = Try(board, 'd');
    // if (wScore >= sScore && wScore >= dScore && wScore >= aScore &&  1 ) {
    //     return 'w';
    // } if (sScore >= wScore && sScore >= dScore && sScore >= aScore &&  1 ) {
    //     return 's';
    // } if (dScore >= wScore && dScore >= sScore && dScore >= aScore &&  1 ) {
    //     return 'd';
    // } if (aScore >= wScore && aScore >= sScore && aScore >= dScore &&  1 ) {
    //     return 'a';
    // } else {
    //     return 's';
    // }
    if(sScore != 0){
        return 's';
    } else if (dScore != 0) { 
        return 'd';
    } else if (aScore != 0) { 
        return 'a';
    } else if (wScore != 0) { 
        return 'w';
    } else { 
        return 'q';
    }
}


