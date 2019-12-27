#ifndef board_H
#define board_H


#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#define BOARD_COLUMNS 4
#define BOARD_ROWS 4

// Likelihood of a '4' appearing on the board represented as a percentage
// multiplied by 100.
#define BOARD_4SPAWN_CHANCE 15

#define WIN_NUMBER  2048
/**
 * Represents the in-game board.
 */
struct board {
	// Holds the value of each tile.
	unsigned tiles[BOARD_ROWS][BOARD_COLUMNS];
};


/**
 *  0: game is not over; 
 * >0: game is over and the user has won
 * <0: game is over and the user has lose.
 */
int board_done(struct board* board);

/**
 * Returns the number of empty tiles in the board.
 */
unsigned board_get_tiles_empty(struct board* board);

/**
 * Initializes the specified board.
 */
void board_init(struct board* board);

/**
 * Merge tiles in the board downwards.
 */
int board_merge_down(struct board* board);

/**
 * Merge tiles in the board leftwards.
 */
int board_merge_left(struct board* board);

/**
 * Merge tiles in the board rightwards.
 */
int board_merge_right(struct board* board);

/**
 * Merge tiles in the board upwards.
 */
int board_merge_up(struct board* board);

/**
 * Processes user move-down request.
 */
int board_move_down(struct board* board);

/**
 * Process user move-left request.
 */
int board_move_left(struct board* board);

/**
 * Process user move-right request.
 */
int board_move_right(struct board* board);

/**
 * Process user move-up request.
 */
int board_move_up(struct board* board);

/**
 * Spawn a new tile on the board.
 */
void board_plop(struct board* board);

/**
 * Print the board to 'stdout'.
 */
void board_print(struct board* board);

/**
 * Shift all the elements in the board down.
 */
int board_shift_down(struct board* board);

/**
 * Shift all the elements in the board left.
 */
int board_shift_left(struct board* board);

/**
 * Shift all the elements in the board right.
 */
int board_shift_right(struct board* board);

/**
 * Shift all the elements in the board up.
 */
int board_shift_up(struct board* board);


#endif // board_H
