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
#include "emulator/helpRVL.h"
#include "emulator/rom.h"
#include "emulator/soundRVL.h"
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

//! TODO: document
extern s32 lbl_802007B0;
extern s32 lbl_802007B4;

static s32 fn_80063680(EDString* pEDString);
static s32 errorDisplayReturnToMenu(EDString* pEDString);

#if IS_MM
static void fn_8007EF3C(ErrorIndex iString, s32* arg1, s32* arg2, s32* arg3, s32* arg4);
#endif

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
static char* lbl_80153DBC[] = {
    "Error #302,\nThere is not enough available\nspace in the Wii System Memory.\nCreate 1 block of free space\nby "
    "either moving data to an\nSD Card or deleting data on\nthe Data Management Screen.",
    "Error #302,\nThere is not enough available\nspace in the Wii System Memory.\nCreate %ld blocks of free space\nby "
    "either moving data to an\nSD Card or deleting data on\nthe Data Management Screen.",
    "Error #303,\nThere is not enough available\nspace in the Wii System Memory.\nEither move data to an SD Card\nor "
    "delete data on the\nData Management Screen.",
    "Error #308,\nThe Wii System Memory\nhas been corrupted.\nRefer to the Wii Operations Manual\nfor further "
    "instructions.",
    "Error #307,\nThis data is corrupted and cannot be used.",
    "Error #305,\nThere is no more available space\nin the Wii System Memory.\nRefer to the Wii Operations Manual\nfor "
    "further information.",
    "Error #306,\nUnable to save any more data\nto the Wii System Memory.\nRefer to the Wii Operations Manual\nfor "
    "further information.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "The system file for this title\nis corrupted.\nAfter deleting the title from\nthe Data Management "
    "screen,\nredownload it from\nthe Wii Shop Channel.\nIf this message still appears\nvisit support.nintendo.com.",
    "Error #311,\nCould not write to/read from\nWii System Memory.For details, please read the Wii\nOperations Manual.",
    "Error #312,\nCould not write to/read from\nWii System Memory.For details, please read the Wii\nOperations Manual.",
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
                &sStringBase[ERROR_EMPTY],
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
                &sStringBase[ERROR_EMPTY],
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
    {NAND_RESULT_ACCESS, ERROR_EMPTY},
    {NAND_RESULT_ALLOC_FAILED, ERROR_INS_SPACE},
    {NAND_RESULT_BUSY, ERROR_EMPTY},
    {NAND_RESULT_CORRUPT, ERROR_INS_INNODE},
    {NAND_RESULT_ECC_CRIT, ERROR_SYS_CORRUPT},
    {NAND_RESULT_EXISTS, ERROR_EMPTY},
    {NAND_RESULT_INVALID, ERROR_MAX_FILES},
    {NAND_RESULT_MAXBLOCKS, ERROR_DATA_CORRUPT},
    {NAND_RESULT_MAXFD, ERROR_MAX_BLOCKS},
    {NAND_RESULT_MAXFILES, ERROR_MAX_BLOCKS},
    {NAND_RESULT_NOEXISTS, ERROR_EMPTY},
    {NAND_RESULT_NOTEMPTY, ERROR_EMPTY},
    {NAND_RESULT_OPENFD, ERROR_EMPTY},
    {NAND_RESULT_AUTHENTICATION, ERROR_SYS_CORRUPT},
    {NAND_RESULT_UNKNOWN, ERROR_MAX_FILES},
    {NAND_RESULT_FATAL_ERROR, ERROR_MAX_FILES},
    {NAND_RESULT_OK, ERROR_EMPTY},
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

#if IS_OOT
/**
 * @brief Prepares the graphics interface for a draw action.
 */
static void errorDisplayDrawSetup(void)
#elif IS_MM
/**
 * @brief Prepares the graphics interface for a draw action.
 * @param iString Index of the message to show.
 * @param bFullscreen `true` to setup full-screen, `false` to setup where the message will be shown.
 */
static void errorDisplayDrawSetup(ErrorIndex iString, bool bFullscreen)
#endif
{
    GXColor WHITE = {255, 255, 255, 255};
    GXColor BLACK = {0, 0, 0, 255};

#if IS_OOT
    DEMOSetupScrnSpc(GC_FRAME_WIDTH, GC_FRAME_HEIGHT, 100.0f);
#elif IS_MM
    if (bFullscreen) {
        DEMOSetupScrnSpc(GC_FRAME_WIDTH, GC_FRAME_HEIGHT, 100.0f);
    } else {
        DEMOSetupScrnSpc(rmode->fbWidth, rmode->efbHeight, 100.0f);
    }
#endif

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

#if IS_OOT
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2s16(0, 0);
    GXPosition2s16(1, GC_FRAME_WIDTH);
    GXPosition2s16(0, 1);
    GXPosition2s16(GC_FRAME_WIDTH, GC_FRAME_HEIGHT);
    GXPosition2s16(1, 0);
    GXPosition2s16(GC_FRAME_HEIGHT, 1);
    GXEnd();
#elif IS_MM
    if (bFullscreen) {
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2s16(0, 0);
        GXPosition2s16(1, GC_FRAME_WIDTH);
        GXPosition2s16(0, 1);
        GXPosition2s16(GC_FRAME_WIDTH, GC_FRAME_HEIGHT);
        GXPosition2s16(1, 0);
        GXPosition2s16(GC_FRAME_HEIGHT, 1);
        GXEnd();
    } else {
        s32 nX0 = 0;
        s32 nX1 = 0;
        s32 nY0 = 0;
        s32 nY1 = 0;

        fn_8007EF3C(iString, &nX0, &nX1, &nY0, &nY1);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2s16(nX0 - 20, nY0 - 20);
        GXPosition2s16(1, nX1 + 20);
        GXPosition2s16(nY0 - 20, 1);
        GXPosition2s16(nX1 + 20, nY1 + 20);
        GXPosition2s16(1, nX0 - 20);
        GXPosition2s16(nY1 + 20, 1);
        GXEnd();
    }
#endif

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

#if IS_MM || IS_MT
static inline bool fn_80063688_Inline(ErrorDisplay* pErrorDisplay) {
    bool ret = true;

    if (pErrorDisplay->anim.unk0C != 0) {
        if (pErrorDisplay->anim.unk18 > 0) {
            pErrorDisplay->anim.unk18 -= 0x11;

            if (pErrorDisplay->anim.unk18 < 0) {
                pErrorDisplay->anim.unk18 = 0;
            }

            ret = false;
        }
    }

    return ret;
}
#endif

static inline s32 fn_80063688_Inline2(ErrorDisplay* pErrorDisplay, EDString* pEDString) {
    s32 var_r3 = pErrorDisplay->callback(pEDString);

    if (var_r3 == 2 && !fn_80063688_Inline(pErrorDisplay)) {
        return 0;
    }

    return var_r3;
}

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
#if IS_OOT
        if (pErrorDisplay->callback != NULL) {
            return pErrorDisplay->callback(pEDString);
        }
#elif IS_MM
        if (pErrorDisplay->anim.unk0C != 0) {
            s32 temp_r3 = OSGetTick();
            s32 temp_r6 = temp_r3 - pErrorDisplay->anim.unk14;

            if (temp_r6 < 0) {
                pErrorDisplay->anim.unk14 = temp_r3;
            } else if (temp_r6 > OSMillisecondsToTicks(pErrorDisplay->anim.unk_04)) {
                pErrorDisplay->anim.unk14 = temp_r3;
                pErrorDisplay->anim.unk10++;

                if (pErrorDisplay->anim.unk10 >= pErrorDisplay->anim.unk0C[1]) {
                    pErrorDisplay->anim.unk10 = 0;
                }
            }
        }

        if (pErrorDisplay->callback != NULL) {
            return fn_80063688_Inline2(pErrorDisplay, pEDString);
        }
#endif
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

#if IS_MM
static void fn_8007EF3C(ErrorIndex iString, s32* pnX0, s32* pnX1, s32* pnY0, s32* pnY1) {
    ErrorDisplay* pErrorDisplay;
    s32 var_r31;
    s32 i;
    EDStringInfo* pStringInfo;
    s32 nWidth;
    s32 nHeight;
    s32 nTextWidth;
    char* var_r27;

    pErrorDisplay = &sStringDraw[iString];
    pStringInfo = pErrorDisplay->message.pStringInfo;
    var_r31 = 0;

    nHeight = pErrorDisplay->nStartY;
    if (nHeight == 0) {
        nHeight = (rmode->efbHeight - pErrorDisplay->unk3C) / 2;
    }

    *pnY0 = nHeight;
    *pnY0 -= DEMOGetRFTextHeight("") - 8;
    *pnY1 = *pnY0 + pStringInfo->nLines * DEMOGetRFTextHeight("") - 8;

    for (var_r27 = pStringInfo->szString, i = 0; i < pStringInfo->nLines; var_r27++, i++) {
        nTextWidth = DEMOGetRFTextWidth(var_r27);

        if (nTextWidth > var_r31) {
            var_r31 = nTextWidth;
            nWidth = (rmode->fbWidth - nTextWidth) / 2;
            *pnX0 = nWidth;
            *pnX1 = nWidth + nTextWidth;
        }

        while (*var_r27 != '\0') {
            var_r27++;
        }
    }
}
#endif

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

    for (pDisplayFiles = &sSTFiles[0]; pDisplayFiles->szErrorsFilename != NULL; pDisplayFiles++) {
        if (pDisplayFiles->eLanguage == nLanguage) {
            break;
        }
    }

    if (pDisplayFiles->szErrorsFilename == NULL) {
        pDisplayFiles = &sSTFiles[0];
    }

#if IS_OOT
    xlFileLoad(pDisplayFiles->szErrorsFilename, (void**)&sBufferErrorStrings);
    xlFileLoad(pDisplayFiles->szSaveCommentsFilename, (void**)&sBufferSaveCommentStrings);
    sFontHeader = DEMOInitROMFont();
#elif IS_MM
    if (pDisplayFiles->eLanguage == SC_LANG_JP) {
        OSSetFontEncode(OS_FONT_ENCODE_SJIS);
    } else {
        OSSetFontEncode(OS_FONT_ENCODE_ANSI);
    }

    sFontHeader = DEMOInitROMFont();
    xlFileLoad(pDisplayFiles->szErrorsFilename, (void**)&sBufferErrorStrings);
#endif

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

#if IS_MM
    lbl_802007B0 = 1;
    xlFileLoad(pDisplayFiles->szSaveCommentsFilename, (void**)&sBufferSaveCommentStrings);
#endif

    bannerCreate(tableGetString(sBufferSaveCommentStrings, SID_COMMENT_GAME_NAME),
                 tableGetString(sBufferSaveCommentStrings, SID_COMMENT_EMPTY));
}

#if IS_MM
bool fn_8007F440(ErrorIndex iString) {
    ErrorDisplay* pErrorDisplay = &sStringDraw[iString];
    fn_800800EC(pErrorDisplay);
    return true;
}

static inline void fn_8007F474_Inline(ErrorDisplay* pErrorDisplay) {
    s32 nSize;

    if (pErrorDisplay->anim.unk0C != 0 && xlFileGetSize(&nSize, pErrorDisplay->anim.szFileName)) {
        fn_800854B8(SYSTEM_HELP(gpSystem), pErrorDisplay, nSize | 0x30000000);
    }
}

bool fn_8007F474(ErrorIndex iString) {
    ErrorDisplay* pErrorDisplay = &sStringDraw[iString];
    fn_8007F474_Inline(pErrorDisplay);
    return true;
}
#endif

static inline void errorDisplaySetFadeInTimer(ErrorDisplay* pErrorDisplay) {
    s32 i;

#if IS_MM
    fn_800800EC(pErrorDisplay);
#endif

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

#if IS_MM
    lbl_802007B4++;
#endif
}

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

#if IS_MM
    bool var_r27 = false;
    Controller* pController = SYSTEM_CONTROLLER(gpSystem);
    s32 temp_r24 = pController->unk_224;
    s64 nTickStart = OSGetTick();

    if (lbl_802007B0 == 0) {
        GXColor bgColor = {255, 255, 255, 255};
        GXColor textColor = {0, 0, 0, 255};
        OSFatal(textColor, bgColor, lbl_80153DBC[iString]);
    }
#endif

    string.iString = ERROR_NONE;

#if IS_MM
    if (iString == ERROR_INS_SPACE && sStringDraw[iString].unk38 > 1) {
        iString = ERROR_INS_SPACE_PLURAL;
    }

    if (!fn_800631B8(SYSTEM_CONTROLLER(gpSystem), 1)) {
        return false;
    }
#endif

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

#if IS_OOT
        errorDisplayDrawSetup();
#elif IS_MM
        errorDisplayDrawSetup(iString, true);
#endif

        errorDisplayPrint(&string);
        fn_8005F7E4(SYSTEM_HELP(gpSystem));
        simulatorDEMODoneRender();
        nResult = fn_80063688(&string, var_r31 & (var_r31 ^ var_r30));

#if IS_MM
        if (iString == ERROR_NO_CONTROLLER) {
            if (OSTicksToMilliseconds(OSGetTick() - nTickStart) > 4000) {
                s32 var_r20_2;

                for (var_r20_2 = 0; var_r20_2 < 0x78; var_r20_2++) {
                    if (fn_80080A7C(pController) == 0) {
                        VIWaitForRetrace();
                    }
                }

                if (fn_80080A7C(pController) == 0) {
                    if (fn_80080C04(pController, 9) == 0) {
                        return 0;
                    }

                    if (fn_80080A7C(pController) == 0) {
                        // VISetBlack(1);
                        // VIFlush();
                        // VIWaitForRetrace();
                        // OSReturnToMenu();
                        errorDisplayReturnToMenu(&string);
                    }
                }

                if (fn_800518EC()) {
                    return errorDisplayShow(ERROR_BLANK_1);
                }

                nResult = 2;
            }
        }
#endif
    } while (nResult == 0);

#if IS_OOT
    if (fn_800607B0(SYSTEM_HELP(gpSystem), 1)) {
        return nResult != 1;
    }

    return false;
#elif IS_MM
    if (iString == ERROR_INS_INNODE || iString - 18 <= ERROR_INS_SPACE_PLURAL) {
        OSPanic("errordisplay.c", 1685, "Fatal error\n");
    }

    if (nResult != 0) {
        ErrorDisplay* pErrorDisplay;

        pErrorDisplay = &sStringDraw[iString];
        lbl_802007B4--;

        fn_8007F474_Inline(pErrorDisplay);

        if (iString == ERROR_BLANK_1) {
            fn_80073E24(SYSTEM_SOUND(gpSystem));
        }

        if (lbl_802007B4 == 0) {
            var_r27 = true;
        }

        if (var_r27 && fn_800868E4(SYSTEM_HELP(gpSystem), 1) == 0) {
            return false;
        }

        if (!fn_800631B8(pController, temp_r24)) {
            return false;
        }
    }

    return nResult != 1;
#endif
}
