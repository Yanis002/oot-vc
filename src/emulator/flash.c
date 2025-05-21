#include "emulator/flash.h"
#include "emulator/ram.h"
#include "emulator/storeRVL.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlHeap.h"
#include "macros.h"

#if IS_MM
_XL_OBJECTTYPE gClassFlash = {
    "FLASH",
    sizeof(Flash),
    NULL,
    (EventFunc)flashEvent,
};

bool flashCopyFLASH(Flash* pFLASH, s32 nOffsetRAM, s32 nOffsetFLASH, s32 nSize) {
    void* pTarget;

    if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pTarget, nOffsetRAM, (u32*)&nSize)) {
        return false;
    }

    //! TODO: ``((s32*)pTarget)[0]`` fake?
    ((s32*)pTarget)[0] = pFLASH->flashStatus;
    switch (pFLASH->flashCommand & 0xFF000000) {
        case 0xE1000000:
            ((s32*)pTarget)[0] = pFLASH->flashStatus;
            ((s32*)pTarget)[1] = 0xC2001E;
            break;
        case 0xF0000000:
            if (!simulatorReadFLASH((nOffsetFLASH * 2) & 0x01FFFFFE, pTarget, nSize)) {
                return false;
            }
            break;
        case 0x0:
        case 0x3C000000:
        case 0x4B000000:
        case 0x78000000:
        case 0xA5000000:
        case 0xB4000000:
        case 0xD2000000:
        default:
            break;
    }

    return true;
}

// ``nOffsetFLASH`` is assumed based on the function above
bool flashTransferFLASH(Flash* pFLASH, s32 nOffsetRAM, s32 nOffsetFLASH, s32 nSize) {
    void* pTarget;
    s32 i;

    if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pTarget, nOffsetRAM, (u32*)&nSize)) {
        return false;
    }

    switch (pFLASH->flashCommand & 0xFF000000) {
        case 0xB4000000:
            for (i = 0; i < nSize; i++) {
                pFLASH->flashBuffer[i] = ((char*)pTarget)[i];
            }
            break;
        case 0x0:
        case 0x3C000000:
        case 0x4B000000:
        case 0x78000000:
        case 0xA5000000:
        case 0xD2000000:
        case 0xE1000000:
        case 0xF0000000:
        default:
            break;
    }

    return true;
}
#endif

static bool flashPut8(Flash* pFLASH, u32 nAddress, s8* pData) { return true; }

static bool flashPut16(Flash* pFLASH, u32 nAddress, s16* pData) { return true; }

static bool flashPut32(Flash* pFLASH, u32 nAddress, s32* pData) {
    void* pRAM;
    char buffer[128];
    u32 nSize;
    s32 i;

#if IS_OOT
    if (*pData == 0) {
        pFLASH->flashStatus = 0;
    }

    switch (*pData & 0xFF000000) {
        case 0xD2000000:
            if (pFLASH->unk_18 == 3) {
                if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pRAM, pFLASH->nOffsetRAM, &nSize)) {
                    return false;
                }
                if (!fn_80061BC0(pFLASH->pStore, pRAM, pFLASH->unk_14, 0x80)) {
                    return false;
                }
            } else if (pFLASH->unk_18 == 4) {
                for (i = 0; i < ARRAY_COUNT(buffer); i++) {
                    buffer[i] = -1;
                }

                if (pFLASH->unk_14 == 0xFFFFFFFF) {
                    for (i = 0; i < pFLASH->pStore->nFileSize; i += 0x80) {
                        if (!fn_80061BC0(pFLASH->pStore, buffer, i, 0x80)) {
                            return false;
                        }
                    }
                } else if (!fn_80061BC0(pFLASH->pStore, buffer, pFLASH->unk_14, 0x80)) {
                    return false;
                }
            }

            pFLASH->unk_18 = 0;
            break;
        case 0x0:
            break;
        case 0xE1000000:
            pFLASH->unk_18 = 1;
            pFLASH->flashStatus = 0x11118001;
            break;
        case 0x3C000000:
            pFLASH->unk_14 = -1;
            break;
        case 0x4B000000:
            pFLASH->unk_14 = (*pData << 7) & 0x7FFFFF80;
            break;
        case 0x78000000:
            pFLASH->flashStatus = 0x11118008;
            pFLASH->unk_18 = 4;
            break;
        case 0xB4000000:
            pFLASH->unk_18 = 3;
            break;
        case 0xA5000000:
            pFLASH->unk_18 = 3;
            pFLASH->unk_14 = (*pData << 7) & 0x7FFFFF80;
            pFLASH->flashStatus = 0x11118004;
            break;
        case 0xF0000000:
            pFLASH->flashStatus = 0x11118004;
            pFLASH->unk_18 = 2;
            break;
        default:
            return false;
    }
#elif IS_MM
    switch (*pData & 0xFF000000) {
        case 0xE1000000:
            pFLASH->flashStatus = 0x11118001;
            break;
        case 0x0:
        case 0xB4000000:
        case 0x3C000000:
        case 0x4B000000:
        case 0xD2000000:
        default:
            break;
        case 0x78000000:
            pFLASH->flashStatus = 0x11118008;
            for (i = 0; i < ARRAY_COUNT(buffer); i++) {
                buffer[i] = 0;
            }
            if ((pFLASH->flashCommand & 0xFF000000) == 0x3C000000) {
                for (i = 0; i < 1024; i++) {
                    if (!simulatorWriteFLASH(i << 7, (u8*)buffer, ARRAY_COUNT(buffer))) {
                        return false;
                    }
                }
            } else {
                if ((pFLASH->flashCommand & 0xFF000000) == 0x4B000000) {
                    if (!simulatorWriteFLASH((pFLASH->flashCommand << 7) & 0x7FFFFF80, (u8*)buffer,
                                             ARRAY_COUNT(buffer))) {
                        return false;
                    }
                }
            }
            break;
        case 0xA5000000:
            pFLASH->flashStatus = 0x11118004;
            if (!simulatorWriteFLASH((*pData << 7) & 0x7FFFFF80, (u8*)pFLASH->flashBuffer, 128)) {
                return false;
            }
            break;
        case 0xF0000000:
            pFLASH->flashStatus = 0x11118004;
            break;
    }
#endif

    pFLASH->flashCommand = *pData;
    return true;
}

static bool flashPut64(Flash* pFLASH, u32 nAddress, s64* pData) { return true; }

static bool flashGet8(Flash* pFLASH, u32 nAddress, s8* pData) { return true; }

static bool flashGet16(Flash* pFLASH, u32 nAddress, s16* pData) { return true; }

static bool flashGet32(Flash* pFLASH, u32 nAddress, s32* pData) {
#if IS_OOT
    if (nAddress == 0x08000000) {
        *pData = pFLASH->flashStatus;
    } else {
        *pData = (nAddress & 0xFFFF) | ((nAddress << 16) & ~0xFFFF);
    }
#elif IS_MM
    switch (pFLASH->flashCommand & 0xFF000000) {
        case 0x0:
        case 0x3C000000:
        case 0x4B000000:
        case 0x78000000:
        case 0xA5000000:
        case 0xB4000000:
        case 0xD2000000:
        case 0xE1000000:
        case 0xF0000000:
            *pData = pFLASH->flashStatus;
            break;
        default:
            break;
    }
#endif

    return true;
}

static bool flashGet64(Flash* pFLASH, u32 nAddress, s64* pData) { return true; }

#if IS_OOT
static bool flashGetBlock(Flash* pFLASH, CpuBlock* pBlock) {
    void* pRAM;

    if ((pBlock->nAddress0 & 0xFF000000) == 0x08000000) {
        if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pRAM, pBlock->nAddress1, &pBlock->nSize)) {
            return false;
        }

        if (pFLASH->unk_18 == 1) {
            *((u32*)pRAM + 0) = pFLASH->flashStatus;
            *((u32*)pRAM + 1) = 0xC2001D;
        } else if (pFLASH->unk_18 == 2) {
            fn_80061B88(pFLASH->pStore, pRAM, pBlock->nAddress0 - 0x08000000, pBlock->nSize);
        }
    } else if ((pBlock->nAddress1 & 0xFF000000) == 0x08000000) {
        pFLASH->nOffsetRAM = pBlock->nAddress0;
    }

    if (pBlock->pfUnknown != NULL && !pBlock->pfUnknown(pBlock, 1)) {
        return false;
    }

    return true;
}
#endif

bool fn_80045260(Flash* pFLASH, s32 arg1, void* arg2) {
    if (pFLASH == NULL) {
        return false;
    }

    if (!fn_80061B88(pFLASH->pStore, arg2, (arg1 * 8) & 0x7F8, 8)) {
        return false;
    }

    return true;
}

bool fn_800452B0(Flash* pFLASH, s32 arg1, void* arg2) {
    if (pFLASH == NULL) {
        return false;
    }

    if (!fn_80061BC0(pFLASH->pStore, arg2, (arg1 * 8) & 0x7F8, 8)) {
        return false;
    }

    return true;
}

bool flashGetSize(Flash* pFLASH, u32* pnFlashSize) {
    *pnFlashSize = pFLASH->nFlashSize;
    return true;
}

static inline bool flashEvent_UnknownInline(Flash* pFLASH, void* pArgument) {
    if (pFLASH->pStore != NULL && !storeFreeObject((void**)&pFLASH->pStore)) {
        return false;
    }

    if ((s32)pArgument < 0x1000) {
        pArgument = (void*)0x1000;
    } else if ((s32)pArgument < 0x4000) {
        pArgument = (void*)0x4000;
    }

    pFLASH->nFlashSize = (u32)pArgument;

    if (!fn_80061770((void**)&pFLASH->pStore, "EEP", gpSystem->eTypeROM, pArgument)) {
        return false;
    }

    return true;
}

bool flashEvent(Flash* pFLASH, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
#if IS_MM
            pFLASH->nFlashSize = (u32)pArgument;
#endif
            pFLASH->flashCommand = 0;
            pFLASH->pStore = NULL;
#if IS_MM
            xlHeapTake((void**)&pFLASH->flashBuffer, 0x40000000 | 0x80);
#endif

            if (!flashEvent_UnknownInline(pFLASH, pArgument)) {
                return false;
            }

#if IS_OOT
            pFLASH->unk_18 = 0;
            pFLASH->nOffsetRAM = -1;
#endif
            break;
        case 3:
#if IS_MM
            xlHeapFree((void**)&pFLASH->flashBuffer);
#endif
            if (pFLASH->pStore != NULL && !storeFreeObject((void**)&pFLASH->pStore)) {
                return false;
            }
            break;
        case 0x1002:
#if IS_OOT
            if (!cpuSetGetBlock(SYSTEM_CPU(gpSystem), pArgument, (GetBlockFunc)flashGetBlock)) {
                return false;
            }
#endif
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)flashPut8, (Put16Func)flashPut16,
                                 (Put32Func)flashPut32, (Put64Func)flashPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)flashGet8, (Get16Func)flashGet16,
                                 (Get32Func)flashGet32, (Get64Func)flashGet64)) {
                return false;
            }
        case 0:
        case 1:
            break;
        case 0x1003:
#if IS_OOT
        case 0x1004:
        case 0x1007:
#endif
            break;
        default:
            return false;
    }

    return true;
}

#if IS_OOT
_XL_OBJECTTYPE gClassFlash = {
    "FLASH",
    sizeof(Flash),
    NULL,
    (EventFunc)flashEvent,
};
#endif
