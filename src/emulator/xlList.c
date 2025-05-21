#include "emulator/xlList.h"
#include "emulator/xlHeap.h"
#include "revolution/os.h"
#include "macros.h"

static tXL_LIST gListList;

bool xlListMake(tXL_LIST** ppList, s32 nItemSize) {
#if IS_MT
    if (ppList == NULL) {
        SAFE_FAILED("xlList.c", 83);
        return false;
    }

    if (nItemSize <= 0) {
        SAFE_FAILED("xlList.c", 85);
        return false;
    }
#endif

    nItemSize = (nItemSize + 3) & ~3;

    if (xlListMakeItem(&gListList, (void**)ppList)) {
        (*ppList)->nItemCount = 0;
        (*ppList)->nItemSize = nItemSize;
        (*ppList)->pNodeNext = NULL;
        (*ppList)->pNodeHead = NULL;
        return true;
    }

    return false;
}

static inline bool xlListWipe(tXL_LIST* pList) {
    void* pNode;
    void* pNodeNext;

#if IS_MT
    if (!xlListTest(pList)) {
        SAFE_FAILED("xlList.c", 55);
        return false;
    }
#endif

    pNode = pList->pNodeHead;
    while (pNode != NULL) {
        pNodeNext = NODE_NEXT(pNode);
        if (!xlHeapFree(&pNode)) {
            SAFE_FAILED("xlList.c", 60);
            return false;
        }
        pNode = pNodeNext;
    }

    pList->nItemCount = 0;
    pList->pNodeNext = NULL;
    pList->pNodeHead = NULL;
    return true;
}

bool xlListFree(tXL_LIST** ppList) {
#if IS_MT
    if (!xlListTest(*ppList)) {
        SAFE_FAILED("xlList.c", 107);
        return false;
    }
#endif

    if (!xlListWipe(*ppList)) {
        SAFE_FAILED("xlList.c", 110);
        return false;
    }

    if (!xlListFreeItem(&gListList, (void**)ppList)) {
        SAFE_FAILED("xlList.c", 112);
        return false;
    }

    return true;
}

static inline bool xlListTest(tXL_LIST* pList) {
    void* pNode;

    if (pList == &gListList) {
        return true;
    }

    pNode = gListList.pNodeHead;
    while (pNode != NULL) {
        if (pList == (tXL_LIST*)NODE_DATA(pNode)) {
            return true;
        }
        pNode = NODE_NEXT(pNode);
    }

    return false;
}

bool xlListMakeItem(tXL_LIST* pList, void** ppItem) {
    s32 nSize;
    void* pListNode;
    void* pNode;
    void* pNodeNext;
    
#if IS_MT
    if (!xlListTest(pList)) {
        SAFE_FAILED("xlList.c", 164);
        return false;
    }

    if (ppItem == NULL) {
        SAFE_FAILED("xlList.c", 165);
        return false;
    }
#endif

    nSize = pList->nItemSize + 4;
    if (!xlHeapTake(&pListNode, nSize)) {
        // this is necessary to get this function inlined in xlListMake...
#if IS_MT
        SAFE_FAILED("xlList.c", 169);
#endif
        return false;
    }

    NODE_NEXT(pListNode) = NULL;
    *ppItem = NODE_DATA(pListNode);
    pNode = &pList->pNodeHead;
    while (pNode != NULL) {
        pNodeNext = NODE_NEXT(pNode);
        if (pNodeNext == NULL) {
            NODE_NEXT(pNode) = pListNode;
            pList->nItemCount++;
            return true;
        }
        pNode = pNodeNext;
    }

    return false;
}

bool xlListFreeItem(tXL_LIST* pList, void** ppItem) {
    void* pNode;
    void* pNodeNext;

#if IS_MT
    if (!xlListTest(pList)) {
        SAFE_FAILED("xlList.c", 204);
        return false;
    }
#endif

    if (pList->pNodeHead == NULL) {
        return false;
    }

#if IS_MT
    if (ppItem == NULL || *ppItem == NULL) {
        SAFE_FAILED("xlList.c", 210);
        return false;
    }
#endif

    pNode = &pList->pNodeHead;
    while (pNode != NULL) {
        pNodeNext = NODE_NEXT(pNode);
        if (*ppItem == NODE_DATA(pNodeNext)) {
            NODE_NEXT(pNode) = NODE_NEXT(pNodeNext);
            *ppItem = NULL;
            if (!xlHeapFree(&pNodeNext)) {
                SAFE_FAILED("xlList.c", 220);
                return false;
            }
            pList->nItemCount--;
            return true;
        }
        pNode = pNodeNext;
    }

    return false;
}

bool xlListTestItem(tXL_LIST* pList, void* pItem) {
    void* pListNode;

    if (!xlListTest(pList) || pItem == NULL) {
        return false;
    }

    pListNode = pList->pNodeHead;
    while (pListNode != NULL) {
        if (pItem == NODE_DATA(pListNode)) {
            return true;
        }
        pListNode = NODE_NEXT(pListNode);
    }

    return false;
}

bool xlListSetup(void) {
    gListList.nItemCount = 0;
    gListList.nItemSize = sizeof(tXL_LIST);
    gListList.pNodeNext = NULL;
    gListList.pNodeHead = NULL;
    return true;
}

bool xlListReset(void) { return true; }
