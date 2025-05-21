#include "emulator/xlFileRVL.h"
#include "emulator/code_80083508.h"
#include "emulator/errordisplay.h"
#include "emulator/xlCoreRVL.h"
#include "emulator/xlHeap.h"
#include "revolution/arc.h"
#include "revolution/cnt.h"
#include "revolution/mem.h"
#include "stddef.h"

MEMAllocator gCNTAllocator;
CNTHandle gCNTHandle;

_XL_OBJECTTYPE gTypeFile = {
    "FILE",
    sizeof(tXL_FILE),
    NULL,
    (EventFunc)xlFileEvent,
};

static DVDReadCallback gpfRead;
static DVDOpenCallback gpfOpen;

#if IS_OOT
bool xlFileSetOpen(DVDOpenCallback pfOpen) { return true; }

bool xlFileSetRead(DVDReadCallback pfRead) { return true; }
#endif

static inline bool xlFileGetFile(tXL_FILE** ppFile, char* szFileName) {
    if (gpfOpen != NULL) {
        return gpfOpen(szFileName, (DVDFileInfo*)&(*ppFile)->info);
    } else {
        return simulatorCNTOpenNAND(&gCNTHandle.handleNAND, szFileName, &(*ppFile)->info) == 0;
    }
}

bool xlFileOpen(tXL_FILE** ppFile, XlFileType eType, char* szFileName) {
    if (!xlObjectMake((void**)ppFile, NULL, &gTypeFile)) {
        return false;
    }

    if (xlFileGetFile(ppFile, szFileName)) {
        (*ppFile)->eType = eType;
        (*ppFile)->nSize = contentGetLengthNAND(&(*ppFile)->info);
        (*ppFile)->pData = &(*ppFile)->info;

        return true;
    }

#if IS_OOT
    xlObjectFree((void**)ppFile);
#elif IS_MM || IS_MT
    errorDisplayShow(ERROR_SYS_CORRUPT);
#endif

    return false;
}

#if IS_MT
bool fn_800A1F80(void) { return false; }
#endif

bool xlFileClose(tXL_FILE** ppFile) {
#if IS_MT
    if (!xlObjectTest(*ppFile, &gTypeFile)) {
        SAFE_FAILED("xlFileRVL.c", 200);
        return false;
    }
#endif

    if (!xlObjectFree((void**)ppFile)) {
        SAFE_FAILED("xlFileRVL.c", 201);
        return false;
    }

    return true;
}

bool xlFileGet(tXL_FILE* pFile, void* pTarget, s32 nSizeBytes) {
    s32 nOffset;
    s32 nOffsetExtra;
    s32 nSize;
    s32 nSizeUsed;
    CNTResult result;

    if (pFile->nOffset + nSizeBytes > pFile->nSize) {
        nSizeBytes = pFile->nSize - pFile->nOffset;
    }

    if (nSizeBytes == 0) {
        *(s8*)pTarget = 0xFF;
        return false;
    }

    while (nSizeBytes > 0) {
        if (pFile->unk_24 != -1) {
            nOffsetExtra = pFile->nOffset - pFile->unk_24;

            if (nOffsetExtra < 0x1000) {
                nSizeUsed = 0x1000 - nOffsetExtra;
                if (nSizeUsed > nSizeBytes) {
                    nSizeUsed = nSizeBytes;
                }

                if (!xlHeapCopy(pTarget, (void*)((u8*)pFile->pBuffer + nOffsetExtra), nSizeUsed)) {
                    return false;
                }

                pTarget = (void*)((s32)pTarget + nSizeUsed);
                nSizeBytes -= nSizeUsed;
                pFile->nOffset += nSizeUsed;
            }
        }

        if (nSizeBytes > 0) {
            if (!((s32)pTarget & 0x1F) && (nOffset = pFile->nOffset, (((nOffset & 3) == 0) != 0)) &&
                !(nSizeBytes & 0x1F)) {
                s32 temp_r0;

                if (gpfRead != NULL) {
                    gpfRead((DVDFileInfo*)pFile->pData, pTarget, nSizeBytes, nOffset, NULL);
                } else {
                    result = simulatorCNTReadNAND((CNTFileInfoNAND*)pFile->pData, pTarget, nSizeBytes, nOffset);
#if IS_MM || IS_MT
                    if (result < 0) {
                        errorDisplayShow(ERROR_SYS_CORRUPT);
                    }
#endif
                }

                temp_r0 = pFile->nOffset + nSizeBytes;
                nSizeBytes = 0;
                pFile->nOffset = temp_r0;
            } else {
                nSize = 0x1000;
                nOffset = pFile->nOffset & ~0x3;
                nOffsetExtra = pFile->nSize - nOffset;
                pFile->unk_24 = nOffset;

                if (nOffsetExtra <= 0x1000) {
                    nSize = ALIGN(nOffsetExtra, 0x1F);
                }

                if (gpfRead != NULL) {
                    gpfRead((DVDFileInfo*)pFile->pData, pFile->pBuffer, nSize, nOffset, NULL);
                } else {
                    result = simulatorCNTReadNAND((CNTFileInfoNAND*)pFile->pData, pFile->pBuffer, nSize, nOffset);
#if IS_MM || IS_MT
                    if (result < 0) {
                        errorDisplayShow(ERROR_SYS_CORRUPT);
                    }
#endif
                }
            }
        }
    }

    return true;
}

#if IS_MT
bool fn_800A2260(tXL_FILE* pFile) {
    if (!xlObjectTest(pFile, &gTypeFile)) {
        SAFE_FAILED("xlFileRVL.c", 326);
        return false;
    }

    //! @bug probably meant to return true?
    return false;
}
#endif

bool xlFileSetPosition(tXL_FILE* pFile, s32 nOffset) {
#if IS_MT
    if (!xlObjectTest(pFile, &gTypeFile)) {
        SAFE_FAILED("xlFileRVL.c", 343);
        return false;
    }
#endif

    if ((nOffset >= 0) && (nOffset < pFile->nSize)) {
        pFile->nOffset = nOffset;
        return true;
    }

    return false;
}

static inline bool xlFileEventInline(void) {
    void* buffer;
    void* ret;

    if (!xlHeapTake(&buffer, 0x20000 | 0x70000000)) {
        return false;
    }

    ret = MEMCreateExpHeapEx(buffer, 0x20000, 0);
    if (ret == NULL) {
        return false;
    }

    MEMInitAllocatorForExpHeap(&gCNTAllocator, ret, 4);
    return true;
}

bool xlFileEvent(tXL_FILE* pFile, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 0:
            if (!xlFileEventInline()) {
                return false;
            }
            simulatorCNTInitHandleNAND(5, &gCNTHandle.handleNAND, &gCNTAllocator);
            break;
        case 1:
            simulatorCNTReleaseHandleNAND(&gCNTHandle.handleNAND);
            break;
        case 2:
            pFile->nSize = 0;
            pFile->nOffset = 0;
            pFile->pData = NULL;
            pFile->iBuffer = 0;
            pFile->unk_24 = -1;
            if (!xlHeapTake(&pFile->pBuffer, 0x1000 | 0x30000000)) {
                return false;
            }
            break;
        case 3:
            if (pFile->iBuffer != NULL && !xlHeapFree(&pFile->iBuffer)) {
                return false;
            }
            simulatorCNTCloseNAND(&pFile->info);
            if (!xlHeapFree(&pFile->pBuffer)) {
                return false;
            }
            break;
        case 4:
            break;
        default:
            return false;
    }

    return true;
}
