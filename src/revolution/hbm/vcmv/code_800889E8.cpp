#include "macros.h"
#include "revolution/cnt/cnt.h"
#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/hbm/vcmv/www.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/nand.h"
#include "revolution/os/OSError.h"
#include "revolution/types.h"

#include <stdlib.h>

extern "C" void fn_80089308(UnkStruct_80089308* param1);
extern "C" s32 fn_80100600(UnkStruct_8025D500* param1, UNKWORD param2);
extern "C" UNKWORD fn_80100010(UnkStruct_8025D500* param1, const char* param2);
extern "C" void fn_801005F8(UnkStruct_8025D500* param1, void* param2);

extern u8* lbl_8025D504;
extern u8* lbl_8025D500;

// required otherwise `VCMV_80088A34` won't match...
#undef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(...)
#include "lbl_80175460.inc"

UnkStruct_80176360 lbl_80176360[] = {
    {"WWWSurfaceInit", (void*)&WWWSurfaceInit},
    {"WWWSurfaceNewScreen", (void*)&WWWSurfaceNewScreen},
    {"WWWSurfaceDeleteScreen", (void*)&WWWSurfaceDeleteScreen},
    {"WWWSurfaceResize", (void*)&WWWSurfaceResize},
    {"WWWSurfaceShutdown", (void*)&WWWSurfaceShutdown},
    {"WWWSurfaceSetFlushCallback", (void*)&WWWSurfaceSetFlushCallback},
    {"WWWSurfaceInvalidate", (void*)&WWWSurfaceInvalidate},
    {"WWWSurfaceUpdateScreen", (void*)&WWWSurfaceUpdateScreen},
    {"WWWSurfaceLockArea", (void*)&WWWSurfaceLockArea},
    {"WWWSurfaceUnlockArea", (void*)&WWWSurfaceUnlockArea},
    {"WWWSurfaceMouseEvt", (void*)&WWWSurfaceMouseEvt},
    {"WWWSurfaceWheelEvt", (void*)&WWWSurfaceWheelEvt},
    {"WWWSurfaceKeyboardEvt", (void*)&WWWSurfaceKeyboardEvt},
    {"WWWSurfaceAddFont", (void*)&WWWSurfaceAddFont},
    {"WWWCreateBrowser", (void*)&WWWCreateBrowser},
    {"WWWTerminateBrowser", (void*)&WWWTerminateBrowser},
    {"WWWRunSlice", (void*)&WWWRunSlice},
    {"WWWCreateBrowserWindow", (void*)&WWWCreateBrowserWindow},
    {"WWWCloseBrowserWindow", (void*)&WWWCloseBrowserWindow},
    {"WWWSetBrowserWindowTransparent", (void*)&WWWSetBrowserWindowTransparent},
    {"WWWGetBrowserWindowRect", (void*)&WWWGetBrowserWindowRect},
    {"WWWSetBrowserWindowRect", (void*)&WWWSetBrowserWindowRect},
    {"WWWRaiseBrowserWindow", (void*)&WWWRaiseBrowserWindow},
    {"WWWLowerBrowserWindow", (void*)&WWWLowerBrowserWindow},
    {"WWWShowBrowserWindow", (void*)&WWWShowBrowserWindow},
    {"WWWHideBrowserWindow", (void*)&WWWHideBrowserWindow},
    {"WWWCommitIme", (void*)&WWWCommitIme},
    {"WWWUpdateIme", (void*)&WWWUpdateIme},
    {"WWWPostUrl", (void*)&WWWPostUrl},
    {"WWWOpenUrl", (void*)&WWWOpenUrl},
    {"WWWGetHistoryCount", (void*)&WWWGetHistoryCount},
    {"WWWNextPage", (void*)&WWWNextPage},
    {"WWWPrevPage", (void*)&WWWPrevPage},
    {"WWWMoveInHistory", (void*)&WWWMoveInHistory},
    {"WWWStop", (void*)&WWWStop},
    {"WWWReload", (void*)&WWWReload},
    {"WWWReflow", (void*)&WWWReflow},
    {"WWWSearch", (void*)&WWWSearch},
    {"WWWResetSearch", (void*)&WWWResetSearch},
    {"WWWSetFocus", (void*)&WWWSetFocus},
    {"WWWLoseFocus", (void*)&WWWLoseFocus},
    {"WWWHistory", (void*)&WWWHistory},
    {"WWWClearHistory", (void*)&WWWClearHistory},
    {"WWWGetTrueZoom", (void*)&WWWGetTrueZoom},
    {"WWWSetTrueZoom", (void*)&WWWSetTrueZoom},
    {"WWWGetZoom", (void*)&WWWGetZoom},
    {"WWWSetZoom", (void*)&WWWSetZoom},
    {"WWWGetSecurityMode", (void*)&WWWGetSecurityMode},
    {"WWWGetScroll", (void*)&WWWGetScroll},
    {"WWWSetScroll", (void*)&WWWSetScroll},
    {"WWWSetRenderingMode", (void*)&WWWSetRenderingMode},
    {"WWWGetRenderingMode", (void*)&WWWGetRenderingMode},
    {"WWWSetImageMode", (void*)&WWWSetImageMode},
    {"WWWCreateCertificateManager", (void*)&WWWCreateCertificateManager},
    {"WWWCloseCertificateManager", (void*)&WWWCloseCertificateManager},
    {"WWWGetNumberOfCertificates", (void*)&WWWGetNumberOfCertificates},
    {"WWWSetLanguageEncoding", (void*)&WWWSetLanguageEncoding},
    {"WWWGetDocumentIcon", (void*)&WWWGetDocumentIcon},
    {"WWWGetDocumentIconUrl", (void*)&WWWGetDocumentIconUrl},
    {"WWWClearCookies", (void*)&WWWClearCookies},
    {"WWWGetDocumentSize", (void*)&WWWGetDocumentSize},
    {"WWWSetIntPref", (void*)&WWWSetIntPref},
    {"WWWGetIntPref", (void*)&WWWGetIntPref},
    {"WWWSetStringPref", (void*)&WWWSetStringPref},
    {"WWWGetStringPref", (void*)&WWWGetStringPref},
    {"WWWCommitPrefs", (void*)&WWWCommitPrefs},
    {"WWWSetFocusColors", (void*)&WWWSetFocusColors},
    {"WWWSetScrollbarColors", (void*)&WWWSetScrollbarColors},
    {"WWWSetScrollbarSize", (void*)&WWWSetScrollbarSize},
    {"WWWSetWidgetColors", (void*)&WWWSetWidgetColors},
    {"WWWSetDisabledWidgetColors", (void*)&WWWSetDisabledWidgetColors},
    {"WWWSetButtonWidgetColors", (void*)&WWWSetButtonWidgetColors},
    {"WWWSetUastringExtension", (void*)&WWWSetUastringExtension},
    {"WWWMarkNextItemInDirection", (void*)&WWWMarkNextItemInDirection},
    {"WWWResetNavigation", (void*)&WWWResetNavigation},
    {"WWWClearHighlight", (void*)&WWWClearHighlight},
    {"WWWSetHighlight", (void*)&WWWSetHighlight},
    {"WWWGetActiveLinkType", (void*)&WWWGetActiveLinkType},
    {"WWWGetBrowserAllocationFunctions", (void*)&WWWGetBrowserAllocationFunctions},
    {"WWWShutdownBrowserAllocationFunctions", (void*)&WWWShutdownBrowserAllocationFunctions},
    {"WWWSetAllocationFunctions", (void*)&WWWSetAllocationFunctions},
    {"WWWHTTPCreateHttpLib", (void*)&WWWHTTPCreateHttpLib},
    {"WWWHTTPTerminateHttpLib", (void*)&WWWHTTPTerminateHttpLib},
    {"WWWHTTPSessionRunSlice", (void*)&WWWHTTPSessionRunSlice},
    {"WWWHTTPInitSession", (void*)&WWWHTTPInitSession},
    {"WWWHTTPDeleteSession", (void*)&WWWHTTPDeleteSession},
    {"WWWHTTPSetSessionHeader", (void*)&WWWHTTPSetSessionHeader},
    {"WWWHTTPRemoveSessionHeader", (void*)&WWWHTTPRemoveSessionHeader},
    {"WWWHTTPRemoveAllSessionHeaders", (void*)&WWWHTTPRemoveAllSessionHeaders},
    {"WWWHTTPCreateRequest", (void*)&WWWHTTPCreateRequest},
    {"WWWHTTPDeleteRequest", (void*)&WWWHTTPDeleteRequest},
    {"WWWHTTPSetRequestHeader", (void*)&WWWHTTPSetRequestHeader},
    {"WWWHTTPGetRequestHeader", (void*)&WWWHTTPGetRequestHeader},
    {"WWWHTTPRemoveRequestHeader", (void*)&WWWHTTPRemoveRequestHeader},
    {"WWWHTTPRemoveAllRequestHeaders", (void*)&WWWHTTPRemoveAllRequestHeaders},
    {"WWWHTTPGetResponseHeader", (void*)&WWWHTTPGetResponseHeader},
    {"WWWHTTPGetResponseHeaders", (void*)&WWWHTTPGetResponseHeaders},
    {"WWWHTTPIssue", (void*)&WWWHTTPIssue},
    {"WWWHTTPNbActiveRequests", (void*)&WWWHTTPNbActiveRequests},
    {"WWWHTTPPostBodyData", (void*)&WWWHTTPPostBodyData},
    {"WWWHTTPSetAuthCredentials", (void*)&WWWHTTPSetAuthCredentials},
    {"WWWHTTPRemoveAuthCredentials", (void*)&WWWHTTPRemoveAuthCredentials},
    {"WWWHTTPSetProxy", (void*)&WWWHTTPSetProxy},
    {"WWWHTTPRemoveProxy", (void*)&WWWHTTPRemoveProxy},
    {"WWWHTTPEndLoading", (void*)&WWWHTTPEndLoading},
    {"WWWAddJSPlugin", (void*)&WWWAddJSPlugin},
    {"WWWAddNSPlugin", (void*)&WWWAddNSPlugin},
    {"WWWProtocolWrite", (void*)&WWWProtocolWrite},
    {"WWWProtocolSetMimeType", (void*)&WWWProtocolSetMimeType},
    {"WWWProtocolFinished", (void*)&WWWProtocolFinished},
    {"WWWProtocolFailed", (void*)&WWWProtocolFailed},
    {"WWWAddProtocol", (void*)&WWWAddProtocol},
};

UnkStruct_801CC058 WWW_FONT_FILE_DATA_TABLE__[3];
UnkStruct_801CC088 lbl_801CC088;
NANDFileInfo lbl_801CC094;

const char* lbl_8025C908[] = {"WiiNTLG-Regular.ttc", ""};
const char* lbl_8025C910[] = {"Regular"};

extern "C" {
void fn_800889D8(void);

static inline void UnknownInline1(UnkStruct_8025D500* temp_r30) {
    for (u32 var_r29 = 0; var_r29 < 0x70; var_r29++) {
        *(UNKWORD*)lbl_80176360[var_r29].unk_04 = fn_80100010(temp_r30, lbl_80176360[var_r29].unk_00);
    }
}

void fn_800889D8(void) { OSReport("\nError: call www unlinked function.\n"); }

void fn_800889E8(void) {
    for (UnkStruct_801CC058* var_r31 = WWW_FONT_FILE_DATA_TABLE__; var_r31->unk_00 != nullptr; var_r31++) {
        WWWSurfaceAddFont(var_r31->unk_00);
    }
}

bool fn_80088A34(void) {
    fn_80089308((UnkStruct_80089308*)&lbl_80175460);

    UnkStruct_801CC088* ptr = &lbl_801CC088;

    while (ptr->path != NULL) {
        if (ptr->len != 0) {
            s32 temp_r3 = NANDPrivateCreate(ptr->path, 0x3C, 0);

            if (temp_r3 != NAND_RESULT_OK && temp_r3 != NAND_RESULT_EXISTS) {
                return false;
            }

            if (NANDPrivateOpen(ptr->path, &lbl_801CC094, 0x02) != NAND_RESULT_OK) {
                return false;
            }

            if (NANDWrite(&lbl_801CC094, ptr->buf, ptr->len) != NAND_RESULT_OK) {
                return false;
            }
        } else {
            if (NANDPrivateOpen(ptr->path, &lbl_801CC094, 0x01) != NAND_RESULT_OK) {
                return false;
            }

            ptr->len = (ptr->len + 0x1F) & ~0x1F;

            if (ptr->buf == NULL) {
                OSReport("AllocIfNecessary(%d)\n", ptr->len);

                if (!VCMV_80087E34(&ptr->buf, ptr->len, lbl_8025D2CC, lbl_8025D2C8)) {
                    return false;
                }
            }

            if (NANDRead(&lbl_801CC094, ptr->buf, ptr->len) != NAND_RESULT_OK) {
                return false;
            }
        }

        break;
    }

    return true;
}

void fn_80088B94(u8* param1, UnkStruct_80088B94_Param2* param2, u32 param3) {
    u32 temp_r8;
    u32 temp_r0_2;

    if (param2->unk_1004 == 0) {
        param2->unk_100C = 0;
        param2->unk_1018 = 0;

        temp_r8 = *(u32*)param2->unk_1000;
        param2->unk_1008 =
            ((temp_r8 >> 24) | ((temp_r8 >> 8) & 0xFF00) | ((temp_r8 & 0xFF00) << 8) | (temp_r8 << 24)) >> 8;
        param2->unk_1000 += sizeof(u32);

        if (param2->unk_1008 == 0) {
            temp_r8 = *(u32*)param2->unk_1000;
            param2->unk_1008 =
                (temp_r8 >> 24) | ((temp_r8 >> 8) & 0xFF00) | ((temp_r8 & 0xFF00) << 8) | (temp_r8 << 24);
            param2->unk_1000 += sizeof(u32);
        }
    }

    if (param2->unk_1008 <= 0) {
        return;
    }

    temp_r0_2 = param2->unk_1004 + param3;

    if (param2->unk_1018 != 0) {
        goto block_12;
    }

    while (param2->unk_1004 < temp_r0_2) {
        param2->unk_100C &= 7;

        if (param2->unk_100C == 0) {
            param2->unk_1010 = *param2->unk_1000++;
        }

        while (param2->unk_100C < 8) {
            if (!(param2->unk_1010 & 0x80)) {
                param2->unk_0000[param2->unk_1004 & 0xFFF] = *param2->unk_1000++;
                *param1++ = param2->unk_0000[param2->unk_1004 & 0xFFF];
                param2->unk_1004++;
                param2->unk_1008--;
            } else {
                param2->unk_1018 = (*param2->unk_1000 >> 4) + 3;
                param2->unk_1014 = (*param2->unk_1000++ & 0x0F) << 8;
                param2->unk_1014 = (*param2->unk_1000++ | param2->unk_1014) + 1;
                param2->unk_1008 -= param2->unk_1018;

                do {
                    if (param2->unk_1004 >= temp_r0_2) {
                        return;
                    }

                block_12:
                    param2->unk_0000[param2->unk_1004 & 0xFFF] =
                        param2->unk_0000[(param2->unk_1004 - param2->unk_1014) & 0xFFF];
                    *param1++ = param2->unk_0000[param2->unk_1004 & 0xFFF];
                    param2->unk_1004++;
                } while (--param2->unk_1018 != 0);
            }

            if (param2->unk_1008 <= 0) {
                return;
            }

            param2->unk_1010 = param2->unk_1010 << 1;
            param2->unk_100C++;
        }
    }
}

s32 fn_80088D84(void) {
    UnkStruct_80088B94_Param2 spA0;
    UnkStruct_8025D500 sp48;
    CNTHandleNAND sp20;
    CNTFileInfoNAND sp10;
    u8* spC;
    u8* sp8;
    s32 temp_r4;
    s32 temp_r3_5;
    s32 temp_r29;
    s32 var_r31;
    UnkStruct_8025D500* temp_r30;

    spC = nullptr;
    sp8 = nullptr;

    var_r31 = contentInitHandleNAND(2, &sp20, lbl_8025D2CC);

    if (var_r31 == 0) {
        var_r31 = contentOpenNAND(&sp20, "wwwlib-rvl.lz7", &sp10);

        if (var_r31 == 0) {
            if (VCMV_80087E34((void**)&spC, (contentGetLengthNAND(&sp10) + 0x1F) & ~0x1F, lbl_8025D2CC, lbl_8025D2C8) ==
                0) {
                var_r31 = -0xE12;
            } else {
                var_r31 = contentReadNAND(&sp10, spC, (contentGetLengthNAND(&sp10) + 0x1F) & ~0x1F, 0);

                if (var_r31 > 0) {
                    var_r31 = contentCloseNAND(&sp10);

                    if (var_r31 == 0) {
                        memset(&spA0, 0, sizeof(spA0));
                        spA0.unk_1004 = 0;
                        spA0.unk_1000 = spC;
                        fn_80088B94((u8*)&sp48, &spA0, sizeof(sp48));
                        temp_r3_5 = fn_80100600(&sp48, 2);

                        if (VCMV_80087E34((void**)&lbl_8025D500, (temp_r3_5 + 0x1F) & ~0x1F, lbl_8025D2C8,
                                          lbl_8025D2CC) == 0) {
                            var_r31 = -0xE11;
                        } else {
                            spA0.unk_1004 = 0;
                            spA0.unk_1000 = spC;
                            fn_80088B94(lbl_8025D500, &spA0, temp_r3_5);
                            temp_r30 = (UnkStruct_8025D500*)lbl_8025D500;

                            if (temp_r30->unk_1C != 0) {
                                if (VCMV_80087E34((void**)&lbl_8025D504, temp_r30->unk_1C, lbl_8025D2C8,
                                                  lbl_8025D2CC) == 0) {
                                    var_r31 = -0xE13;
                                    goto block_20; //! TODO: can we get rid of that?
                                }

                                memset(lbl_8025D504, 0, temp_r30->unk_1C);
                            }

                            temp_r29 = 0x902420 - temp_r3_5;
                            if (VCMV_80087E34((void**)&sp8, (temp_r29 + 0x1F) & ~0x1F, lbl_8025D2CC, lbl_8025D2C8) ==
                                0) {
                                var_r31 = -0xE14;
                            } else {
                                fn_80088B94(sp8, &spA0, temp_r29);
                                VCMV_80087EE4((void**)&spC);
                                temp_r4 = ((uintptr_t)sp8 - temp_r3_5) - (uintptr_t)temp_r30;
                                temp_r30->unk_30 += temp_r4;
                                temp_r30->unk_38 += temp_r4;
                                temp_r30->unk_4C += temp_r4;
                                temp_r30->unk_54 += temp_r4;
                                fn_801005F8(temp_r30, lbl_8025D504);

                                if (temp_r30->unk_24 != NULL) {
                                    temp_r30->unk_24();
                                }

                                UnknownInline1(temp_r30);

                                VCMV_80087EE4((void**)&sp8);
                                contentReleaseHandleNAND(&sp20);

                                if (var_r31 == 0) {
                                    lbl_8025D2C1 = 1;
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

block_20:
    VCMV_80087EE4((void**)&sp8);
    VCMV_80087EE4((void**)&lbl_8025D504);
    VCMV_80087EE4((void**)&lbl_8025D500);
    VCMV_80087EE4((void**)&spC);
    return var_r31;
}

s32 fn_80089060(void) {
    CNTHandleNAND sp18;
    CNTFileInfoNAND sp8;
    s32 temp_r3;
    s32 temp_r3_3;
    s32 var_r28;

    temp_r3 = contentInitHandleNAND(3, &sp18, lbl_8025D2CC);
    if (temp_r3 != 0) {
        return temp_r3;
    }

    var_r28 = 0;

    for (int i = 0; *lbl_8025C908[i] != nullptr; i++) {
        temp_r3_3 = contentConvertPathToEntrynumNAND(&sp18, lbl_8025C908[i]);

        if (temp_r3_3 >= 0 && contentFastOpenNAND(&sp18, temp_r3_3, &sp8) == 0) {
            VCMV_80087E34((void**)&WWW_FONT_FILE_DATA_TABLE__[var_r28].unk_08, (contentGetLengthNAND(&sp8) + 0x1F) & ~0x1F,
                          lbl_8025D2CC, lbl_8025D2C8);
            WWW_FONT_FILE_DATA_TABLE__[var_r28].unk_0C = (WWW_FONT_FILE_DATA_TABLE__[var_r28].unk_08 + contentGetLengthNAND(&sp8));

            if (contentReadNAND(&sp8, (void*)WWW_FONT_FILE_DATA_TABLE__[var_r28].unk_08, (contentGetLengthNAND(&sp8) + 0x1F) & ~0x1F,
                                0) > 0) {
                WWW_FONT_FILE_DATA_TABLE__[var_r28].unk_00 = lbl_8025C910[i];

                if (contentCloseNAND(&sp8) == 0) {
                    var_r28++;
                }
            }
        }
    }

    temp_r3 = contentReleaseHandleNAND(&sp18);
    if (temp_r3 != 0) {
        return temp_r3;
    }

    if (var_r28 == 0) {
        return -0xE15;
    }

    return 0;
}

s32 fn_800891B4(void) {
    static bool lbl_8025C914 = true;

    lbl_8025D2C0 = 0;

    if (lbl_8025C914) {
        lbl_8025C914 = false;

        if (fn_80088A34() == 0) {
            return 0;
        }
    }

    s32 temp_r31 = fn_80089060();

    if (fn_80089060() == 0) {
        return fn_80088D84();
    }

    return temp_r31;
}

#pragma push
#pragma opt_unroll_loops off
void fn_80089224(void) {
    lbl_8025D2C1 = 0;

    if (lbl_8025D500 != NULL) {
        if (((UnkStruct_8025D500*)lbl_8025D500)->unk_28 != NULL) {
            ((UnkStruct_8025D500*)lbl_8025D500)->unk_28();
        }

        for (int i = 0; i < 112; i++) {
            *(UNKWORD*)lbl_80176360[i].unk_04 = (UNKWORD)fn_800889D8;
        }

        VCMV_80087EE4((void**)&lbl_8025D504);
    }

    VCMV_80087EE4((void**)&lbl_8025D500);

    for (int i = 0; WWW_FONT_FILE_DATA_TABLE__[i].unk_00; i++) {
        VCMV_80087EE4((void**)&WWW_FONT_FILE_DATA_TABLE__[i].unk_08);
    }
}
#pragma pop

int abs(int __x) { return labs(__x); }
}
