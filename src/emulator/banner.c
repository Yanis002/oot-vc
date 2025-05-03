/**
 * @file banner.c
 *
 * This file implements methods to create, write and delete a banner to the NAND.
 *
 * The banner has a title (usually the emulated game's name), a subtitle (usually an empty string) and an icon.
 */
#include "emulator/banner.h"
#include "emulator/errordisplay.h"
#include "emulator/xlFile.h"
#include "emulator/xlHeap.h"
#include "macros.h"
#include "mem_funcs.h"
#include "revolution/tpl.h"

//! TODO: fix inline oddity
#define NO_INLINE2() \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0;         \
    (void)0

static bool bannerWrite(void);
static s32 bannerDelete(void);
static s32 bannerGetBufferSize(void);
static void fn_8006496C(void);

static u8 lbl_8025C888[] = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
static u8 lbl_8025C890[] = {0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x04, 0x04};

static u8 sBannerBuffer[BANNER_SIZE];
static NANDBanner* sBanner;

static inline void fn_80063F30_Inline2(ErrorIndex eStringIndex) {
    switch (eStringIndex) {
        default:
            break;
        case ERROR_NULL:
            errorDisplayShow(eStringIndex);
            break;
    }
}

static inline void fn_80063F30_Inline3(NANDResult result) {
    volatile struct_80174988* var_r4;

    for (var_r4 = lbl_80174988; var_r4->result != NAND_RESULT_OK; var_r4++) {
        if (var_r4->result == result) {
            fn_80063F30_Inline2(var_r4->eStringIndex);
            break;
        }
    }
}

static inline void fn_80063F30_Inline(NANDResult result) {
    switch (result) {
        default:
            break;
        case NAND_RESULT_OK:
            fn_80063F30_Inline3(result);
            return;
    }
}

static inline s32 __fn_80063F30(char* arg0, u32 arg1) {
    NANDFileInfo nandFileInfo;
    u32 length;
    s32 openResult;

    length = 0;
    openResult = NANDSafeOpen(arg0, &nandFileInfo, 1, sBannerBuffer, sizeof(sBannerBuffer));

    switch (openResult) {
        case NAND_RESULT_AUTHENTICATION:
        case NAND_RESULT_ECC_CRIT:
            return 1;
        case NAND_RESULT_NOEXISTS:
            return 2;
        default:
            fn_80063F30_Inline(openResult);
        case NAND_RESULT_OK:
            fn_80063F30_Inline(NANDGetLength(&nandFileInfo, &length));
            break;
    }

    fn_80063F30_Inline(NANDSafeClose(&nandFileInfo));

    if (length == arg1) {
        return 1;
    }

    return 0;
}

// non-matching
static s32 fn_80063F30(char* szBannerFileName, u32 arg1) {
    NO_INLINE2();
    return __fn_80063F30(szBannerFileName, arg1);
}

s32 fn_800640BC(char* szFileName, u32 arg1, s32 arg2) {
    NANDFileInfo arg10;
    u8 arg8[32] ATTRIBUTE_ALIGN(32);
    s32 var_r31;
    s32 var_r30;
    NANDResult result;

    result = NANDCreate(szFileName, 0x34, 0);
    if (result != NAND_RESULT_OK && result != NAND_RESULT_EXISTS) {
        return 0;
    }

    result = NANDSafeOpen(szFileName, &arg10, 3, sBannerBuffer, sizeof(sBannerBuffer));
    if (result != NAND_RESULT_OK) {
        return 0;
    }

    var_r30 = arg1 >> 5;
    memset(arg8, arg2, sizeof(arg8));
    DCFlushRange(arg8, sizeof(arg8));

    for (var_r31 = 0; var_r31 < var_r30; var_r31++) {
        result = NANDWrite(&arg10, &arg8, sizeof(arg8));
        if (result < NAND_RESULT_OK) {
            break;
        }
    }

    NANDSafeClose(&arg10);
    if (result < NAND_RESULT_OK) {
        return 0;
    }

    return 1;
}

static inline s32 bannerNANDOpen(char* szFileName, NANDFileInfo* info, u8 access, void* buffer, s32 len) {
    if (access & 2) {
        return NANDOpen(szFileName, info, access);
    } else {
        return NANDSafeOpen(szFileName, info, access, buffer, len);
    }

    return NAND_RESULT_OK;
}

s32 fn_800641CC(NANDFileInfo* nandFileInfo, char* szFileName, u32 arg2, s32 arg3, u8 access) {
    s32 sp10[16];
    s32 i;
    u32 temp_r25;
    s32 var_r3;
    s32 var_r3_2;
    u32 spC;
    u32 var_r4_4;
    s32 var_r3_3;

    if (!lbl_8025D130) {
        for (i = 0; i < ARRAY_COUNT(sp10); i++) {
            sp10[i] = 0;
        }

        if (NANDGetHomeDir((char*)sp10) != NAND_RESULT_OK) {
            errorDisplayShow(ERROR_MAX_FILES);
        }

        if (fn_800B48C4((char*)sp10) != NAND_RESULT_OK) {
            errorDisplayShow(ERROR_MAX_FILES);
        }

        lbl_8025D130 = true;
    }

    temp_r25 = ((s32)(arg2 + 0x3FFF) / 16384) << 0xE;

    while (true) {
        lbl_8025D12C = 0;
        fn_8006496C();
        lbl_8025D12C |= var_r3 * 0x10;

        var_r3 = __fn_80063F30(szFileName, temp_r25);

        lbl_8025D12C |= var_r3;

        if (lbl_8025D12C & 0x11) {
            if (!errorDisplayShow(ERROR_SYS_CORRUPT)) {
                return 0;
            }

            if (lbl_8025D12C & 0x10) {
                bannerDelete();
            }

            if ((lbl_8025D12C & 1) && NANDDelete(szFileName) != NAND_RESULT_OK) {
                return 0;
            }
        } else if (lbl_8025D12C & 0x22) {
            var_r3_2 = 0;
            var_r4_4 = 0;

            if (lbl_8025D12C & 0x20) {
                var_r4_4 = 1;
                var_r3_2 = (s32)(((s32)(bannerGetBufferSize() + 0x3FFF) / 16384) << 0xE) / 16384;
            }

            if (lbl_8025D12C & 2) {
                var_r4_4 += 1;
                var_r3_2 += temp_r25;
            }

            sStringDraw[ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU].unk38 = var_r4_4;
            sStringDraw[ERROR_INS_SPACE].unk38 = ((var_r3_2 << 0xE) + 0x1FFFF) / 131072;

            fn_80063F30_Inline(NANDCheck(var_r3_2, var_r4_4, &spC));

            if (spC & 5) {
                if (!errorDisplayShow(ERROR_INS_SPACE)) {
                    return 0;
                }
            } else if (spC & 0xA) {
                if (!errorDisplayShow(ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU)) {
                    return 0;
                }
            } else if ((!(lbl_8025D12C & 0x20) || bannerWrite()) && (lbl_8025D12C & 2)) {
                fn_800640BC(szFileName, temp_r25, arg3);
            }
        } else if (bannerNANDOpen(szFileName, nandFileInfo, access, sBannerBuffer, sizeof(sBannerBuffer)) ==
                   NAND_RESULT_OK) {
            return 1;
        }
    }
}

/**
 * @brief Close the NAND access.
 * @param info Pointer to `NANDFileInfo`.
 * @param access Access type, either safe or unsafe.
 * @return `bool` – Always `true`.
 */
bool bannerNANDClose(NANDFileInfo* info, u8 access) {
    if (access & 2) {
        NANDClose(info);
    } else {
        NANDSafeClose(info);
    }

    return true;
}

static inline void bannerSetString(char* src, wchar_t* dest, s32 max) {
    s32 i;
    s32 nSize;

    nSize = 0;

    for (i = 0; i < max; i++) {
        if (src[0] == 0x00 && src[1] == (char)0xBB) {
            break;
        }

        ((char*)dest)[1] = *src++;
        ((char*)dest)[0] = *src++;

        dest++;
        nSize++;
    }

    if (nSize == 0) {
        *dest++ = ' ';
    }

    *dest = '\0';
}

/**
 * @brief Creates a new banner.
 * @param szGameName The title of the banner, usually the emulated game's name.
 * @param szEmpty The subtitle of the banner, usually an empty string.
 * @return `bool` – `true` on success, `false` on failure.
 */
bool bannerCreate(char* szGameName, char* szEmpty) {
    wchar_t subtitle[BANNER_TITLE_MAX];
    wchar_t title[BANNER_TITLE_MAX];
    TPLPalette* tplPal;

    u32 i;
    NANDBanner* temp_r26;

    xlHeapTake((void**)&sBanner, 0x10000 | 0x70000000);

    temp_r26 = sBanner;

    if (!xlFileLoad("save_banner.tpl", (void**)&tplPal)) {
        return false;
    }

    TPLBind(tplPal);

    memset(temp_r26, 0, BANNER_BUFFER_SIZE);
    memset(&title, 0, 0x80);

    if (szGameName != NULL) {
        bannerSetString(szGameName, title, 31);
    }

    if (szEmpty != NULL) {
        bannerSetString(szEmpty, subtitle, 31);
    }

    NANDInitBanner(temp_r26, 0x10, &title[0], &subtitle[0]);
    memcpy(temp_r26->bannerTexture, tplPal->descriptors[0].texHeader->data, ARRAY_COUNT(temp_r26->bannerTexture));

    for (i = 0; i < 8; i++) {
        memcpy(temp_r26->iconTexture[i * 0x240], tplPal->descriptors[lbl_8025C890[i]].texHeader->data,
               ARRAY_COUNT(temp_r26->iconTexture));
        temp_r26->iconSpeed = (temp_r26->iconSpeed & ~(3 << (i * 2))) | (lbl_8025C888[i] << (i * 2));
    }

    if (i < 8) {
        temp_r26->iconSpeed = (temp_r26->iconSpeed & ~(3 << (i * 2)));
    }

    xlHeapFree((void**)&tplPal);
    return true;
}

/**
 * @brief Writes the banner file to the NAND.
 * @return `bool` – `true` on success, `false` on failure.
 */
static bool bannerWrite(void) {
    NANDFileInfo info;
    void* pBuffer;
    s32 nResult;

    pBuffer = sBanner;

    nResult = NANDCreate("banner.bin", 0x34, 0);
    if (nResult != NAND_RESULT_EXISTS && nResult != NAND_RESULT_OK) {
        return false;
    }

    if (NANDSafeOpen("banner.bin", &info, 3, sBannerBuffer, sizeof(sBannerBuffer))) {
        return false;
    }

    DCFlushRange(pBuffer, BANNER_BUFFER_SIZE);
    nResult = NANDWrite(&info, pBuffer, BANNER_BUFFER_SIZE);
    NANDSafeClose(&info);

    return nResult >= NAND_RESULT_OK;
}

/**
 * @brief Deletes the banner file from the NAND.
 * @return `bool` – `true` on success, `false` on failure.
 */
static s32 bannerDelete(void) { return NANDDelete("banner.bin") == NAND_RESULT_OK; }

/**
 * @brief Returns the buffer size.
 * @return `s32` – The size of the buffer.
 */
static s32 bannerGetBufferSize(void) { return BANNER_BUFFER_SIZE; }

static void fn_8006496C(void) { fn_80063F30("banner.bin", bannerGetBufferSize()); }
