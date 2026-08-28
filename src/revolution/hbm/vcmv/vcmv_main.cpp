#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/hbm/vcmv/www.h"
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

extern "C" {
void VCMV_8008725C(int index);
char* VCMV_8008844C(void* param1, char* param2, u8 param3);
void fn_800891B4(void);
void fn_80089224(void);
void VCMV_80087734(UNKWORD, UNKWORD, UNKWORD, UNKWORD, void*);

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

void VCMV_80087918(s32 param1, s32 param2, s32 param3, u8* param4) {
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
