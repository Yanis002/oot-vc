#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/hbm/vcmv/www.h"
#include "revolution/kpad/KPAD.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/os/OSError.h"
#include "revolution/os/OSTime.h"
#include "revolution/types.h"

extern UNKWORD lbl_8025D310;
extern UNKWORD lbl_8025D314;
extern UNKWORD lbl_8025D318;
extern UNKWORD lbl_8025D334;
extern UNKWORD lbl_8025D330;
extern volatile UNKWORD lbl_8025D32C;
extern void* lbl_8025D30C;
extern OSThreadQueue lbl_8025D320;
extern bool lbl_8025D2C0;
extern s32 lbl_8025D2F8;
extern s32 lbl_8025D300;
extern u8 lbl_8025D2F0;
extern s32 lbl_8025D2F4;
extern s32 lbl_8025D304;
extern s32 lbl_8025D2EC;

extern "C" {
void VCMV_8008725C(int index);
char* VCMV_8008844C(void* param1, char* param2, u8 param3);
void fn_800891B4(void);
void fn_80089224(void);
void fn_800889E8(void);

static inline void UnknownInline1(void* block) {
    if (!((uintptr_t)block & 0x30000000)) {
        MEMFreeToAllocator(lbl_8025D2C8, block);
    } else {
        MEMFreeToAllocator(lbl_8025D2CC, block);
    }
}

static inline bool UnknownInline2(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    if (*param1 == NULL) {
        *param1 = MEMAllocFromAllocator(allocator1, size);

        if (*param1 == NULL) {
            *param1 = MEMAllocFromAllocator(allocator2, size);
        }

        if (*param1 == NULL) {
            return false;
        }
    }

    return true;
}

static inline bool UnknownInline3(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    if (!UnknownInline2(param1, size, allocator1, allocator2)) {
        OSReport("AllocIfNecessary size=%p failed\n ", size);
        return false;
    }

    return true;
}

static inline f32 UnknownInline4(f32 param1, f32 param2, UNKWORD param3, UNKWORD param4, f32 param5) {
    if (param3 >= param4) {
        return param2;
    }

    f32 temp = (param2 - param1) * 3.0f / param5;
    return param1 + (temp * param3 * param3 * (param4 - param3 * 0.6666667f));
}

void VCMV_80086E38(volatile UnkStruct_801CA6B0* param1) {
    s32 var_r5;
    s32 var_r6;

    var_r6 = lbl_8025D2D8 - param1->unk_40;
    var_r5 = lbl_8025D2D8 - param1->unk_44;

    if (param1->unk_08 & 0x02) {
        param1->unk_40 = lbl_8025D2D8;

        if (var_r5 < 7) {
            param1->unk_40 -= 7 - var_r5;
        }

        var_r6 = lbl_8025D2D8 - param1->unk_40;
    } else if (param1->unk_0C & 2) {
        param1->unk_44 = lbl_8025D2D8;

        if (var_r6 < 7) {
            param1->unk_44 -= 7 - var_r6;
        }

        var_r5 = lbl_8025D2D8 - param1->unk_44;
    }

    param1->unk_38 = param1->unk_20;

    if (param1->unk_04 & 0x02) {
        UNKWORD unused1 = param1->unk_40;
        UNKWORD unused2 = lbl_8025D2D8;
        param1->unk_34 = UnknownInline4(param1->unk_1C, lbl_8025D248, var_r6, 7, 343.0f);
    } else {
        param1->unk_34 = UnknownInline4(lbl_8025D248, param1->unk_1C, var_r5, 7, 343.0f);
    }
}

void VCMV_80086FDC(volatile UnkStruct_801CA6B0* param1, KPADStatus* param2) {
    param1->unk_1C = (param2->pos.x * lbl_8025C8E8);
    param1->unk_20 = (param2->pos.y * lbl_8025C8EC);
    param1->unk_14 = (param1->unk_1C + lbl_8025C8E8);
    param1->unk_18 = (param1->unk_20 + lbl_8025C8EC);
    param1->unk_2C = param2->horizon.x;
    param1->unk_30 = param2->horizon.y;

    param1->unk_04 = 0x00;

    if (param2->hold & 0x800) {
        param1->unk_04 |= 0x01;
    }

    if (param2->hold & 0x08) {
        param1->unk_04 |= 0x08;
    }

    if (param2->hold & 0x04) {
        param1->unk_04 |= 0x04;
    }

    if (param2->hold & 0x1000) {
        param1->unk_04 |= 0x10;
    }

    if (param2->hold & 0x10) {
        param1->unk_04 |= 0x20;
    }

    if (param2->hold & 0x8000) {
        param1->unk_04 |= 0x40;
    }
}

#pragma push
#pragma gen_fsel on // it won't generate the fsel otherwise
void VCMV_800870E8(volatile UnkStruct_801CA6B0* param1, KPADStatus* param2) {
    f32 temp_f0;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 var_f5;

    temp_f2 = (param1->unk_24 * param2->ex_status.cl.lstick.x) + (param1->unk_28 * -param2->ex_status.cl.lstick.y);

    var_f5 = 1.0f;
    if (temp_f2 > 0.0f) {
        var_f5 += ((0.02f * temp_f2) + var_f5);
        var_f5 = ((2.5f - var_f5) >= 0.0f) ? var_f5 : 2.5f;
    }

    param1->unk_24 = (param2->ex_status.cl.lstick.x * var_f5);
    param1->unk_28 = (-param2->ex_status.cl.lstick.y * var_f5);
    temp_f2_2 = (param1->unk_24 * 4.0f) + param1->unk_1C;
    temp_f0 = (param1->unk_28 * 4.0f) + param1->unk_20;

    param1->unk_34 = temp_f2_2;
    param1->unk_1C = temp_f2_2;
    param1->unk_20 = temp_f0;
    param1->unk_38 = temp_f0;
    param1->unk_14 = (temp_f2_2 + lbl_8025C8E8);
    param1->unk_18 = (param1->unk_20 + lbl_8025C8EC);
    param1->unk_04 = 0;
    param1->unk_30 = -0.2f;
    param1->unk_2C = 0.97899997f;

    if (param2->ex_status.cl.hold & 0x10) {
        param1->unk_04 |= 0x01;
    }

    if (param2->ex_status.cl.hold & 0x01) {
        param1->unk_04 |= 0x08;
    }

    if (param2->ex_status.cl.hold & 0x4000) {
        param1->unk_04 |= 0x04;
    }

    if (param2->ex_status.cl.hold & 0x1000) {
        param1->unk_04 |= 0x10;
    }

    if (param2->ex_status.cl.hold & 0x400) {
        param1->unk_04 |= 0x20;
    }

    if (param2->ex_status.cl.hold & 0x800) {
        param1->unk_04 |= 0x40;
    }
}
#pragma pop

void VCMV_80087654(void) {
    u32 var_r4;

    if (lbl_801CA6B0[lbl_8025D2D4].unk_00 == 0) {
        var_r4 = 0;

        for (int i = 0; i < 4; i++) {
            if (var_r4 < lbl_801CA6B0[i].unk_00) {
                var_r4 = lbl_801CA6B0[i].unk_00;
                lbl_8025D2D4 = i;
            }
        }
    }

    for (int var_r31 = 0; var_r31 < 4; var_r31++) {
        if (var_r31 != lbl_8025D2D4) {
            VCMV_8008725C(var_r31);
            VCMV_8008345C(var_r31);
        }
    }

    VCMV_8008725C(lbl_8025D2D4);
    VCMV_8008345C(lbl_8025D2D4);
}

void VCMV_80087734(s32 param1, s32 param2, s32 param3, s32 param4, volatile UnkStruct_801CA6B0* param5) {
    s32 temp_r0;
    s32 var_r4;

    temp_r0 = lbl_8025D240 - 2;

    if (param2 > temp_r0) {
        param2 = temp_r0;
    } else if (param2 < 2) {
        param2 = 2;
    }

    temp_r0 = lbl_8025D242 - 2;
    if (param3 > temp_r0) {
        param3 = temp_r0;
    } else if (param3 < 2) {
        param3 = 2;
    }

    var_r4 = 0;
    if (param2 == lbl_8025D2F8 && param3 == lbl_8025D2FC) {
        var_r4 = 1;
    }

    if (param1 == 0) {
        if (lbl_8025D2F4 != 0 || var_r4 == 0) {
            if (lbl_8025D2F0 != 0 && lbl_8025D300 != 0) {
                lbl_8025D280 = 0;

                if (lbl_8025D2D8 - lbl_8025D2E8 > 5) {
                    temp_r0 = param3 - lbl_8025D2EC;

                    if ((temp_r0 * temp_r0) > 0xF) {
                        lbl_8025D2EC = param3;
                        lbl_8025D2E8 = lbl_8025D2D8;
                    }
                }
            }

            if ((lbl_8025D300 != 0) && !(param5->unk_04 & 3)) {
                lbl_8025D300 = 0;
                WWWSurfaceMouseEvt(2, param2, param3, 1, 0, 0);
            }

            goto block_29;
        }

        goto end;
    } else if (param1 == 1) {
        if ((lbl_8025D300 == 0) || (var_r4 == 0)) {
            lbl_8025D300 = 1;
            goto block_29;
        }

        goto end;
    } else if (param1 == 2) {
        if (lbl_8025D300 != 0) {
            lbl_8025D300 = 0;
            goto block_29;
        }

        goto end;
    }

block_29:
    WWWSurfaceMouseEvt(param1, param2, param3, param4, 0, 0);
    lbl_8025D2F4 = param1;
    lbl_8025D2F8 = param2;
    lbl_8025D2FC = param3;
    lbl_8025D304 = param4;

end:
    return;
}

void VCMV_80087918(s32 param1, s32 param2, s32 param3, volatile UnkStruct_801CA6B0* param4) {
    s32 var_r31 = param2;
    s32 var_r30 = param3;

    if (param2 > lbl_8025D240 - 0x14) {
        var_r31 = lbl_8025D240 - 0x14;
    } else if (param2 < 2) {
        var_r31 = 2;
    }

    if (param3 > lbl_8025D242 - 2) {
        var_r30 = lbl_8025D242 - 2;
    } else if (param3 < 2) {
        var_r30 = 2;
    }

    if (var_r31 != param2 || var_r30 != param3) {
        VCMV_80087734(0, var_r31, var_r30, 4, param4);
    }

    WWWSurfaceWheelEvt(0, var_r31, var_r30, param1, 0);
}

bool VCMV_80087E34(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    return UnknownInline3(param1, size, allocator1, allocator2);
}

void VCMV_80087EE4(void** param1) {
    if (*param1 != NULL) {
        UnknownInline1(*param1);
        *param1 = NULL;
    }
}

void VCMV_80087F3C(void) {
    const char* sp8[6];
    s32 temp_r3;

    if (WWWSurfaceInit(lbl_8025D240, lbl_8025D242, lbl_8025D240 * 4, 0, lbl_8025D230) != 0) {
        OSPanic(__FILE__, 838, "Failed to initialize WWW");
    }

    if (WWWSurfaceSetFlushCallback((void*)VCMV_800840B4, 0) != 0) {
        OSPanic(__FILE__, 841, "Failed to init flush callback for WWW");
    }

    fn_800889E8();

    sp8[0] = "Wii NTLG PGothic";
    sp8[1] = "Wii NTLG PGothic";
    sp8[4] = "Wii NTLG PGothic";
    sp8[3] = "Wii NTLG PGothic";
    sp8[2] = "Wii NTLG PGothic";
    sp8[5] = "Wii NTLG PGothic";

    const char* temp = "/flash/tmp/opera.arc/opera";
    temp_r3 = WWWCreateBrowser(&lbl_8025D2C4, (void*)VCMV_8008461C, sp8, temp);

    if (temp_r3 != 0) {
        OSReport("Failed to init Opera: %d, %s\n", temp_r3, temp_r3 == -1 ? "OOM" : "Failure");
        WWWSurfaceShutdown();
    } else {
        if (!lbl_8025D2C0) {
            VCMV_800867E4();
        }

        WWWAddProtocol("arc");
        lbl_8025D2C0 = true;
    }
}

void VCMV_80088090(void) { OSWakeupThread(&lbl_8025D320); }

void VCMV_80088654(MEMAllocator* param1, MEMAllocator* param2) {
    lbl_8025D2C8 = param1;
    lbl_8025D2CC = param2;
}

void VCMV_80088660(void) { fn_800891B4(); }

void VCMV_80088664(void) { fn_80089224(); }

void VCMV_80088668(UNKWORD param1, UNKWORD param2) { VCMV_80084A34(param1, param2); }

void VCMV_8008866C(s32 param1, s32 param2) { VCMV_80084AAC(param1, param2); }

void VCMV_80088670(u16 param1) { VCMV_80084A40(param1); }

void VCMV_80088674(void) { VCMV_80084DF4(); }

bool VCMV_80088678(u32 param1) {
    UnknownInline3(&lbl_8025D30C, param1, lbl_8025D2CC, lbl_8025D2C8);

    if (lbl_8025D30C == NULL) {
        return false;
    }

    WWWGetBrowserAllocationFunctions(lbl_8025D30C, param1, &lbl_8025D310, &lbl_8025D314, &lbl_8025D318);
    WWWSetAllocationFunctions(lbl_8025D310, lbl_8025D314, lbl_8025D318, lbl_8025D310, lbl_8025D314, lbl_8025D310,
                              lbl_8025D314);
    return true;
}

void VCMV_8008876C(void) {
    WWWShutdownBrowserAllocationFunctions();

    if (lbl_8025D30C != NULL) {
        UnknownInline1(lbl_8025D30C);
        lbl_8025D30C = NULL;
    }
}

void VCMV_800887C4(void* param1) { VCMV_80084A18(param1); }

char* VCMVRun(void* param1, char* param2, u8 param3) { return VCMV_8008844C(param1, param2, param3); }

void VCMV_800887CC(char* param1) { lbl_8025D228 = param1; }

void VCMV_800887D4(UNKWORD param1) {
    lbl_8025D2BE = true;

    if (lbl_8025D32C > param1) {
        if (param1 >= 0x1E) {
            VCMV_80083070(3);
        }

        lbl_8025D334 = param1;
        lbl_8025D330 = param1;
        lbl_8025D32C = param1;
    }
}

bool VCMV_8008882C(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    return UnknownInline3(param1, size, allocator1, allocator2);
}

void VCMV_800888DC(void** param1) {
    if (*param1 != NULL) {
        UnknownInline1(*param1);
        *param1 = NULL;
    }
}

void VCMV_80088934(void) {
    static UNKWORD lbl_8025D338;

    if (lbl_8025D2D8 == lbl_8025D338) {
        lbl_8025D2D8++;
    }

    lbl_8025D338 = lbl_8025D2D8;

    for (int var_r31 = 0; var_r31 < 4; var_r31++) {
        VCMV_8008725C(var_r31);
    }
}

void VCMV_80088994(void** param1) { VCMV_8008315C(param1); }

BOOL NETGetUniversalCalendar(OSCalendarTime* time) {
    OSTicksToCalendarTime(OSGetTick(), time);
    return true;
}
}
