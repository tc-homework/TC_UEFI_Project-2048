#define X_BASE_POSITION 0
#define Y_BASE_POSITION 1

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "board.h"
#include "gui.c"
#include "ai.c"



int main (int argc, char* argv[]) {
    struct board board;
    int status; // Store the game status.
    int valid;
    EFI_STATUS EStatus = EFI_SUCCESS;
    EFI_INPUT_KEY key;
    char operation;
    UINTN Index;	// Key
    UINTN move = 0; // The counter of move times
    unsigned autoMove = 0;

    if(argc == 2 && argv[1][1] == 'a') {
        autoMove = 1;
    }


    InitGUI();
    InitBackground();
    UpdateMoveGUI(move);
    //gBS->WaitForEvent(1,&(gSimpleTextInputEx->WaitForKeyEx),&Index);


    // Set up board.
    board_init(&board);


    
    // Start the game.
    status = board_done(&board);

    while (!(status)) {
		
        // Print the board.
        UpdateBoardGUI(&board);

        // Get the player's move.
        valid = 0;
        
        if(autoMove == 1) {
            operation = GetAINextMove(board);
        } else {
            // Wait for key press
            EStatus = gBS->WaitForEvent(
                1, 
                &(gST->ConIn->WaitForKey), 
                &Index
		    );
            EStatus = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
            operation = (key.UnicodeChar == L'w'|| key.ScanCode == 0x01) 
                ? 'w' : (key.UnicodeChar == L's'|| key.ScanCode == 0x02)
                ? 's' : (key.UnicodeChar == L'd'|| key.ScanCode == 0x03)
                ? 'd' : (key.UnicodeChar == L'a'|| key.ScanCode == 0x04)
                ? 'a' : (key.UnicodeChar == L'q') ? 'q' : 't';
        }
        if (operation == 'w'){
            valid = board_move_up(&board);
        } else if (operation == 's'){
            valid = board_move_down(&board);
        } else if (operation == 'a'){
            valid = board_move_left(&board);
        } else if (operation == 'd'){
            valid = board_move_right(&board);
        } else if (operation == 'q'){
            break;
        } else {
            continue;
        }

        UpdateValidGUI(valid);
        // Prepare for user's next move.
        if (valid) {
            UpdateMoveGUI(++move);
            board_plop(&board);
        }

        status = autoMove ? 0 : board_done(&board);

    }

    FinallyDisplay(autoMove, status) ;
    gBS->WaitForEvent(1, &(gSimpleTextInputEx->WaitForKeyEx),&Index);
    DestoryGUI();

    return EStatus;
 }
