#include "revolution/arc.h"
#include "revolution/gx.h"
#include "revolution/gx/GXTypes.h"
#include "revolution/gx/GXVert.h"
#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/hbm/vcmv/www.h"
#include "revolution/os/OSCache.h"
#include "revolution/os/OSError.h"
#include "revolution/types.h"

#include <string.h>

extern "C" void fn_80084268();
extern "C" void fn_80087EE4(void* param1);
extern "C" s32 fn_80087E34(void*, s32, s32, s32);

extern u8 lbl_801752F0[];
extern u16 lbl_8025D240;
extern u16 lbl_8025D242;
extern u32* lbl_8025D230;
extern u32* lbl_8025D234;
extern u8 lbl_8025D27D;
extern u8 lbl_8025D27E;
extern UNKWORD lbl_8025D284;
extern volatile u8 lbl_8025D288;
extern volatile u8 lbl_8025D289;
extern u8 lbl_8025D26F;
extern UNKWORD lbl_8025D260;
extern u8 lbl_8025D270;
extern u32 lbl_8025D274;
extern u8 lbl_8025D278;
extern u8 lbl_8025D26D;
extern u8 lbl_8025D271;
extern u8 lbl_8025D27B;
extern u8 lbl_8025D27C;
extern s8 lbl_8025D280;
extern u8 lbl_8025D2C2;
extern u8 lbl_8025D27A;
extern s32 lbl_8025D258;
extern s32 lbl_8025D2E8;
extern u8 lbl_8025D26E;
extern f32 lbl_8025D268;
extern char* lbl_8025D228;
extern char* lbl_8025D22C;
extern u8 lbl_8025D2A4;
extern UNKWORD lbl_8025D2C4;
extern UNKWORD lbl_8025D254;
extern s8 lbl_8025D26C;
extern s8 lbl_8025D2BE;
extern s32 lbl_8025D25C;
extern void* lbl_8025D28C;
extern u16 lbl_8025D23C;
extern u16 lbl_8025D23E;
extern u16 lbl_8025D244;
extern s16 lbl_8025D246;
extern f32 lbl_8025D248;
extern f32 lbl_8025C8E8;
extern f32 lbl_8025D24C;
extern f32 lbl_8025C8EC;
extern f32 lbl_8025D250;
extern bool lbl_8025D279;
extern s32 lbl_8025D2C8;
extern s32 lbl_8025D2CC;
extern s32 lbl_8025D238;
extern s32 lbl_8025C8F0;

char lbl_801CA1C8[256];
UNKWORD lbl_801CA2C8[16];
GXTexObj lbl_801CA308[4];
void* lbl_801CA388[148];
volatile UnkStruct_801CA5D8 lbl_801CA5D8;
ARCHandle lbl_801CA630;

static inline int UnknownInline1(u32 value) {
    return ((((u8*)&value)[1] & 0xF8) << 8) | ((((u8*)&value)[2] & 0xFC) << 3) | ((((u8*)&value)[3] & 0xF8) >> 3);
}

static inline void UnknownInline2(UNKWORD param1) {
    if (param1 == 0) {
        WWWCloseBrowserWindow(lbl_8025D2C4, param1);
    } else {
        WWWSetBrowserWindowRect(param1, &lbl_801CA2C8);
        WWWSetImageMode(param1, 4);
        WWWSetRenderingMode(param1, lbl_8025D254);
        WWWShowBrowserWindow(param1);

        lbl_8025D258 = param1;
        if (param1 != 0) {
            WWWRaiseBrowserWindow(param1);
        }

        WWWSetFocus(param1);
        WWWSurfaceUpdateScreen(0);
    }
}

static inline void UnknownInline3(const char* param1) {
    s32 sp8;

    if (WWWCreateBrowserWindow(lbl_8025D2C4, &sp8, 0) != 0) {
        OSReport("NO MEMORY\n");
    } else {
        if (param1 != NULL) {
            WWWOpenUrl(sp8, param1);
        }

        UnknownInline2(sp8);
    }
}

static inline void UnknownInline4() {
    lbl_8025D271 = 1;
    lbl_8025D27B = 0;
    lbl_8025D26D = 0;

    do {
        lbl_8025D27C = (lbl_8025D27C + 1) % 4;
    } while (lbl_8025D27C == lbl_8025D27E || lbl_8025D27C == lbl_8025D27D);
}

UNKWORD VCMV_800838C0(UnkStruct_800838C0* param1, f32* param2) {
    s32 temp_r28;
    s32 temp_r27;
    s32 temp_r0;
    s32 temp_r12;
    s32 temp2;
    s32 temp;
    u32* var_r10;
    u32* var_r27;
    u32* var_r29;
    s32 var_r3;
    s32 var_r30;
    s32 var_r5;
    u8 temp_r31_2;
    u32* var_r26;
    u32* var_r25;

    temp_r27 = param1->unk_00 + param1->unk_08;
    temp_r28 = param1->unk_04 + param1->unk_0C;
    temp_r0 = (lbl_8025D240 + 0x7F) / 128;
    temp_r12 = param1->unk_00 / 128;
    temp2 = param1->unk_00 & 0x7F;
    temp = temp_r27 & 0x7F;
    var_r10 = lbl_8025D234 + (param1->unk_04 * temp_r0) + temp_r12;
    var_r27 = &lbl_8025D230[(param1->unk_04 * lbl_8025D240) + (temp_r12 * 128)];
    var_r29 = &lbl_8025D230[(param1->unk_04 * lbl_8025D240) + temp_r27];

    var_r30 = 0;
    var_r5 = param1->unk_04;
    var_r3 = param1->unk_0C * ((temp_r27 / 128) - temp_r12 + 1);
    while (var_r5 < temp_r28) {
        temp_r31_2 = lbl_801752F0[var_r5 & 0x7F];
        var_r26 = var_r27 + temp_r31_2;
        var_r25 = var_r10;

        if (temp_r31_2 < temp2) {
            var_r25++;
            var_r26 += 128;
            var_r3--;
        }

        while (var_r26 < var_r29) {
            if (*var_r25 != *var_r26) {
                *var_r25 = *var_r26;
                var_r30++;
            }

            var_r26 += 128;
            var_r25++;
        }

        if (temp_r31_2 >= temp) {
            var_r3--;
        }

        var_r27 += lbl_8025D240;
        var_r29 += lbl_8025D240;
        var_r10 += temp_r0;
        var_r5++;
    }

    if (var_r3 != 0) {
        *param2 = (f32)var_r30 / (f32)var_r3;
    } else {
        *param2 = 0.0f;
    }

    return var_r3;
}

BOOL VCMV_80083A60(UnkStruct_800838C0* param1, s32* param2) {
    u8 sp58[0x4D];
    u8 sp8[0x4D];

    s32 temp10;
    s32 var_r31;
    s32 temp_r0;
    s32 temp3;
    s32 i;
    s32 temp_r22;
    s32 temp_r23;
    s32 temp2;
    u32* var_r29;
    u32* var_r28;
    u32* var_r27;
    s32 other_temp_r27;
    s32 other_temp;
    s32 temptemp3;
    s32 index2;
    s32 k;
    u8 val;
    u32* var_r25;
    u32* var_r24;
    s32 var_r23;
    u32 temp9;

    var_r31 = 0x4D;
    temp10 = param1->unk_04 + param1->unk_0C;
    temp_r0 = lbl_8025D242 - temp10;

    if (var_r31 > temp_r0) {
        var_r31 = temp_r0;
    }

    temp3 = param1->unk_04 + var_r31;

    for (i = 0; i < var_r31; i++) {
        sp58[i] = (u8)(i + 4);
        sp8[i] = 3;
    }

    other_temp_r27 = param1->unk_00 + param1->unk_08;
    temp_r22 = (lbl_8025D240 + 0x7F) / 128;
    temp_r23 = param1->unk_00 / 128;
    temp2 = param1->unk_00 & 0x7F;
    other_temp = other_temp_r27 & 0x7F;
    var_r29 = lbl_8025D234 + (temp3 * temp_r22) + temp_r23;
    var_r28 = &lbl_8025D230[(temp3 * lbl_8025D240) + (temp_r23 * 128)];
    var_r27 = &lbl_8025D230[(temp3 * lbl_8025D240) + other_temp_r27];

    temptemp3 = temp3;

    while (temptemp3 < temp10 + sp58[var_r31 - 1]) {
        val = lbl_801752F0[temptemp3 & 0x7F];
        var_r25 = var_r28 + val;
        var_r24 = var_r29;

        if (val < temp2) {
            var_r24++;
            var_r25 += 128;
        }

        while (var_r25 < var_r27) {
            var_r23 = 0;
            index2 = 0;

            for (k = 0; k < var_r31; k++) {
                sp8[var_r23] = sp8[index2];
                sp58[var_r23] = sp58[index2];

                temp9 = var_r25[-sp58[var_r23] * lbl_8025D240];

                if (*var_r24 != temp9) {
                    sp8[k]--;

                    if (sp8[k] == 0) {
                        var_r23--;
                    }
                }

                var_r23++;
                index2++;
            }

            var_r31 = var_r23;
            var_r25 += 128;
            var_r24++;
        }

        var_r28 += lbl_8025D240;
        var_r27 += lbl_8025D240;
        var_r29 += temp_r22;
        temptemp3++;
    }

    if (var_r31 == 1) {
        *param2 = sp58[0];
        return 1;
    }

    return 0;
}

BOOL VCMV_80083D18(UnkStruct_800838C0* param1, s32* param2) {
    u8 sp58[0x4D];
    u8 sp8[0x4D];

    s32 temp10;
    s32 var_r31;
    s32 temp_r0;
    s32 temp3;
    s32 i;
    s32 temp_r22;
    s32 temp_r23;
    s32 temp2;
    u32* var_r29;
    u32* var_r28;
    u32* var_r27;
    s32 other_temp_r27;
    s32 other_temp;
    s32 temptemp3;
    s32 index2;
    s32 k;
    u8 val;
    u32* var_r25;
    u32* var_r24;
    s32 var_r23;
    u32 temp9;

    var_r31 = 0x4D;
    temp10 = param1->unk_04 + param1->unk_0C;
    temp_r0 = lbl_8025D242 - temp10;

    if (var_r31 > param1->unk_04) {
        var_r31 = param1->unk_04;
    }

    temp3 = param1->unk_04;

    for (i = 0; i < var_r31; i++) {
        sp58[i] = (u8)(i + 4);
        sp8[i] = 3;
    }

    other_temp_r27 = param1->unk_00 + param1->unk_08;
    temp_r22 = (lbl_8025D240 + 0x7F) / 128;
    temp_r23 = param1->unk_00 / 128;
    temp2 = param1->unk_00 & 0x7F;
    other_temp = other_temp_r27 & 0x7F;
    var_r29 = lbl_8025D234 + (temp3 * temp_r22) + temp_r23;
    var_r28 = &lbl_8025D230[(temp3 * lbl_8025D240) + (temp_r23 * 128)];
    var_r27 = &lbl_8025D230[(temp3 * lbl_8025D240) + other_temp_r27];

    temptemp3 = temp3;

    while (temptemp3 < temp10 - sp58[var_r31 - 1]) {
        val = lbl_801752F0[temptemp3 & 0x7F];
        var_r25 = var_r28 + val;
        var_r24 = var_r29;

        if (val < temp2) {
            var_r24++;
            var_r25 += 128;
        }

        while (var_r25 < var_r27) {
            var_r23 = 0;
            index2 = 0;

            for (k = 0; k < var_r31; k++) {
                sp8[var_r23] = sp8[index2];
                sp58[var_r23] = sp58[index2];

                temp9 = var_r25[sp58[var_r23] * lbl_8025D240];

                if (*var_r24 != temp9) {
                    sp8[k]--;

                    if (sp8[k] == 0) {
                        var_r23--;
                    }
                }

                var_r23++;
                index2++;
            }

            var_r31 = var_r23;
            var_r25 += 128;
            var_r24++;
        }

        var_r28 += lbl_8025D240;
        var_r27 += lbl_8025D240;
        var_r29 += temp_r22;
        temptemp3++;
    }

    if (var_r31 == 1) {
        *param2 = -sp58[0];
        return 1;
    }

    return 0;
}

void VCMV_80083FC4(UnkStruct_800838C0* param1, s32 param2) {
    if (lbl_8025D288 == 0) {
        lbl_8025D289 = 1;

        lbl_801CA5D8.unk_00[0].unk_04 = -100000;
        lbl_801CA5D8.unk_00[1].unk_04 = -100000;
        lbl_801CA5D8.unk_00[2].unk_04 = -100000;
        lbl_801CA5D8.unk_00[3].unk_04 = -100000;

        lbl_8025D27E = lbl_8025D27D;
        lbl_801CA5D8.unk_00[lbl_8025D27D].unk_00 = (0, lbl_8025D284);
        lbl_801CA5D8.unk_00[lbl_8025D27D].unk_04 = 0;
        lbl_801CA5D8.unk_38 = lbl_8025D27D;
        lbl_801CA5D8.unk_34 = lbl_8025D27D;
        lbl_801CA5D8.unk_30 = lbl_8025D27D;
        lbl_801CA5D8.unk_4C = 0;
        lbl_801CA5D8.unk_50 = 0;
        lbl_801CA5D8.unk_44 = lbl_801CA5D8.unk_48 = 0.0f;
    }

    lbl_801CA5D8.unk_4C += param2;

    if (lbl_801CA5D8.unk_51 == 0) {
        lbl_801CA5D8.unk_51 = 1;
        lbl_801CA5D8.unk_20.unk_00 = param1->unk_00;
        lbl_801CA5D8.unk_20.unk_04 = param1->unk_04;
        lbl_801CA5D8.unk_20.unk_08 = param1->unk_08;
        lbl_801CA5D8.unk_20.unk_0C = param1->unk_0C;

        if (param2 < 0) {
            lbl_801CA5D8.unk_20.unk_0C -= param2;
            lbl_801CA5D8.unk_20.unk_04 += param2;
            return;
        }

        lbl_801CA5D8.unk_20.unk_0C += param2;
    }
}

void VCMV_800840B4(UnkStruct_800838C0* param1, s32 param2) {
    s32 sp8;

    if (lbl_8025D274 < 1 && lbl_8025D26F == 0) {
        if (lbl_8025D270 != 0) {
            if (lbl_8025D2C2 == 0 || lbl_8025D260 >= 0x0C) {
                lbl_8025D270 = 0;

                if (lbl_8025D278 == 0) {
                    lbl_8025D278 = 1;
                }
            } else {
                lbl_8025D27B = 1;
                return;
            }
        } else if (lbl_8025D280 != 0) {
            if (param2 == 1 && param1->unk_0C > 0x50) {
                lbl_801CA5D8.unk_52 = 0;

                if (lbl_8025D288 != 0 && lbl_801CA5D8.unk_51 != 0) {
                    if ((param1->unk_00 != lbl_801CA5D8.unk_20.unk_00 ||
                         param1->unk_08 != lbl_801CA5D8.unk_20.unk_08)) {
                        lbl_801CA5D8.unk_52 = 1;
                        goto end; //! TODO: fake match?
                    }
                }

                if (lbl_8025D280 == 1) {
                    if (VCMV_80083A60(param1, &sp8)) {
                        VCMV_80083FC4(param1, sp8);
                    }
                } else if (lbl_8025D280 == -1 && VCMV_80083D18(param1, &sp8)) {
                    VCMV_80083FC4(param1, sp8);
                }
            }
        }

    end:
        UnknownInline4();
    }
}

void VCMV_80084268() {
    void* temp_r31;
    u8* var_r3;
    u32 temp_r4;
    u8* var_r5;
    s32 var_r6;
    u16* var_r7;
    u32* var_r8;
    u32* var_r9;
    u32* var_r10;
    u32* var_r11;
    s32 var_r12;

    temp_r31 = lbl_801CA388[lbl_8025D27C];

    lbl_8025D271 = 0;
    lbl_8025D274 = 0;
    var_r3 = (u8*)lbl_8025D230;
    temp_r4 = lbl_8025D240 * 4;
    var_r5 = (u8*)temp_r31;

    for (var_r6 = 0; var_r6 < lbl_8025D242; var_r6 += 4) {
        var_r7 = (u16*)var_r5;
        var_r8 = (u32*)var_r3;
        var_r9 = (u32*)((u8*)var_r8 + temp_r4);
        var_r10 = (u32*)((u8*)var_r9 + temp_r4);
        var_r11 = (u32*)((u8*)var_r10 + temp_r4);

        for (var_r12 = 0; var_r12 < lbl_8025D240; var_r12 += 4) {
            *var_r7++ = UnknownInline1(*var_r8++);
            *var_r7++ = UnknownInline1(*var_r8++);
            *var_r7++ = UnknownInline1(*var_r8++);
            *var_r7++ = UnknownInline1(*var_r8++);

            *var_r7++ = UnknownInline1(*var_r9++);
            *var_r7++ = UnknownInline1(*var_r9++);
            *var_r7++ = UnknownInline1(*var_r9++);
            *var_r7++ = UnknownInline1(*var_r9++);

            *var_r7++ = UnknownInline1(*var_r10++);
            *var_r7++ = UnknownInline1(*var_r10++);
            *var_r7++ = UnknownInline1(*var_r10++);
            *var_r7++ = UnknownInline1(*var_r10++);

            *var_r7++ = UnknownInline1(*var_r11++);
            *var_r7++ = UnknownInline1(*var_r11++);
            *var_r7++ = UnknownInline1(*var_r11++);
            *var_r7++ = UnknownInline1(*var_r11++);
        }

        var_r5 += lbl_8025D240 * 8;
        var_r3 += temp_r4 * 4;
    }

    DCStoreRange(temp_r31, lbl_8025D240 * lbl_8025D242 * 2);
    GXInitTexObj(&lbl_801CA308[lbl_8025D27C], temp_r31, lbl_8025D240, lbl_8025D242, GX_TF_RGB565, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&lbl_801CA308[lbl_8025D27C], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                    GX_ANISO_1);
    lbl_8025D27A = 1;
}

UNKWORD VCMV_8008461C(void* param1, UNKWORD param2, UNKWORD param3, UnkStruct_8008461C_Arg4* param4) {
    void* temp_r31;
    UNKWORD var_r30;
    char* var_r29;
    UNKWORD var_r29_2;
    UNKWORD temp_r3;
    UNKWORD temp_r3_2;

    switch (param3) {
        case 0x06:
            lbl_8025D2E8 = lbl_8025D2D8 - 5;
            lbl_8025D26D = 1;
            lbl_8025D26E = 1;
            lbl_8025D268 = 1.6f;
            break;
        case 0x05:
            strncpy(lbl_801CA1C8, param4->unk_00, sizeof(lbl_801CA1C8) - 1);
            lbl_8025D22C = lbl_801CA1C8;
            lbl_801CA5D8.unk_51 = 0;

            if (lbl_8025D2A4 != 0) {
                lbl_8025D2A4 = 0;
                if (strcmp(lbl_8025D22C, lbl_8025D228) == 0) {
                    VCMV_80083070(0x05);
                } else {
                    VCMV_80083070(0x01);
                }
            }

            lbl_8025D26E = 1;
            lbl_8025D26F = 1;
            lbl_8025D270 = 0;
            break;
        case 0x08:
            lbl_8025D268 = 6.0f / (234.0f - lbl_8025D268);
            break;
        case 0x07: {
            lbl_8025D2E8 = lbl_8025D2D8 - 5;
            lbl_8025D26E = 0;

            if (lbl_8025D26F != 0) {
                lbl_8025D26F = 0;
                lbl_8025D270 = 1;
            }
            if (lbl_8025D2C2 != 0) {
                lbl_8025D2C2--;

                if (lbl_8025D2C2 == 1) {
                    VCMV_80083070(2);
                }
            }

            UnkStruct_800838C0 sp18 = {0x000, 0x000, 0x260, 0x1C8};
            VCMV_800840B4(&sp18, 1);
            break;
        }
        case 0x01:
            return 0;
        case 0x02:
            UnknownInline2(param2);
            return 0;
        case 0x23:
            OSReport("!!WWW OUT OF MEMORY!!\n");
            lbl_8025D2BE = 1;
            VCMV_80083070(3);
            return 0;
        case 0x2E:
            if (lbl_8025D25C == 0) {
                if (stricmp(param4->unk_00, "GOGI Previous Page") == 0) {
                    WWWPrevPage(param2);
                } else if (stricmp(param4->unk_00, "GOGI Next Page") == 0) {
                    WWWNextPage(param2);
                }

                return 0;
            }

            break;
        case 0x2C:
            var_r29 = param4->unk_00 + 5;

            while (*var_r29 == 0x2F) {
                var_r29++;
            }

            if (strcmp(param4->unk_04, "arc") == 0) {
                temp_r3_2 = WWWProtocolSetMimeType(param4->unk_08, "text/html");
                if (temp_r3_2 != 0) {
                    return temp_r3_2;
                }

                if (lbl_8025D28C != 0) {
                    ARCFileInfo sp8;

                    if (ARCOpen(&lbl_801CA630, var_r29, &sp8) != 0) {
                        temp_r31 = ARCGetStartAddrInMem(&sp8);

                        var_r29_2 = 0U;
                        while (var_r29_2 < sp8.size) {
                            var_r30 = sp8.size - var_r29_2;

                            if (var_r30 > 0x7FFF) {
                                var_r30 = 0x7FFF;
                            }

                            temp_r3 = WWWProtocolWrite(param4->unk_08, (u8*)temp_r31 + var_r29_2, var_r30);
                            if (temp_r3 != 0) {
                                ARCClose(&sp8);
                                return temp_r3;
                            }

                            var_r29_2 += var_r30;
                        }

                        ARCClose(&sp8);
                        return WWWProtocolFinished(param4->unk_08);
                    }

                    WWWProtocolFailed(param4->unk_08);
                    return 1;
                }
                break;
            }
            break;
        case 0x34:
            lbl_8025D26C = 1;
            break;
        default:
            break;
    }

    return 0;
}

void VCMV_80084A18(void* param1) {
    lbl_8025D28C = param1;

    if (param1 != 0) {
        ARCInitHandle(param1, &lbl_801CA630);
    }
}

void VCMV_80084A34(UNKWORD param1, UNKWORD param2) {
    lbl_8025D23C = param1;
    lbl_8025D23E = param2;
}

void VCMV_80084A40(u16 param1) {
    lbl_8025D244 = param1;
    lbl_8025D248 = (lbl_8025D240 - param1) * 0.5f;
    lbl_8025D246 = lbl_8025D240 - (s32)(param1 * 0.5f);
}

s32 VCMV_80084AAC(s32 param1, s32 param2) {
    bool var_r31;
    bool var_r30;
    s32 var_r29;
    s32 temp_r26;
    u8 var_r6;

    lbl_8025D240 = (param1 + 3) & ~3;
    lbl_8025D242 = (param2 + 3) & ~3;
    lbl_8025C8E8 = lbl_8025D240 * 0.5f;
    lbl_8025C8EC = lbl_8025D242 * 0.5f;
    lbl_8025D24C = 0.5f / lbl_8025C8E8;
    lbl_8025D250 = 0.5f / lbl_8025C8EC;
    lbl_8025D248 = (lbl_8025D240 - lbl_8025D244) * 0.5f;
    lbl_8025D246 = lbl_8025D240 - (s32)(lbl_8025D244 * 0.5f);
    lbl_8025D27E = 5;

    var_r31 = 1;
    var_r30 = 0;

    if (lbl_8025D27C < 5 && lbl_801CA388[lbl_8025D27C] != NULL) {
        lbl_8025D27D = lbl_8025D27C;
        var_r30 = 1;
        var_r6 = lbl_8025D27C;
    } else {
        lbl_8025D27C = 0;
        lbl_8025D27D = 0;
        var_r6 = 0;
    }

    if (lbl_8025D279 == 0) {
        lbl_8025D27E = var_r6;
        lbl_8025D279 = 1;
    }

    temp_r26 = lbl_8025D240 * lbl_8025D242 * 2;

    for (var_r29 = 0; var_r29 < 4; var_r29++) {
        // why??
        var_r31 &= fn_80087E34(&lbl_801CA388[var_r29], temp_r26, lbl_8025D2CC, lbl_8025D2C8);

        if (var_r31 == 0) {
            goto loop_at_end;
        }

        GXInitTexObj(&lbl_801CA308[var_r29], lbl_801CA388[var_r29], lbl_8025D240, lbl_8025D242, GX_TF_RGB565, GX_CLAMP,
                     GX_CLAMP, 0);
        GXInitTexObjLOD(&lbl_801CA308[var_r29], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    }

    if (var_r30 == 0) {
        memset(lbl_801CA388[lbl_8025D27D], 0, temp_r26);
        DCFlushRange(lbl_801CA388[lbl_8025D27D], temp_r26);
        GXInitTexObj(&lbl_801CA308[lbl_8025D27D], lbl_801CA388[lbl_8025D27D], lbl_8025D240, lbl_8025D242, GX_TF_RGB565,
                     GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(&lbl_801CA308[lbl_8025D27D], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
    }

    lbl_8025D238 = lbl_8025D240 * lbl_8025D242 * 4;
    var_r31 &= fn_80087E34(&lbl_8025D230, lbl_8025D238, lbl_8025D2CC, lbl_8025D2C8);

    if (var_r31 != 0) {
        var_r31 &=
            fn_80087E34(&lbl_8025D234, (((lbl_8025D242 * ((s32)(lbl_8025D240 + 0x7F) / 128)) * 4) + 0x1F) & ~0x1F,
                        lbl_8025D2CC, lbl_8025D2C8);

        if (var_r31 != 0) {
            return 1;
        }

        fn_80087EE4(&lbl_8025D230);
    }

loop_at_end:
    while (var_r29-- != 0) {
        fn_80087EE4(&lbl_801CA388[var_r29]);
    }

    return 0;
}

void VCMV_80084DF4(void) {
    s32 var_r30;

    fn_80087EE4(&lbl_8025D234);
    fn_80087EE4(&lbl_8025D230);

    for (var_r30 = 0; var_r30 < 4; var_r30++) {
        fn_80087EE4(&lbl_801CA388[var_r30]);
    }
}

void VCMV_80084E54(void) {
    Mtx44 sp38;
    Mtx sp8;

    GXSetViewport(0.0f, 0.0f, lbl_8025D23C, lbl_8025D23E, 0.0f, 1.0f);
    GXSetScissor(0, 0, lbl_8025D240, lbl_8025D242);
    C_MTXOrtho(sp38, lbl_8025C8EC, -lbl_8025C8EC, -lbl_8025C8E8, lbl_8025C8E8, 0.0f, -1.0f);
    GXSetProjection(sp38, GX_ORTHOGRAPHIC);
    PSMTXIdentity(sp8);
    GXLoadPosMtxImm(sp8, 3);

    lbl_801CA2C8[0] = 0;
    lbl_801CA2C8[1] = 0;
    lbl_801CA2C8[2] = lbl_8025D240;
    lbl_801CA2C8[3] = lbl_8025D242;

    lbl_8025D270 = 0;
    lbl_8025D26F = 0;
}

void VCMV_80084F34(s32 param1) {
    lbl_8025D254 = param1;
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_CLR_RGBA, GX_RGBA4, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_CLR_RGBA, GX_RGBA4, 8);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    GXSetAlphaUpdate(1);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetNumChans(0);

    GXColor spC = {255, 255, 255, 255};
    GXSetCopyClear(spC, 0x00FFFFFF);

    UnknownInline4();
    lbl_8025D25C = 1;
    UnknownInline3(lbl_8025D22C);
}

void VCMV_800855BC(GXTexObj* param1, GXColor param2) {
    GXClearVtxDesc();
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_CLR_RGBA, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_CLR_RGBA, GX_F32, 0);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    GXSetColorUpdate(GX_ENABLE);
    GXSetAlphaUpdate(GX_ENABLE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetZMode(GX_DISABLE, GX_ALWAYS, GX_DISABLE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXLoadTexObj(param1, GX_TEXMAP0);
    GXSetTevColor(GX_TEVREG0, param2);
}

void VCMV_80085798(const UnkStack20* param1) {
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXPosition3f32(param1->x1, param1->y1, 0.0f);
    GXPosition2f32(0.0f, 0.0f);

    GXPosition3f32(param1->x2, param1->y2, 0.0f);
    GXPosition2f32(0.0f, 1.0f);

    GXPosition3f32(param1->x3, param1->y3, 0.0f);
    GXPosition2f32(1.0f, 1.0f);

    GXPosition3f32(param1->x4, param1->y4, 0.0f);
    GXPosition2f32(1.0f, 0.0f);

    GXEnd();
}

void VCMV_80086670(void) {
    u32 temp_r30 = lbl_8025D258;
    char* temp_r31 = lbl_8025D228;

    UnknownInline3(temp_r31);

    if (lbl_8025D258 != temp_r30) {
        WWWCloseBrowserWindow(lbl_8025D2C4, temp_r30);
    }

    lbl_8025D25C = 0;
}

void VCMV_800867D0(void) { lbl_8025D260 = lbl_8025C8F0; }
