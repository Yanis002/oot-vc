#include "revolution/gx/GXTexture.h"
#include "revolution/gx/GXTypes.h"
#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/os.h"
#include "revolution/os/OSCache.h"
#include "revolution/types.h"

extern "C" {
extern u8 lbl_801752F0[];
extern u16 lbl_8025D240;
extern u16 lbl_8025D242;
extern u32* lbl_8025D230;
extern u32* lbl_8025D234;

typedef struct UnkStruct_800838C0 {
    /* 00 */ UNKWORD unk_00;
    /* 04 */ UNKWORD unk_04;
    /* 08 */ UNKWORD unk_08;
    /* 0C */ UNKWORD unk_0C;
} UnkStruct_800838C0; // size = 0x10

typedef struct UnkStruct_801CA5D8_00 {
    /* 00 */ UNKWORD unk_00;
    /* 04 */ UNKWORD unk_04;
} UnkStruct_801CA5D8_00; // size = 0x08

typedef struct UnkStruct_801CA5D8 {
    /* 00 */ UnkStruct_801CA5D8_00 unk_00[4];
    /* 20 */ UnkStruct_800838C0 unk_20;
    /* 30 */ UNKWORD unk_30;
    /* 34 */ UNKWORD unk_34;
    /* 38 */ UNKWORD unk_38;
    /* 3C */ UNKWORD unk_3C;
    /* 40 */ f32 unk_40;
    /* 44 */ f32 unk_44;
    /* 48 */ f32 unk_48;
    /* 4C */ UNKWORD unk_4C;
    /* 50 */ UNKWORD unk_50;
    /* 51 */ u8 unk_51;
    /* 52 */ u8 unk_52;
} UnkStruct_801CA5D8;
extern volatile UnkStruct_801CA5D8 lbl_801CA5D8;

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
        lbl_8025D271 = 1;
        lbl_8025D27B = 0;
        lbl_8025D26D = 0;
        do {
            lbl_8025D27C = (lbl_8025D27C + 1) % 4;
        } while (lbl_8025D27C == lbl_8025D27E || lbl_8025D27C == lbl_8025D27D);
    }
}

static inline u16 UnknownInline1(u32 value) {
    u8 a = ((u8*)&value)[1];
    u8 b = ((u8*)&value)[2];
    u8 c = ((u8*)&value)[3];

    return ((a << 8) & 0xF800) | ((b << 3) & 0x7E0) | ((c & 0xF8) >> 3);
}

extern GXTexObj lbl_801CA308[4];
extern void* lbl_801CA388[4];
extern u8 lbl_8025D27A;

// https://decomp.me/scratch/UlEUz
void fn_80084268() {
    void* temp_r31;
    u8* var_r3;
    u32 temp_r4;
    u8* var_r5;
    s32 var_r6;
    u16* var_r7;

    temp_r31 = lbl_801CA388[lbl_8025D27C];

    lbl_8025D271 = 0;
    lbl_8025D274 = 0;
    var_r3 = (u8*)lbl_8025D230;
    temp_r4 = lbl_8025D240 * 4;
    var_r5 = (u8*)temp_r31;

    for (var_r6 = 0; var_r6 < lbl_8025D240; var_r6 += 4) {
        var_r7 = (u16*)var_r5;
        const u32* var_r8 = (const u32*)var_r3;
        const u32* var_r9 = (const u32*)((u8*)var_r8 + temp_r4);
        const u32* var_r10 = (const u32*)((u8*)var_r9 + temp_r4);
        const u32* var_r11 = (const u32*)((u8*)var_r10 + temp_r4);
        s32 var_r12;

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

    DCStoreRange(temp_r31, lbl_8025D242 * lbl_8025D240 * 2);
    GXInitTexObj(&lbl_801CA308[lbl_8025D27C], temp_r31, lbl_8025D240, lbl_8025D242, GX_TF_RGB565, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&lbl_801CA308[lbl_8025D27C], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                    GX_ANISO_1);
    lbl_8025D27A = 1;
}

#pragma use_lmw_stmw off
}
