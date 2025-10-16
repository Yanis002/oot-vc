#ifndef _PAK_H
#define _PAK_H

#include "emulator/xlObject.h"
#include "revolution/types.h"
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IS_OOT || IS_MT
typedef struct Pak {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ void* pRAM;
    /* 0x08 */ struct Store* pStore;
} Pak; // size = 0x0C
#elif IS_MM
typedef struct Pak {
    /* 0x00 */ s32 unk_00;
    /* 0x08 */ struct Store* pStore;
    /* 0x04 */ void* pRAM;
} Pak; // size = 0x0C
#endif

bool fn_80044708(Pak* pPak, s32 arg2, u32 nUnknown, void* pBuffer);
bool fn_8004477C(Pak* pPak, s32 arg2, u32 nUnknown, void* pBuffer);
bool pakEvent(Pak* pPak, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassPak;

#ifdef __cplusplus
}
#endif

#endif
