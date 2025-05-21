#include "emulator/xlHeap.h"
#include "macros.h"
#include "revolution/os.h"

s32 gnSizeHeap[HEAP_COUNT];
static u32* gpHeap[HEAP_COUNT];
static u32* gapHeapBlockCache[HEAP_COUNT][11][32];

static u32* gpHeapBlockFirst[HEAP_COUNT];
static u32* gpHeapBlockLast[HEAP_COUNT];
static s32 gnHeapTakeCount[HEAP_COUNT];
static s32 gnHeapFreeCount[HEAP_COUNT];
static s32 gnHeapTakeCacheCount[HEAP_COUNT];

#if IS_MM
s32 gnSizeHeapOS[HEAP_COUNT];
#endif

u32* gnHeapOS[HEAP_COUNT];

#define PADDING_MAGIC 0x1234ABCD

#define FLAG_FREE 0x01000000
#define FLAG_TAKEN 0x02000000

// Blocks have a 32-bit header:
//   copy low bits of size (6 bits) | flags (2 bits) | size (26 bits)
#define MAKE_BLOCK(size, flags) ((size) | ((size) << 26) | (flags))

#define BLOCK_IS_FREE(v) ((v) & FLAG_FREE)
#define BLOCK_IS_TAKEN(v) ((v) & FLAG_TAKEN)
#define BLOCK_SIZE(v) ((s32)((v) & 0xFFFFFF))

//! TODO: these need better names
#define CHKSUM_HI(v) ((u32)((v) >> 26))
#define CHKSUM_LO(v) ((u32)((v) & 0x3F))

static bool __xlHeapGetFree(s32 iHeap, s32* pnFreeBytes) NO_INLINE;

static bool xlHeapBlockCacheGet(s32 iHeap, s32 nSize, u32** ppBlock, s32* pnBlockSize) {
    s32 nBlockCachedSize;
    s32 nBlock;
    s32 nBlockSize;
    s32 nBlockBest;
    s32 nBlockBestSize;
    u32* pBlock;

    if (nSize < 8) {
        nBlockSize = 0;
    } else if (nSize < 16) {
        nBlockSize = 1;
    } else if (nSize < 32) {
        nBlockSize = 2;
    } else if (nSize < 64) {
        nBlockSize = 3;
    } else if (nSize < 128) {
        nBlockSize = 4;
    } else if (nSize < 256) {
        nBlockSize = 5;
    } else if (nSize < 512) {
        nBlockSize = 6;
    } else if (nSize < 1024) {
        nBlockSize = 7;
    } else if (nSize < 4096) {
        nBlockSize = 8;
    } else if (nSize < 8192) {
        nBlockSize = 9;
    } else {
        nBlockSize = 10;
    }

    for (; nBlockSize < 11; nBlockSize++) {
        nBlockBest = -1;
        nBlockBestSize = 0x1000000;
        for (nBlock = 0; nBlock < 32; nBlock++) {
            if ((pBlock = gapHeapBlockCache[iHeap][nBlockSize][nBlock]) != NULL) {
                nBlockCachedSize = BLOCK_SIZE(*pBlock);
                if (nBlockCachedSize < nBlockBestSize && nBlockCachedSize >= nSize) {
                    nBlockBest = nBlock;
                    nBlockBestSize = nBlockCachedSize;
                }
            }
        }

        if (nBlockBest >= 0) {
            *pnBlockSize = nBlockBestSize;
            *ppBlock = gapHeapBlockCache[iHeap][nBlockSize][nBlockBest];
            gapHeapBlockCache[iHeap][nBlockSize][nBlockBest] = NULL;

            gnHeapTakeCacheCount[iHeap]++;
            return true;
        }
    }

    *ppBlock = NULL;
    return false;
}

static s32 xlHeapBlockCacheAdd(s32 iHeap, u32* pBlock) {
    s32 nSize;
    s32 nBlock;
    s32 nBlockSize;
    s32 nBlockCachedSize;
    u32* pBlockCached;

    nSize = BLOCK_SIZE(*pBlock);
    if (nSize == 0) {
        return false;
    }

    if (nSize < 8) {
        nBlockSize = 0;
    } else if (nSize < 16) {
        nBlockSize = 1;
    } else if (nSize < 32) {
        nBlockSize = 2;
    } else if (nSize < 64) {
        nBlockSize = 3;
    } else if (nSize < 128) {
        nBlockSize = 4;
    } else if (nSize < 256) {
        nBlockSize = 5;
    } else if (nSize < 512) {
        nBlockSize = 6;
    } else if (nSize < 1024) {
        nBlockSize = 7;
    } else if (nSize < 4096) {
        nBlockSize = 8;
    } else if (nSize < 8192) {
        nBlockSize = 9;
    } else {
        nBlockSize = 10;
    }

    for (nBlock = 0; nBlock < 32; nBlock++) {
        if ((pBlockCached = gapHeapBlockCache[iHeap][nBlockSize][nBlock]) == NULL ||
            (nBlockCachedSize = BLOCK_SIZE(*pBlockCached)) < nSize) {
            gapHeapBlockCache[iHeap][nBlockSize][nBlock] = pBlock;
            return true;
        }
    }

    return false;
}

static bool xlHeapBlockCacheClear(s32 iHeap, u32* pBlock) {
    s32 nSize;
    s32 nBlock;
    s32 nBlockSize;

    nSize = BLOCK_SIZE(*pBlock);
    if (nSize == 0) {
        return false;
    }

    if (nSize < 8) {
        nBlockSize = 0;
    } else if (nSize < 16) {
        nBlockSize = 1;
    } else if (nSize < 32) {
        nBlockSize = 2;
    } else if (nSize < 64) {
        nBlockSize = 3;
    } else if (nSize < 128) {
        nBlockSize = 4;
    } else if (nSize < 256) {
        nBlockSize = 5;
    } else if (nSize < 512) {
        nBlockSize = 6;
    } else if (nSize < 1024) {
        nBlockSize = 7;
    } else if (nSize < 4096) {
        nBlockSize = 8;
    } else if (nSize < 8192) {
        nBlockSize = 9;
    } else {
        nBlockSize = 10;
    }

    for (nBlock = 0; nBlock < 32; nBlock++) {
        if (gapHeapBlockCache[iHeap][nBlockSize][nBlock] == pBlock) {
            gapHeapBlockCache[iHeap][nBlockSize][nBlock] = NULL;
            return true;
        }
    }

    return false;
}

static bool xlHeapBlockCacheReset(s32 iHeap) {
    s32 nBlockSize;
    u32* pBlock;
    u32 nBlock;

    gnHeapTakeCacheCount[iHeap] = 0;
    gnHeapFreeCount[iHeap] = 0;
    gnHeapTakeCount[iHeap] = 0;

    for (nBlockSize = 0; nBlockSize < 11; nBlockSize++) {
        for (nBlock = 0; nBlock < 32; nBlock++) {
            gapHeapBlockCache[iHeap][nBlockSize][nBlock] = NULL;
        }
    }

    pBlock = gpHeapBlockFirst[iHeap];
    while ((nBlockSize = BLOCK_SIZE(*pBlock)) != 0) {
        if (BLOCK_IS_FREE(*pBlock)) {
            xlHeapBlockCacheAdd(iHeap, pBlock);
        }
        pBlock += nBlockSize + 1;
    }

    return true;
}

static inline bool xlHeapFindUpperBlock(s32 iHeap, s32 nSize, u32** ppBlock, s32* pnBlockSize) {
    s32 nBlockSize;
    u32 nBlock;
    u32* pBlock;
    u32* pBlockBest;
    u32* pBlockNext;

    pBlockBest = NULL;
    pBlock = gpHeapBlockFirst[iHeap];

    while ((u32)pBlock < (u32)gpHeapBlockLast) {
        nBlock = *pBlock;
        nBlockSize = BLOCK_SIZE(nBlock) & 0xFFFFFF;
        if (CHKSUM_LO(nBlock) != CHKSUM_HI(nBlock)) {
            return false;
        }
        if (BLOCK_IS_FREE(nBlock) && nBlockSize >= nSize) {
            pBlockBest = pBlock;
        }
        pBlock += nBlockSize + 1;
    }

    if (pBlockBest == NULL) {
        return false;
    }

    nBlockSize = BLOCK_SIZE(*pBlockBest);
    xlHeapBlockCacheClear(iHeap, pBlockBest);

    if (nBlockSize > nSize + 0x20) {
        pBlockNext = pBlockBest + ((nBlockSize - nSize) - 1);
        *pBlockNext = MAKE_BLOCK(nSize, FLAG_FREE);
        xlHeapBlockCacheAdd(iHeap, pBlockBest);
        *ppBlock = pBlockNext;
        *pnBlockSize = BLOCK_SIZE(*pBlockNext);
    } else {
        *ppBlock = pBlockBest;
        *pnBlockSize = BLOCK_SIZE(*pBlockBest);
    }
    return true;
}

bool xlHeapCompact(s32 iHeap) {
    s32 nCount;
    s32 nBlockLarge;
    s32 nBlockSize;
    s32 nBlockNextSize;
    s32 anBlockLarge[6];
    u32 nBlock;
    u32* pBlock;
    u32* pBlockPrevious;
    u32 nBlockNext;
    u32* pBlockNext;

    pBlockPrevious = NULL;
    pBlock = gpHeapBlockFirst[iHeap];
    while ((nBlockSize = BLOCK_SIZE(*pBlock)) != 0) {
        nBlock = *pBlock;
        pBlockNext = pBlock + 1 + nBlockSize;
        nBlockNext = *pBlockNext;

        if (BLOCK_IS_FREE(nBlock)) {
            for (nCount = 0; nCount < 6; nCount++) {
                if (anBlockLarge[nCount] < nBlockSize) {
                    anBlockLarge[nCount] = nBlockSize;
                }
            }

            nBlockNextSize = BLOCK_SIZE(nBlockNext);
            if (nBlockNextSize != 0 && BLOCK_IS_FREE(nBlockNext)) {
                nBlockLarge = nBlockNextSize + 1;
                nBlockSize += nBlockLarge;
                pBlockNext += nBlockLarge;
            }

            if (pBlockPrevious != NULL && BLOCK_IS_FREE(*pBlockPrevious)) {
                nBlockSize += BLOCK_SIZE(*pBlockPrevious) + 1;
                *pBlockPrevious = MAKE_BLOCK(nBlockSize, FLAG_FREE);
            } else {
                *pBlock = MAKE_BLOCK(nBlockSize, FLAG_FREE);
                pBlockPrevious = pBlock;
            }
        } else {
            pBlockPrevious = pBlock;
        }
        pBlock = pBlockNext;
    }

    xlHeapBlockCacheReset(iHeap);
    return true;
}

#if IS_MT
bool fn_800A3490(void) { return true; }

bool fn_800A34A0(void) { return true; }
#endif

bool xlHeapTake(void** ppHeap, s32 nByteCount) {
    bool bValid;
    s32 iHeap;
    u32 nSizeExtra;
    u32 iTry;
    s32 nSize;
    s32 nBlockSize;
    s32 nBlockNextSize;
    s32 nBlockNextNextSize;
    u32 nBlock;
    u32* pBlock;
    u32* pBlockNext;
    u32* pBlockNextNext;

    bValid = false;
    iHeap = (nByteCount >> 30) & 1;
    *ppHeap = NULL;

    switch (nByteCount & 0x30000000) {
        case 0:
            nSizeExtra = 3;
            break;
        case 0x10000000:
            nSizeExtra = 7;
            break;
        case 0x20000000:
            nSizeExtra = 15;
            break;
        case 0x30000000:
            nSizeExtra = 31;
            break;
        default:
            nSizeExtra = 0;
            break;
    }

    if ((nSize = ((nByteCount & 0x8FFFFFFF) + nSizeExtra) >> 2) < 1) {
        return false;
    }
    if (nSize > 0x01000000) {
        return false;
    }

#if IS_MM
    gnSizeHeapOS[iHeap] += nSize << 2;
#endif

    iTry = 0;
    while (iTry++ < 8) {
        if (xlHeapBlockCacheGet(iHeap, nSize, &pBlock, &nBlockSize)) {
            bValid = true;
        } else {
            pBlock = gpHeapBlockFirst[iHeap];
            while ((u32)pBlock < (u32)gpHeapBlockLast[iHeap]) {
                nBlock = *pBlock;
                nBlockSize = BLOCK_SIZE(nBlock);

                if (CHKSUM_LO(nBlock) != CHKSUM_HI(nBlock)) {
                    return false;
                }

                if (BLOCK_IS_FREE(nBlock) && nBlockSize >= nSize) {
                    bValid = true;
                    break;
                }

                pBlock += nBlockSize + 1;
            }
        }

        if (bValid) {
            if (nSize == nBlockSize - 1) {
                nSize++;
            }

            if (nSize < nBlockSize) {
                pBlockNext = pBlock + nSize + 1;
                nBlockNextSize = nBlockSize - nSize - 1;

                pBlockNextNext = pBlock + nBlockSize + 1;
                if ((nBlockNextNextSize = BLOCK_SIZE(*pBlockNextNext)) != 0 && BLOCK_IS_FREE(*pBlockNextNext)) {
                    xlHeapBlockCacheClear(iHeap, pBlockNextNext);
                    nBlockNextSize += nBlockNextNextSize + 1;
                }

                *pBlockNext = MAKE_BLOCK(nBlockNextSize, FLAG_FREE);
                xlHeapBlockCacheAdd(iHeap, pBlockNext);
            }

            *pBlock = MAKE_BLOCK(nSize, FLAG_TAKEN);
            gnHeapTakeCount[iHeap]++;

            pBlock++;
            while (((u32)pBlock & nSizeExtra) != 0) {
                *pBlock++ = PADDING_MAGIC;
            }

            *ppHeap = pBlock;
            return true;
        }

        if (!xlHeapCompact(iHeap)) {
            return false;
        }
    }

    return false;
}

bool xlHeapFree(void** ppHeap) {
    s32 iHeap;
    s32 nBlockSize;
    s32 nBlockNextSize;
    u32* pBlock;
    u32* pBlockNext;

#if IS_MT
    if (ppHeap == NULL) {
        SAFE_FAILED("xlHeap.c", 801);
        return false;
    }
#endif

    for (iHeap = 0; iHeap < 2; iHeap++) {
        if ((u32)gpHeapBlockFirst[iHeap] <= (u32)*ppHeap && (u32)gpHeapBlockLast[iHeap] >= (u32)*ppHeap) {
            break;
        }
    }

    if (iHeap == 2) {
        return false;
    }

    pBlock = (u32*)*ppHeap - 1;
    while (*pBlock == PADDING_MAGIC) {
        pBlock--;
    }

    nBlockSize = BLOCK_SIZE(*pBlock);
#if IS_MM
    gnSizeHeapOS[iHeap] -= nBlockSize;
#endif

    if (BLOCK_IS_FREE(*pBlock)) {
        return false;
    }

    if (!BLOCK_IS_TAKEN(*pBlock)) {
        return false;
    }

    if (CHKSUM_HI(*pBlock) != CHKSUM_LO(nBlockSize)) {
        return false;
    }

    pBlockNext = pBlock + nBlockSize + 1;
    if ((nBlockNextSize = BLOCK_SIZE(*pBlockNext)) != 0 && BLOCK_IS_FREE(*pBlockNext)) {
        xlHeapBlockCacheClear(iHeap, pBlockNext);
        nBlockSize += nBlockNextSize + 1;
    } else {
#if IS_MM
        OSReport("you fucked up the cache\n");
#endif
    }

    *pBlock = MAKE_BLOCK(nBlockSize, FLAG_FREE);
    xlHeapBlockCacheAdd(iHeap, pBlock);

    gnHeapFreeCount[iHeap]++;
    *ppHeap = NULL;

    return true;
}

bool xlHeapCopy(void* pHeapTarget, void* pHeapSource, s32 nByteCount) {
    u8* pSource8;
    u8* pTarget8;
    u32* pSource32;
    u32* pTarget32;

    pSource32 = (u32*)pHeapSource;
    pTarget32 = (u32*)pHeapTarget;

#if IS_MT
    if (pTarget32 == NULL) {
        SAFE_FAILED("xlHeap.c", 887);
        return false;
    }

    if (pSource32 == NULL) {
        SAFE_FAILED("xlHeap.c", 888);
        return false;
    }

    if (nByteCount < 0) {
        SAFE_FAILED("xlHeap.c", 890);
        return false;
    }
#endif

    if ((u32)pSource32 % 4 == 0 && (u32)pTarget32 % 4 == 0) {
        for (; nByteCount >= 64; nByteCount -= 64) {
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
        }

        for (; nByteCount >= 32; nByteCount -= 32) {
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
            *pTarget32++ = *pSource32++;
        }

        for (; nByteCount >= 4; nByteCount -= 4) {
            *pTarget32++ = *pSource32++;
        }
    }

    pSource8 = (u8*)pSource32;
    pTarget8 = (u8*)pTarget32;
    for (; nByteCount >= 16; nByteCount -= 16) {
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
    }

    for (; nByteCount >= 8; nByteCount -= 8) {
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
        *pTarget8++ = *pSource8++;
    }

    for (; nByteCount > 0; nByteCount--) {
        *pTarget8++ = *pSource8++;
    }

    return true;
}

bool xlHeapFill8(void* pHeap, s32 nByteCount, u32 nData) {
    u8* pnTarget = pHeap;
    s32 nWordCount = nByteCount;

    for (; nWordCount > 16; nWordCount -= 16) {
        pnTarget[0] = nData;
        pnTarget[1] = nData;
        pnTarget[2] = nData;
        pnTarget[3] = nData;
        pnTarget[4] = nData;
        pnTarget[5] = nData;
        pnTarget[6] = nData;
        pnTarget[7] = nData;
        pnTarget[8] = nData;
        pnTarget[9] = nData;
        pnTarget[10] = nData;
        pnTarget[11] = nData;
        pnTarget[12] = nData;
        pnTarget[13] = nData;
        pnTarget[14] = nData;
        pnTarget[15] = nData;
        pnTarget += 16;
    }

    for (; nWordCount > 0; nWordCount--) {
        *pnTarget++ = nData;
    }

    return true;
}

bool xlHeapFill32(void* pHeap, s32 nByteCount, u32 nData) {
    u32* pnTarget = pHeap;
    s32 nWordCount;

#if IS_MT
    if (nByteCount & 3) {
        SAFE_FAILED("xlHeap.c", 1085);
        return false;
    }
#endif

    nWordCount = nByteCount >> 2;

#if IS_MT
    if ((u32)pnTarget & 3) {
        SAFE_FAILED("xlHeap.c", 1089);
        return false;
    }
#endif

    for (; nWordCount > 16; nWordCount -= 16) {
        pnTarget[0] = nData;
        pnTarget[1] = nData;
        pnTarget[2] = nData;
        pnTarget[3] = nData;
        pnTarget[4] = nData;
        pnTarget[5] = nData;
        pnTarget[6] = nData;
        pnTarget[7] = nData;
        pnTarget[8] = nData;
        pnTarget[9] = nData;
        pnTarget[10] = nData;
        pnTarget[11] = nData;
        pnTarget[12] = nData;
        pnTarget[13] = nData;
        pnTarget[14] = nData;
        pnTarget[15] = nData;
        pnTarget += 16;
    }

    for (; nWordCount > 0; nWordCount--) {
        *pnTarget++ = nData;
    }

    return true;
}

static bool __xlHeapGetFree(s32 iHeap, s32* pnFreeBytes) {
    s32 nBlockSize;
    s32 nFree;
    s32 nCount;
    u32* pBlock;
    u32 nBlock;

    if (!xlHeapCompact(iHeap)) {
        SAFE_FAILED("xlHeap.c", 1139);
        return false;
    }

    pBlock = gpHeapBlockFirst[iHeap];
    nFree = 0;
    while (pBlock < gpHeapBlockLast[iHeap]) {
        nBlock = *pBlock;
        nBlockSize = BLOCK_SIZE(nBlock);

        if (CHKSUM_LO(nBlock) != CHKSUM_HI(nBlock)) {
            return false;
        }

        if (BLOCK_IS_FREE(nBlock)) {
            nFree += nBlockSize * 4;
        }

        pBlock += nBlockSize + 1;
    }

#if IS_MT
    if (pBlock != gpHeapBlockLast[iHeap]) {
        SAFE_FAILED("xlHeap.c", 1158);
        return false;
    }
#endif

    *pnFreeBytes = nFree;
    return true;
}

bool xlHeapGetHeap1Free(s32* pnFreeBytes) { return __xlHeapGetFree(0, pnFreeBytes); }

#if IS_MM || IS_MT
bool xlHeapGetHeap2Free(s32* pnFreeBytes) { return __xlHeapGetFree(1, pnFreeBytes); }
#endif

bool xlHeapSetup(void) {
    s32 arenaHi[2];
    s32 arenaLo[2];
    u32 iHeap;
    s32 nSize;

    gnHeapOS[0] = OSGetMEM1ArenaLo();
    arenaHi[0] = ROUND_UP((u32)gnHeapOS[0], 4);
    arenaLo[0] = (u32)OSGetMEM1ArenaHi();

    if (arenaLo[0] - arenaHi[0] > 0x4000000) {
        arenaLo[0] = arenaHi[0] + 0x4000000;
    }

    OSSetMEM1ArenaLo((void*)arenaLo[0]);

    gnHeapOS[1] = OSGetMEM2ArenaLo();
    arenaHi[1] = ROUND_UP((u32)gnHeapOS[1], 4);
    arenaLo[1] = (u32)OSGetMEM2ArenaHi();

    if (arenaLo[1] - arenaHi[1] > 0x4000000) {
        arenaLo[1] = arenaHi[1] + 0x4000000;
    }

    OSSetMEM2ArenaLo((void*)arenaLo[1]);

#if IS_MT
    OSReport("arenaLo_1, 0x%08x |||| arenaHi_1 0x%08x\n", arenaLo[1], arenaHi[1]);
#endif

    nSize = arenaLo[0] - arenaHi[0];
#if IS_MT
    OSReport("Setup: ---- HEAP0 (LO): %dK (%dMB) available ----\n", nSize / 1024, (nSize / 1024) / 1024);
#endif
    gpHeap[0] = (u32*)arenaHi[0];
    gnSizeHeap[0] = nSize;


    nSize = arenaLo[1] - arenaHi[1];
#if IS_MT
    OSReport("Setup: ---- HEAP1 (HI): %dK (%dMB) available ----\n", nSize / 1024, (nSize / 1024) / 1024);
#endif
    gpHeap[1] = (u32*)arenaHi[1];
    gnSizeHeap[1] = nSize;


    for (iHeap = 0; iHeap < 2; iHeap++) {
        s32 nBlockSize = (gnSizeHeap[iHeap] >> 2) - 2;

        gpHeapBlockFirst[iHeap] = (void*)gpHeap[iHeap];
        gpHeapBlockLast[iHeap] = (void*)(gpHeap[iHeap] + nBlockSize + 1);
        *(gpHeapBlockFirst[iHeap]) = MAKE_BLOCK(nBlockSize, FLAG_FREE);
        *(gpHeapBlockLast[iHeap]) = 0;

        xlHeapBlockCacheReset(iHeap);
    }

    return true;
}

bool xlHeapReset(void) {
    OSSetMEM1ArenaLo(gnHeapOS[0]);
    OSSetMEM2ArenaLo(gnHeapOS[1]);
    return true;
}
