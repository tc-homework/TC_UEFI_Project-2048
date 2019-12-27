#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>

#include <Protocol/HiiFont.h>
#include <Protocol/HiiDatabase.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "board.h"

#define EFI_COLOR           EFI_GRAPHICS_OUTPUT_BLT_PIXEL
#define MARGIN_TOP          200
#define MARGIN_LEFT         275
#define TILE_WIDTH          100
#define GAP_WIDTH           15
#define TITLE_PADDING_LEFT  10
#define TITLE_PADDING_TOP   -60
#define TITLE_SIZE          50
#define MOVE_PADDING_LEFT   200
#define MOVE_PADDING_TOP    -60
#define MOVE_SIZE           20
#define VALID_PADDING_LEFT  200
#define VALID_PADDING_TOP   -100
#define VALID_SIZE          20
#define RADIUS              10
#define body_backgd         &(backgroundBuffer[0])
#define board_backgd        &(backgroundBuffer[1])
#define tile_backgd         &(backgroundBuffer[2])
#define font_color_gray     &(fontColorBuffer[0])
#define font_color_white    &(fontColorBuffer[1])

EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *gSimpleTextInputEx;
EFI_GRAPHICS_OUTPUT_PROTOCOL        *gGraphicsOutput;
EFI_HII_FONT_PROTOCOL               *gHiiFont;

EFI_COLOR backgroundBuffer[] = {
    239, 248, 250,   0,         // #faf8ef, body_backgd
    160, 173, 187,   0,         // #bbada0, board_backgd
    180, 193, 205,   0,         // #cdc1b4, tile_backgd
};

EFI_COLOR tileColorBuffer[] = {
    218, 228, 238,   0,         //#eee4da, 2
    200, 224, 237,   0,         //#ede0c8, 4
    121, 177, 242,   0,         //#f2b179, 8
     99, 149, 245,   0,         //#f59563, 16
     95, 124, 246,   0,         //#f67c5f, 32
     59,  94, 246,   0,         //#f65e3b, 64
    114, 207, 237,   0,         //#edcf72, 128
     80, 200, 237,   0,         //#edc850, 512
     63, 197, 237,   0,         //#edc53f, 1024
     46, 194, 237,   0,         //#edc22e, 2048
     50,  58,  60,   0          //#3c3a32, 4096+
};

EFI_COLOR fontColorBuffer[] = {
    101, 110, 119,   0,         //#776e65, grayfont /2, 4, title
    242, 246, 249,   0          //#f9f6f2, whitefont >4, score
};

struct board preBoard;
int preValid = 1;

EFI_STATUS LocateSimpleTextInputEx(void);
EFI_STATUS LocateGraphicsOutput(void);
EFI_STATUS SwitchGraphicsMode(BOOLEAN flag);
EFI_STATUS GetKeyEx(UINT16 *ScanCode, UINT16 *UniChar, UINT32 *ShiftState, EFI_KEY_TOGGLE_STATE * ToggleState);

EFI_COLOR *NumToTileColor(unsigned num);
EFI_COLOR *NumToFontColor(unsigned num);

VOID InitGUI();
VOID DestoryGUI();
VOID UpdateMoveGUI(UINTN move);
VOID SetMyMode(UINT32 ModeNumber);
VOID DrawPixel(UINTN x, UINTN y, EFI_COLOR *color);
VOID ColorTile(UINT16 x, UINT16 y, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *color);
VOID DrawRadius(UINTN x_0, UINTN y_0, EFI_COLOR *bcgdColor, unsigned radius, UINT8 isLeft, UINT8 isTop);
VOID DrawRoundedRectangle(UINTN x, UINTN y, UINTN w, UINTN h, EFI_COLOR *foreColor, EFI_COLOR *bkgdColor, unsigned radius);
VOID WriteStrOnPosition(CHAR16* str, UINTN x, UINTN y, EFI_COLOR fontColor, EFI_COLOR backgdColor, EFI_HII_FONT_STYLE fontStyle, UINT16 fontSize);

VOID InitGUI() {
    EFI_STATUS Status;
    Status=LocateSimpleTextInputEx();
    Status=LocateGraphicsOutput();
    SwitchGraphicsMode(TRUE);
    SetMyMode(0x3);
}

VOID DestoryGUI() {
    SetMyMode(0x0);
	SwitchGraphicsMode(FALSE);
}

int InitBackground() {
    UINT16 i, j;
    for(i = 0; i < BOARD_ROWS; i++) {
        for(j = 0; j < BOARD_COLUMNS; j++) {
            preBoard.tiles[i][j] = 0;
        }
    }
    gGraphicsOutput->Blt(gGraphicsOutput, body_backgd, EfiBltVideoFill, 0, 0, 0, 0, 1024, 768, 0);
    DrawRoundedRectangle(
        MARGIN_LEFT, MARGIN_TOP,  
        (TILE_WIDTH + GAP_WIDTH) * BOARD_COLUMNS + GAP_WIDTH,
        (TILE_WIDTH + GAP_WIDTH) * BOARD_ROWS + GAP_WIDTH,
        board_backgd, body_backgd, RADIUS
    );
    for(i = 0; i < BOARD_ROWS; i++) {
        for(j = 0; j < BOARD_COLUMNS; j++) {
            ColorTile(i, j, tile_backgd);
        }
    }
    WriteStrOnPosition(
        L"2048",
        MARGIN_LEFT + TITLE_PADDING_LEFT,
        MARGIN_TOP + TITLE_PADDING_TOP,
        *font_color_gray, 
        *body_backgd,
        EFI_HII_FONT_STYLE_BOLD,
        TITLE_SIZE);
    return 0;
}

VOID ColorTile(UINT16 i, UINT16 j, EFI_COLOR *color) {
    DrawRoundedRectangle(
        MARGIN_LEFT + (TILE_WIDTH + GAP_WIDTH) * j + GAP_WIDTH,
        MARGIN_TOP  + (TILE_WIDTH + GAP_WIDTH) * i + GAP_WIDTH,
        TILE_WIDTH, TILE_WIDTH,
        color, board_backgd, RADIUS
    );
}

VOID DrawPixel(UINTN x, UINTN y, EFI_COLOR *color) {
    gGraphicsOutput->Blt(
        gGraphicsOutput, color, EfiBltVideoFill, 0, 0, x, y, 1, 1, 0
    );
}

VOID DrawRadius(UINTN x_0, UINTN y_0, EFI_COLOR *bcgdColor, unsigned radius, UINT8 isLeft, UINT8 isTop) {
    UINTN dx, dy;
    for(dx = 0; dx <= radius; dx++) {
        for(dy = 0; dy <= radius; dy++) {
            if(dx * dx + dy * dy > radius * radius) {
                DrawPixel(
                    x_0 + (radius - dx) * (isLeft ? 1 : -1),
                    y_0 + (radius - dy) * (isTop ? 1 : -1), 
                    bcgdColor
                );
            }
        }
    } 
}

VOID DrawRoundedRectangle(UINTN x, UINTN y, UINTN w, UINTN h, EFI_COLOR *foreColor, EFI_COLOR *bkgdColor, unsigned radius) {
    gGraphicsOutput->Blt(
        gGraphicsOutput, foreColor, EfiBltVideoFill, 0, 0, x, y, w, h, 0
    );
    if(radius != 0) {
        DrawRadius(x     , y     , bkgdColor, radius, 1, 1);
        DrawRadius(x + w , y     , bkgdColor, radius, 0, 1);
        DrawRadius(x     , y + h , bkgdColor, radius, 1, 0);
        DrawRadius(x + w , y + h , bkgdColor, radius, 0, 0);
    }
}

VOID WriteStrOnPosition(
    CHAR16* str, 
    UINTN x, UINTN y, 
    EFI_COLOR fontColor, 
    EFI_COLOR backgdColor,
    EFI_HII_FONT_STYLE fontStyle,
    UINT16 fontSize
) {
    #pragma warning( disable: 4204)
    EFI_FONT_DISPLAY_INFO fontDisplayInfo = {
        fontColor, 
        backgdColor,
        EFI_FONT_INFO_ANY_FONT,
        {fontStyle, fontSize, L'0'}
    };
    EFI_STATUS Status;
    EFI_IMAGE_OUTPUT sc_p = {
        (UINT16) gGraphicsOutput->Mode->Info->HorizontalResolution,
        (UINT16) gGraphicsOutput->Mode->Info->VerticalResolution,
        NULL
    };
    EFI_IMAGE_OUTPUT* Screen = &sc_p;
    Screen->Image.Screen = gGraphicsOutput;
    Status = gHiiFont->StringToImage(
        gHiiFont,
        EFI_HII_IGNORE_IF_NO_GLYPH | EFI_HII_OUT_FLAG_CLIP | 
        EFI_HII_OUT_FLAG_CLIP_CLEAN_X | EFI_HII_OUT_FLAG_CLIP_CLEAN_Y | 
        EFI_HII_IGNORE_LINE_BREAK | EFI_HII_DRAW_FLAG_TRANSPARENT |
        EFI_HII_DIRECT_TO_SCREEN, 
        (CHAR16 *) str, (EFI_FONT_DISPLAY_INFO *) (& fontDisplayInfo),
        &Screen,
        (UINTN) x, (UINTN) y,
        NULL, NULL, NULL
    );
}

VOID UpdateMoveGUI(UINTN move) {
    WriteStrOnPosition(
        L"             ",
        MARGIN_LEFT + MOVE_PADDING_LEFT,
        MARGIN_TOP + MOVE_PADDING_TOP,
        *font_color_gray,
        *body_backgd,
        EFI_HII_FONT_STYLE_NORMAL,
        MOVE_SIZE
    );
    unsigned i;
    char str_t[30];
    sprintf(str_t, "All Moves: %d", move);
    CHAR16 str_t_16[30];
    for(i = 0; i < sizeof(str_t) / sizeof(char); i++){
        str_t_16[i] = (CHAR16) str_t[i];
    }
    WriteStrOnPosition(
        str_t_16,
        MARGIN_LEFT + MOVE_PADDING_LEFT,
        MARGIN_TOP + MOVE_PADDING_TOP,
        *font_color_gray,
        *body_backgd,
        EFI_HII_FONT_STYLE_NORMAL,
        MOVE_SIZE
    );
}

VOID UpdateValidGUI(int valid) {
    if (valid == preValid) {
        return;
    }
    if (valid == 0) {
        WriteStrOnPosition(
            L"Invalid move!",
            MARGIN_LEFT + VALID_PADDING_LEFT,
            MARGIN_TOP + VALID_PADDING_TOP,
            *font_color_gray,
            *body_backgd,
            EFI_HII_FONT_STYLE_BOLD,
            VALID_SIZE
        );
    } else {
        WriteStrOnPosition(
            L"                ",
            MARGIN_LEFT + VALID_PADDING_LEFT,
            MARGIN_TOP + VALID_PADDING_TOP,
            *font_color_gray,
            *body_backgd,
            EFI_HII_FONT_STYLE_BOLD,
            VALID_SIZE
        );
    }
    preValid = valid;
}


EFI_COLOR *NumToTileColor(unsigned num) {
    unsigned mNums[12]={ 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
    if (num == 0) {
        return tile_backgd;
    } else if (num >= 4096) {
        return &(tileColorBuffer[10]);
    } 
    else {
        unsigned i = 0;
        for(i = 0; i < 12; i++) {
            if(mNums[i] == num) {
                return &(tileColorBuffer[i]);
            }
        }
    }
    return tile_backgd;
}

EFI_COLOR *NumToFontColor(unsigned num) {
    if(num == 0) {
        return tile_backgd;
    } else if (num < 8) {
        return font_color_gray;
    } else {
        return font_color_white;
    }
}


VOID UpdateBoardGUI(struct board* srcBoard) {
    UINT16 i, j;
    for(i = 0; i < BOARD_ROWS; i++) {
        for(j = 0; j < BOARD_COLUMNS; j++) {
            if(preBoard.tiles[i][j] != srcBoard->tiles[i][j]) {
                unsigned num = srcBoard->tiles[i][j];
                EFI_COLOR *tileColor = NumToTileColor(num);
                EFI_COLOR *fontColor = NumToFontColor(num);
                ColorTile(i, j, tileColor);

                if(num != 0) {
                    char str_t[10];
                    sprintf(str_t, "%4d", num);
                    CHAR16 str_t_16[30];
                    unsigned ii;
                    for(ii = 0; ii < sizeof(str_t) / sizeof(char); ii++){
                        str_t_16[ii] = (CHAR16) str_t[ii];
                    }
                    WriteStrOnPosition(
                        str_t_16,
                        MARGIN_LEFT + (TILE_WIDTH + GAP_WIDTH) * j + GAP_WIDTH + TILE_WIDTH / 2,
                        MARGIN_TOP  + (TILE_WIDTH + GAP_WIDTH) * i + GAP_WIDTH + TILE_WIDTH / 2,
                        *fontColor,
                        *tileColor,
                        EFI_HII_FONT_STYLE_BOLD,
                        30
                    );
                }
                preBoard.tiles[i][j] = srcBoard->tiles[i][j];
            }
        }
    }
}

VOID FinallyDisplay(int autoMove, int status) {
    WriteStrOnPosition(
        L"                ",
        MARGIN_LEFT + VALID_PADDING_LEFT,
        MARGIN_TOP + VALID_PADDING_TOP,
        *font_color_gray,
        *body_backgd,
        EFI_HII_FONT_STYLE_BOLD,
        VALID_SIZE
    );
    if(autoMove) {
        WriteStrOnPosition(
            L"Game over. Press any Key to quit.",
            MARGIN_LEFT + VALID_PADDING_LEFT,
            MARGIN_TOP + VALID_PADDING_TOP,
            *font_color_gray,
            *body_backgd,
            EFI_HII_FONT_STYLE_BOLD,
            VALID_SIZE
        );
    } else {
        char str[50];
        sprintf(str, "Game over, you %s! Press any Key to quit.", (status < 0) ? "LOSE" : "WIN");
        CHAR16 str_16[50];
        int i;
        for(i = 0; i < sizeof(str) / sizeof(char); i++) {
            str_16[i] = str[i];
        }
        WriteStrOnPosition(
            str_16,
            MARGIN_LEFT + VALID_PADDING_LEFT,
            MARGIN_TOP + VALID_PADDING_TOP,
            *font_color_gray,
            *body_backgd,
            EFI_HII_FONT_STYLE_BOLD,
            VALID_SIZE
        );
        
    }
}

EFI_STATUS SwitchGraphicsMode(BOOLEAN flag) {
	EFI_STATUS  Status;
    Status=gST->ConOut->EnableCursor (gST->ConOut, flag);
	return Status;
}

VOID SetMyMode(UINT32 ModeNumber) {
   gGraphicsOutput->SetMode(gGraphicsOutput, ModeNumber);
   return;
}

EFI_STATUS LocateSimpleTextInputEx(void) {
    EFI_STATUS Status;
    EFI_HANDLE *Handles;
    UINTN HandleCount;
    UINTN HandleIndex;

    Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiSimpleTextInputExProtocolGuid,
                NULL,
                &HandleCount,
                &Handles
                );
    if(EFI_ERROR (Status)) return Status;

    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        Status = gBS->HandleProtocol (Handles[HandleIndex], &gEfiSimpleTextInputExProtocolGuid, (VOID **) &gSimpleTextInputEx);
        if (EFI_ERROR(Status))	
            continue;
        else {
            return EFI_SUCCESS;
        }
    }
    return Status;
}

EFI_STATUS LocateGraphicsOutput (void) {
	EFI_STATUS Status;
	EFI_HANDLE *GraphicsOutputControllerHandles = NULL;
	UINTN HandleIndex = 0;
	UINTN HandleCount = 0;
	//get the handles which supports GraphicsOutputProtocol
	Status = gBS->LocateHandleBuffer(
		ByProtocol,
		&gEfiGraphicsOutputProtocolGuid,
		NULL,
		&HandleCount,
		&GraphicsOutputControllerHandles
		);
	if (EFI_ERROR(Status))	return Status;		//unsupport
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
		Status = gBS->HandleProtocol(
			GraphicsOutputControllerHandles[HandleIndex],
			&gEfiGraphicsOutputProtocolGuid,
			(VOID**)&gGraphicsOutput);
		if (EFI_ERROR(Status))	continue;

		else {
			return EFI_SUCCESS;
		}
	}
	return Status;
}