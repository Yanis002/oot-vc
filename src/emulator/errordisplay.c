/**
 * @file errordisplay.c
 *
 * This file implements the screen that can be viewed during the emulator's initialization process.
 * In most cases it prints "You will need the Classic Controller."
 * but an error message can be printed if something goes wrong.
 *
 * This file is also responsible for reading the banner title and subtitle from the string table.
 */
#include "emulator/errordisplay.h"
#include "emulator/controller.h"
#include "emulator/frame.h"
#include "emulator/rom.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlCoreRVL.h"
#include "emulator/xlFile.h"
#include "emulator/xlHeap.h"
#include "macros.h"
#include "revolution/demo.h"
#include "revolution/nand.h"
#include "revolution/os.h"
#include "revolution/vi.h"

static s32 fn_80063680(EDString* pEDString);
static s32 errorDisplayReturnToMenu(EDString* pEDString);

static EDStringInfo sStringBase[] = {
    {SID_ERROR_INS_SPACE, 0, NULL, 0x00000000, 0x00000000},
#if IS_MM
    {SID_ERROR_INS_SPACE_PLURAL, 0, NULL, 0x00000000, 0x00000000},
#endif
    {SID_ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_INS_INNODE, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_SYS_CORRUPT, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_DATA_CORRUPT, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_MAX_BLOCKS, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_MAX_FILES, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_NO_CONTROLLER, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_NEED_CLASSIC, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_REMOTE_BATTERY, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_REMOTE_COMMUNICATION, 0, NULL, 0x00000000, 0x00000000},
#if IS_MM
    {SID_ERROR_NWC24_ERROR_BUFFER, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_PHOTO_ALREADY_POSTED, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_PHOTO_CANT_POST, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_PHOTO_POSTED, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_BLANK, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_SYS_CORRUPT_REDOWNLOAD, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_SYS_MEM_CANT_ACCESS, 0, NULL, 0x00000000, 0x00000000},
    {SID_ERROR_SYS_MEM_CANT_ACCESS_2, 0, NULL, 0x00000000, 0x00000000},
#endif
    {SID_ERROR_BLANK, 0, NULL, 0x00000000, 0x00000000},
#if IS_OOT
    {SID_NONE, 0, NULL, 0x00000000, 0x00000000},
#endif
};

#if IS_MM
// unused strings?
static char lbl_801538BC[] = {
    "Error #302,\nThere is not enough available\nspace in the Wii System Memory.\nCreate 1 block of free space\nby either moving data to an\nSD Card or deleting data on\nthe Data Management Screen."
};

static char lbl_80153978[] = {
    "Error #302,\nThere is not enough available\nspace in the Wii System Memory.\nCreate %ld blocks of free space\nby either moving data to an\nSD Card or deleting data on\nthe Data Management Screen."
};

static char lbl_80153A38[] = {
    "Error #303,\nThere is not enough available\nspace in the Wii System Memory.\nEither move data to an SD Card\nor delete data on the\nData Management Screen."
};

static char lbl_80153AD0[] = {
    "Error #308,\nThe Wii System Memory\nhas been corrupted.\nRefer to the Wii Operations Manual\nfor further instructions."
};

static char lbl_80153B44[] = {
    "Error #307,\nThis data is corrupted and cannot be used."
};

static char lbl_80153B7C[] = {
    "Error #305,\nThere is no more available space\nin the Wii System Memory.\nRefer to the Wii Operations Manual\nfor further information."
};

static char lbl_80153C00[] = {
    "Error #306,\nUnable to save any more data\nto the Wii System Memory.\nRefer to the Wii Operations Manual\nfor further information."
};

static char lbl_80153C80[] = {
    "The system file for this title\nis corrupted.\nAfter deleting the title from\nthe Data Management screen,\nredownload it from\nthe Wii Shop Channel.\nIf this message still appears\nvisit support.nintendo.com."
};

static char lbl_80153D4C[] = {
    "Error #311,\nCould not write to/read from\nWii System Memory.For details, please read the Wii\nOperations Manual."
};

static char lbl_80153DBC[] = {
    "Error #312,\nCould not write to/read from\nWii System Memory.For details, please read the Wii\nOperations Manual."
};
#endif

ErrorDisplay sStringDraw[] = {
    {
        {&sStringBase[ERROR_INS_SPACE], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU],
                FLAG_COLOR_WHITE,
                0,
                0,
                errorDisplayReturnToMenu,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
#if IS_MM
    {
        {&sStringBase[ERROR_INS_SPACE_PLURAL], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU],
                FLAG_COLOR_WHITE,
                0,
                0,
                errorDisplayReturnToMenu,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,

        {0},
    },
#endif
    {
        {&sStringBase[ERROR_INS_INNODE], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU],
                FLAG_COLOR_WHITE,
                0,
                0,
                errorDisplayReturnToMenu,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_SYS_CORRUPT], FLAG_COLOR_WHITE, 0, 0},
        {0},
        0,
        NULL,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_DATA_CORRUPT], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU],
                FLAG_COLOR_WHITE,
                0x0000,
                0x00000000,
                errorDisplayReturnToMenu,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_MAX_BLOCKS], FLAG_COLOR_WHITE, 0, 0},
        {0},
        0,
        NULL,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_MAX_FILES], FLAG_COLOR_WHITE, 0, 0},
        {0},
        0,
        NULL,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_SYS_CORRUPT], FLAG_COLOR_WHITE, 0, 0},
        {0},
        0,
        NULL,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_NO_CONTROLLER], FLAG_RESET_FADE_TIMER, 0, 0},
        {
            {
#if IS_OOT
                &sStringBase[ERROR_NULL],
#elif IS_MM
                &sStringBase[ERROR_BLANK_2],
#endif
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        0,
        fn_80042E30,
        120,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_NO_CONTROLLER], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_NEED_CLASSIC],
                FLAG_RESET_FADE_TIMER | FLAG_COLOR_YELLOW,
                0,
                0,
                fn_80063680,
            },
        },
        1,
        fn_80062028,
        120,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_REMOTE_BATTERY], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_NULL],
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        1,
        fn_80062028,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
    {
        {&sStringBase[ERROR_REMOTE_COMMUNICATION], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_NULL],
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        1,
        fn_80062028,
        0,
        0,
        0,
        0,
#if IS_MM
        {0},
#endif
    },
#if IS_MM
    {
        {&sStringBase[ERROR_NWC24_ERROR_BUFFER], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_BLANK_2],
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
        {0},
    },
    {
        {&sStringBase[ERROR_PHOTO_ALREADY_POSTED], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_BLANK_2],
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
        {0},
    },
    {
        {&sStringBase[ERROR_PHOTO_CANT_POST], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_BLANK_2],
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
        {0},
    },
    {
        {&sStringBase[ERROR_PHOTO_POSTED], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_BLANK_2],
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
        {0},
    },
#endif
    {
        {&sStringBase[ERROR_BLANK_2], FLAG_RESET_FADE_TIMER, 0, 0},
        {
            {
#if IS_OOT
                &sStringBase[ERROR_NULL],
#elif IS_MM
                &sStringBase[ERROR_BLANK_2],
#endif
                FLAG_COLOR_WHITE,
                0,
                0,
                fn_80063680,
            },
        },
        0,
        fn_80042E30,
        120,
        0,
        0,
        0,
#if IS_MM
        {"gameReloadingAnimation.tpl", 0x21, 320, 216},
#endif
    },
#if IS_MM
    {
        {&sStringBase[ERROR_SYS_CORRUPT_REDOWNLOAD], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU],
                FLAG_COLOR_WHITE,
                0,
                0,
                errorDisplayReturnToMenu,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
        {0},
    },
    {
        {&sStringBase[ERROR_SYS_MEM_CANT_ACCESS], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_BLANK_2],
                FLAG_COLOR_WHITE,
                0,
                0,
                NULL,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
        {0},
    },
    {
        {&sStringBase[ERROR_SYS_MEM_CANT_ACCESS_2], FLAG_COLOR_WHITE, 0, 0},
        {
            {
                &sStringBase[ERROR_BLANK_2],
                FLAG_COLOR_WHITE,
                0,
                0,
                NULL,
            },
        },
        1,
        NULL,
        0,
        0,
        0,
        0,
        {0},
    },
#endif
};

struct_80174988 lbl_80174988[] = {
    {NAND_RESULT_ACCESS, ERROR_NULL},
    {NAND_RESULT_ALLOC_FAILED, ERROR_INS_SPACE},
    {NAND_RESULT_BUSY, ERROR_NULL},
    {NAND_RESULT_CORRUPT, ERROR_INS_INNODE},
    {NAND_RESULT_ECC_CRIT, ERROR_SYS_CORRUPT},
    {NAND_RESULT_EXISTS, ERROR_NULL},
    {NAND_RESULT_INVALID, ERROR_MAX_FILES},
    {NAND_RESULT_MAXBLOCKS, ERROR_DATA_CORRUPT},
    {NAND_RESULT_MAXFD, ERROR_MAX_BLOCKS},
    {NAND_RESULT_MAXFILES, ERROR_MAX_BLOCKS},
    {NAND_RESULT_NOEXISTS, ERROR_NULL},
    {NAND_RESULT_NOTEMPTY, ERROR_NULL},
    {NAND_RESULT_OPENFD, ERROR_NULL},
    {NAND_RESULT_AUTHENTICATION, ERROR_SYS_CORRUPT},
    {NAND_RESULT_UNKNOWN, ERROR_MAX_FILES},
    {NAND_RESULT_FATAL_ERROR, ERROR_MAX_FILES},
    {NAND_RESULT_OK, ERROR_NULL},
};

static DisplayFiles sSTFiles[] = {
#if VERSION == OOT_J || VERSION == MM_J
    {SC_LANG_JP, "Errors_VC64ErrorStrings_jp.bin", "saveComments_saveComments_jp.bin"},
#elif VERSION == OOT_U
    {SC_LANG_EN, "Errors_VC64ErrorStrings_en.bin", "saveComments_saveComments_en.bin"},
    {SC_LANG_FR, "Errors_VC64ErrorStrings_fr.bin", "saveComments_saveComments_fr.bin"},
    {SC_LANG_SP, "Errors_VC64ErrorStrings_es.bin", "saveComments_saveComments_es.bin"},
#elif VERSION == OOT_E
    {SC_LANG_EN, "Errors_VC64ErrorStrings_en.bin", "saveComments_saveComments_en.bin"},
    {SC_LANG_FR, "Errors_VC64ErrorStrings_fr.bin", "saveComments_saveComments_fr.bin"},
    {SC_LANG_SP, "Errors_VC64ErrorStrings_es.bin", "saveComments_saveComments_es.bin"},
    {SC_LANG_DE, "Errors_VC64ErrorStrings_de.bin", "saveComments_saveComments_de.bin"},
    {SC_LANG_IT, "Errors_VC64ErrorStrings_it.bin", "saveComments_saveComments_it.bin"},
    {SC_LANG_NL, "Errors_VC64ErrorStrings_nl.bin", "saveComments_saveComments_nl.bin"},
#endif
    {SC_LANG_NONE, NULL, NULL},
};

static void* sBufferErrorStrings;
static void* sBufferSaveCommentStrings;
bool lbl_8025D130;
s32 lbl_8025D12C;
static OSFontHeader* sFontHeader;

/**
 * @brief Custom implementation of the `DEMOGetRenderModeObj` SDK function.
 * @return `GXRenderModeObj*` – Pointer to the render mode object.
 */
GXRenderModeObj* DEMOGetRenderModeObj(void) { return rmode; }

/**
 * @brief Prepares the graphics interface for a draw action.
 */
static void errorDisplayDrawSetup(void) {
    GXColor WHITE = {255, 255, 255, 255};
    GXColor BLACK = {0, 0, 0, 255};

    DEMOSetupScrnSpc(GC_FRAME_WIDTH, GC_FRAME_HEIGHT, 100.0f);

    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_ONE, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColor(GX_TEVREG0, BLACK);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA4, 0);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2s16(0, 0);
    GXPosition2s16(1, GC_FRAME_WIDTH);
    GXPosition2s16(0, 1);
    GXPosition2s16(GC_FRAME_WIDTH, GC_FRAME_HEIGHT);
    GXPosition2s16(1, 0);
    GXPosition2s16(GC_FRAME_HEIGHT, 1);
    GXEnd();

    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetNumChans(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColor(GX_TEVREG0, WHITE);
}

static s32 fn_80063680(EDString* pEDString) { return 2; }

static s32 fn_80063688(EDString* pEDString, s32 arg1) {
    ErrorDisplay* pErrorDisplay = pEDString->apStringDraw[pEDString->iString];

    if (arg1 & 0x08200000) {
        if (pEDString->iAction > 0) {
            pEDString->iAction--;
        }
    } else if (arg1 & 0x10400000) {
        if (pEDString->iAction < pErrorDisplay->nAction - 1) {
            pEDString->iAction++;
        }
    } else if ((arg1 & 0x20000001) | (arg1 & 1)) {
        if (pErrorDisplay->nAction > 0 && pErrorDisplay->action[pEDString->iAction].callback != NULL) {
            return pErrorDisplay->action[pEDString->iAction].callback(pEDString);
        }
    } else {
        if (pErrorDisplay->callback != NULL) {
            return pErrorDisplay->callback(pEDString);
        }
    }

    return 0;
}

static s32 errorDisplayReturnToMenu(EDString* pEDString) {
    VISetBlack(true);
    VIFlush();
    VIWaitForRetrace();
    OSReturnToMenu();

    return 0;
}

static void fn_80063764(EDStringInfo* pStringInfo) {
    u32 widthOut;
    s16 nSize;
    s16 nSpace;
    char* var_r27;
    char* var_r31;
    char* var_r3;
    s32 var_r30;
    s32 nCellWidth;
    s32 i;
    s32 temp_r27;
    char* temp_r3_2;

    nCellWidth = sFontHeader->cellWidth;
    DEMOGetROMFontSize(&nSize, &nSpace);
    nSize *= 0x10;
    nSpace *= 0x10;
    pStringInfo->unk10 = 0;
    pStringInfo->unk0C = 0;

    if (pStringInfo->eStringID != SID_NONE) {
        pStringInfo->szString = tableGetString(sBufferErrorStrings, pStringInfo->eStringID);

        if (pStringInfo->szString != NULL) {
            var_r27 = pStringInfo->szString;
            pStringInfo->nLines = 1;

            while (*var_r27 != '\0') {
                temp_r3_2 = (char*)OSGetFontWidth(var_r27, &widthOut);

                if (*var_r27 == '\n') {
                    pStringInfo->nLines++;
                    *var_r27 = '\0';
                }

                var_r27 = temp_r3_2;
            }

            var_r3 = pStringInfo->szString;
            temp_r27 = pStringInfo->nLines;

            for (i = 0; i < temp_r27; i++) {
                var_r30 = 0;
                var_r31 = NULL;

                while (*var_r3 != '\0') {
                    if (*var_r3 == ' ' || *var_r3 == '\t') {
                        var_r31 = var_r3;
                    }

                    var_r3 = (char*)OSGetFontWidth(var_r3, &widthOut);
                    var_r30 = ((s32)(nSize * widthOut) / nCellWidth) + var_r30 + nSpace;

#if IS_OOT
#define COND_UNK 0x230
#elif IS_MM
#define COND_UNK 0x216
#endif

                    if (((var_r30 + 15) / 16 > COND_UNK) && var_r31 != NULL) {
                        *var_r31 = 0;
                        var_r3 = var_r31 + 1;
                        var_r30 = 0;
                        var_r31 = NULL;
                        pStringInfo->nLines++;
                    }
                }

                var_r3++;
            }

            pStringInfo->unk10 = ((pStringInfo->nLines * ((sFontHeader->leading * nSize) / nCellWidth)) + 15) / 16;
        }
    }
}

static void fn_80063910(ErrorDisplay* pErrorDisplay) {
    s32 i;

    pErrorDisplay->unk3C = 0;

    if (pErrorDisplay->message.pStringInfo != NULL) {
        pErrorDisplay->message.nShiftY = pErrorDisplay->message.pStringInfo->unk10;
    } else {
        pErrorDisplay->message.nShiftY = 0;
    }

    if (pErrorDisplay->nAction > 0) {
        pErrorDisplay->message.nShiftY += DEMOGetRFTextHeight("\n");
    }

    pErrorDisplay->unk3C = pErrorDisplay->message.nShiftY;

    for (i = 0; i < pErrorDisplay->nAction; i++) {
        if (pErrorDisplay->action[i].message.pStringInfo != NULL) {
            pErrorDisplay->action[i].message.nShiftY = pErrorDisplay->action[i].message.pStringInfo->unk10;
        } else {
            pErrorDisplay->action[i].message.nShiftY = 0;
        }

        pErrorDisplay->unk3C += pErrorDisplay->action[i].message.nShiftY;
    }
}

/**
 * @brief Prints a message.
 * @param pEDString Pointer to `EDString`.
 * @param nHeight The Y-position of the message to print.
 * @param nPrintfArg Printf format string argument.
 * @param color The color of the message.
 */
static void errorDisplayPrintMessage(EDMessage* pMessage, s32 nHeight, s32 nPrintfArg, GXColor color) {
    char* szString;
    s32 i;
    EDStringInfo* pStringInfo;
    s32 nY;
    s32 nTextHeight;

    pStringInfo = pMessage->pStringInfo;

    if (pStringInfo != NULL) {
        nTextHeight = DEMOGetRFTextHeight("");

        nY = nHeight;
        color.a = (pMessage->nFadeInTimer * 255) / FADE_TIMER_MAX;

        if (pMessage->nFadeInTimer < FADE_TIMER_MAX) {
            pMessage->nFadeInTimer++;
        }

        GXSetTevColor(GX_TEVREG0, color);
        szString = pStringInfo->szString;

        for (i = 0; i < pStringInfo->nLines; i++) {
            DEMOPrintf((GC_FRAME_WIDTH - DEMOGetRFTextWidth(szString)) / 2, nY, 1, szString, nPrintfArg);

            while (*szString != '\0') {
                szString++;
            }

            szString++;
            nY += nTextHeight;
        }
    }
}

/**
 * @brief Prints the error message and the corresponding action message (if applicable)
 * @param pEDString Pointer to `EDString`.
 */
static void errorDisplayPrint(EDString* pEDString) {
    GXColor YELLOW = {255, 255, 0, 255};
    GXColor WHITE = {255, 255, 255, 255};
    GXColor color;
    s32 nHeight;
    ErrorDisplay* pErrorDisplay;
    s32 i;

    pErrorDisplay = pEDString->apStringDraw[pEDString->iString];

    nHeight = pErrorDisplay->nStartY;
    if (nHeight == 0) {
        nHeight = (GC_FRAME_HEIGHT - pErrorDisplay->unk3C) / 2;
    }

    errorDisplayPrintMessage(&pErrorDisplay->message, nHeight, pErrorDisplay->unk38, WHITE);

#if IS_MM
    fn_80080370(pErrorDisplay);
#endif

    nHeight += pErrorDisplay->message.nShiftY;

    i = 0;
    while (i < pErrorDisplay->nAction) {
        if (i == pEDString->iAction && !(pErrorDisplay->action[i].message.nFlags & FLAG_COLOR_YELLOW)) {
            color = YELLOW;
        } else {
            color = WHITE;
        }

        errorDisplayPrintMessage(&pErrorDisplay->action[i].message, nHeight, 0, color);
        i++;
        nHeight += pErrorDisplay->action[i].message.nShiftY;
    }
}

/**
 * @brief Custom implementation of the `OSAllocFromHeap` SDK function
 * @param handle Unused.
 * @param size Size of the heap to allocate.
 * @return `void*` – Pointer to the allocated heap.
 */
void* OSAllocFromHeap(s32 handle, s32 size) {
    void* pHeap;

    xlHeapTake(&pHeap, size | 0x70000000);

    return pHeap;
}

/**
 * @brief Custom implementation of the `OSFreeToHeap` SDK function
 * @param handle Unused.
 * @param p Pointer to the heap to free.
 */
void OSFreeToHeap(s32 handle, void* p) {
    void* pHeap = p;

    xlHeapFree(&pHeap);
}

/**
 * @brief Initializes the error display system and the NAND banner.
 *
 * This function does the following:
 *
 * - load the string tables corresponding to the system's language
 *
 * - load and setup the font to use
 *
 * - setup the print positions (the horizontal position is always centered to the screen)
 *
 * - prepare the NAND banner (the title and subtitle of the banner comes from the "save comments" string table)
 */
void errorDisplayInit(void) {
    ErrorDisplay* pErrorDisplay;
    EDStringInfo* pStringInfo;
    s32 iError;
    s32 iInfo;
    DisplayFiles* pDisplayFiles;
    u32 nLanguage;

    nLanguage = SCGetLanguage();
    pDisplayFiles = &sSTFiles[0];

    while (pDisplayFiles->szErrorsFilename != NULL) {
        if (pDisplayFiles->eLanguage == nLanguage) {
            break;
        }
        pDisplayFiles++;
    }

    if (pDisplayFiles->szErrorsFilename == NULL) {
        pDisplayFiles = &sSTFiles[0];
    }

    xlFileLoad(pDisplayFiles->szErrorsFilename, (void**)&sBufferErrorStrings);
    xlFileLoad(pDisplayFiles->szSaveCommentsFilename, (void**)&sBufferSaveCommentStrings);
    sFontHeader = DEMOInitROMFont();

    pStringInfo = &sStringBase[ERROR_INS_SPACE];
    for (iInfo = 0; iInfo < ARRAY_COUNT(sStringBase); iInfo++) {
        fn_80063764(pStringInfo);
        pStringInfo++;
    }

    pErrorDisplay = &sStringDraw[0];
    for (iError = 0; iError < ARRAY_COUNT(sStringDraw); iError++) {
        fn_80063910(pErrorDisplay);
        pErrorDisplay++;
    }

    bannerCreate(tableGetString(sBufferSaveCommentStrings, SID_COMMENT_GAME_NAME),
                 tableGetString(sBufferSaveCommentStrings, SID_COMMENT_EMPTY));
}

static inline void errorDisplaySetFadeInTimer(ErrorDisplay* pErrorDisplay) {
    s32 i;

    if (pErrorDisplay->message.nFlags & FLAG_RESET_FADE_TIMER) {
        pErrorDisplay->message.nFadeInTimer = 0;
    } else {
        pErrorDisplay->message.nFadeInTimer = FADE_TIMER_MAX;
    }

    for (i = 0; i < pErrorDisplay->nAction; i++) {
        if (pErrorDisplay->action[i].message.nFlags & FLAG_RESET_FADE_TIMER) {
            pErrorDisplay->action[i].message.nFadeInTimer = 0;
        } else {
            pErrorDisplay->action[i].message.nFadeInTimer = FADE_TIMER_MAX;
        }
    }
}

bool fn_8007F440(s32 arg0) {
    // fn_800800EC(sStringDraw[arg0] + 0x5C);
    return true;
}

// fn_8007F474

/**
 * @brief Main error display function.
 * @param iString Index of the message to show.
 * @return `bool` – `true` on success, `false` on failure.
 */
bool errorDisplayShow(ErrorIndex iString) {
    EDString string;
    s32 var_r31;
    s32 var_r30;
    s32 iController;
    s32 spC;
    s32 sp8;
    s32 nResult;

    string.iString = ERROR_NONE;

    if (!fn_800607B0(SYSTEM_HELP(gpSystem), 0)) {
        return false;
    }

    string.iString++;
    string.apStringDraw[string.iString] = &sStringDraw[iString];
    string.iAction = 0;

    errorDisplaySetFadeInTimer(&sStringDraw[iString]);
    VISetBlack(false);

    do {
        var_r30 = 0;
        var_r31 = 0;

        for (iController = 0; iController < PAD_MAX_CONTROLLERS; iController++) {
            fn_80062C18(SYSTEM_CONTROLLER(gpSystem), iController, &spC, &sp8, NULL, NULL, NULL, NULL);
            var_r31 |= spC;
            var_r30 |= sp8;
        }

        xlCoreBeforeRender();
        errorDisplayDrawSetup();
        errorDisplayPrint(&string);
        fn_8005F7E4(SYSTEM_HELP(gpSystem));
        fn_80007020();
        nResult = fn_80063688(&string, var_r31 & (var_r31 ^ var_r30));
    } while (nResult == 0);

    if (fn_800607B0(SYSTEM_HELP(gpSystem), 1)) {
        return nResult != 1;
    }

    return false;
}
