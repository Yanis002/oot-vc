#ifndef _STORE_H
#define _STORE_H

#include "emulator/system.h"
#include "emulator/xlObject.h"
#include "revolution/nand.h"
#include "revolution/types.h"
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IS_OOT || IS_MT
typedef struct Store {
    /* 0x00 */ u32 nFileSize;
    /* 0x04 */ char szFileName[12];
    /* 0x10 */ NANDFileInfo nandFileInfo;
    /* 0x9C */ u8* unk_9C;
    /* 0xA0 */ SystemRomType eTypeROM;
    /* 0xA4 */ s32 unk_A4;
    /* 0xA8 */ void* unk_A8;
    /* 0xAC */ void* unk_AC;
    /* 0xB0 */ u32 unk_B0;
    /* 0xB4 */ u32 unk_B4;
    /* 0xB8 */ u8 unk_B8;
    /* 0xB9 */ u8 unk_B9;
    /* 0xBA */ u8 unk_BA;
    /* 0xBB */ u8 unk_BB;
    /* 0xBC */ union {
        NANDResult eResult;
        u32 nSize2;
    };
    /* 0xC0 */ NANDCommandBlock nandCmdBlock;
} Store; // size = 0x188
#elif IS_MM
typedef struct Store {
    /* 0x00 */ u32 nFileSize;
    /* 0x04 */ char szFileName[12];
    /* 0x10 */ u32 unk_04;
    /* 0x14 */ NANDFileInfo nandFileInfo;
    /* 0xA0 */ u8* unk_9C;
    /* 0xA4 */ SystemRomType eTypeROM;
    /* 0xA8 */ s32 unk_A4;
    /* 0xAC */ void* unk_A8;
    /* 0xB0 */ void* unk_AC;
    /* 0xB8 */ s32 unk_B0;
    /* 0xBC */ s32 unk_B4;
    /* 0xC0 */ s64 time;
    /* 0xC8 */ u8 unk_B8;
    /* 0xC9 */ u8 unk_B9;
    /* 0xCA */ u8 unk_BA;
    /* 0xCB */ u8 unk_BB;
    /* 0xCC */ union {
        NANDResult eResult;
        u32 nSize2;
    };
    /* 0xD0 */ NANDCommandBlock nandCmdBlock;
} Store; // size = 0x188
#endif

bool fn_80061770(void** pObject, char* szName, SystemRomType eTypeROM, void* pArgument);
bool storeFreeObject(void** ppObject);
bool fn_80061B88(Store* pStore, void* pHeapTarget, s32 nOffset, s32 nByteCount);
bool fn_80061BC0(Store* pStore, void* pHeapTarget, s32 nOffset, s32 nByteCount);
bool fn_80061DB8(void);
bool storeEvent(Store* pStore, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassStore;

#ifdef __cplusplus
}
#endif

#endif
