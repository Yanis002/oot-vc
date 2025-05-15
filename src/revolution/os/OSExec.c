#include "macros.h"
#include "revolution/esp.h"
#include "revolution/ipc.h"
#include "revolution/os.h"
#include "stdio.h"
#include "string.h"

typedef struct AppLoaderStruct {
    // total size: 0x20
    char date[16]; // offset 0x0, size 0x10
    u32 entry; // offset 0x10, size 0x4
    u32 size; // offset 0x14, size 0x4
    u32 rebootSize; // offset 0x18, size 0x4
    u32 reserved2; // offset 0x1C, size 0x4
} AppLoaderStruct;

typedef int (*appGetNextCallback)(void*, u32*, u32*);
typedef void (*appInitCallback)(void (*)(char*));
typedef void* (*appGetEntryCallback)();
typedef void (*AppLoaderCallback)(appInitCallback*, appGetNextCallback*, appGetEntryCallback*);

extern u32 BOOT_REGION_START AT_ADDRESS(0x812FDFF0);
extern u32 BOOT_REGION_END AT_ADDRESS(0x812FDFEC);
extern u8 OS_REBOOT_BOOL AT_ADDRESS(0x800030E2); // unknown function, set to true by __OSReboot
s32 __OSAppLoaderOffset AT_ADDRESS(0x800030F4);

#define MENU_TITLE_ID 0x0000000100000002
#define TICKET_VIEW_SIZE 0xD8

// Force CW to align the file boundary to 64
u8 FORCE_BSS_ALIGN[0xABCD] ATTRIBUTE_ALIGN(64);

static u8 views[0xBD00] ATTRIBUTE_ALIGN(32);

bool __OSInReboot;
static bool Prepared;

static void Run(register void* ptr) NO_INLINE;

// These were actually re(?)implemented in NANDCore/OSExec according to BBA
static s32 _ES_InitLib(s32* fd);
static s32 _ES_GetTicketViews(s32* fd, u64 tid, void* pViews, u32* count);
static s32 _ES_LaunchTitle(s32* fd, u64 tid, void* pViews) NO_INLINE;

static AppLoaderStruct* LoadApploader();
static void* LoadDol(const OSExecParams* params, AppLoaderCallback getInterface);

static inline s32 _ES_InitLib(s32* fd) {
    s32 result;

    // Had to remove fd initialization to match __OSLaunchMenu
    // *fd = -1;

    result = IPC_RESULT_OK;

    *fd = IOS_Open("/dev/es", IPC_OPEN_NONE);
    if (*fd < 0) {
        result = *fd;
    }

    return result;
}

bool PackArgs(void* addr, s32 argc, char* argv[]) {
    s32 numArgs;
    char* bootInfo2;
    char* ptr;
    char** list;
    u32 i;

    bootInfo2 = (char*)addr;
    memset(bootInfo2, 0, 0x2000);

    if (argc == 0) {
        *(u32*)&bootInfo2[8] = 0;
    } else {
        numArgs = argc;
        ptr = bootInfo2 + 0x2000;
        while (--argc >= 0) {
            ptr -= (strlen(argv[argc]) + 1);
            strcpy(ptr, argv[argc]);
            argv[argc] = (char*)(ptr - bootInfo2);
        }

        ptr = bootInfo2 + ((ptr - bootInfo2) & ~3);
        ptr -= 4 * (numArgs + 1);
        list = (char**)ptr;

        for (i = 0; i < numArgs + 1; i++) {
            list[i] = argv[i];
        }

        ptr -= 4;
        *(s32*)ptr = numArgs;
        *(u32*)&bootInfo2[8] = (u32)(ptr - bootInfo2);
    }

    return true;
}

static ASM void Run(register void* ptr) {
#ifdef __MWERKS__ // clang-format off
    fralloc
    bl ICFlashInvalidate
    sync
    isync
    mtctr ptr
    bctr

    frfree
    blr
#endif // clang-format on
}

static void Callback(s32, DVDCommandBlock*) { Prepared = true; }

void __OSGetExecParams(OSExecParams* out) {
    if (OS_DOL_EXEC_PARAMS >= (void*)0x80000000) {
        memcpy(out, OS_DOL_EXEC_PARAMS, sizeof(OSExecParams));
    } else {
        out->valid = 0;
    }
}

void __OSSetExecParams(const OSExecParams* params, OSExecParams* addr) {
    memcpy(addr, params, sizeof(OSExecParams));
    OS_DOL_EXEC_PARAMS = addr;
}

void __OSSetPrepared(bool prepared) { Prepared = prepared; }

void __OSLaunchMenu(void) {
    s32 result;
    void* pviews = &views;
    u32 count = 1;
    s32 fd = -1;
    struct {
        u8 tmp[4];
    } unused = {0xFF, 0xFF, 0xFF, 0};

    if (_ES_InitLib(&fd) != IPC_RESULT_OK) {
        return;
    }

    // Get num ticket views
    result = _ES_GetTicketViews(&fd, MENU_TITLE_ID, NULL, &count);
    if (count != 1 || result != IPC_RESULT_OK) {
        return;
    }

    // Get ticket views
    if (_ES_GetTicketViews(&fd, MENU_TITLE_ID, pviews, &count) != IPC_RESULT_OK) {
        return;
    }

    // Launch title
    if (_ES_LaunchTitle(&fd, MENU_TITLE_ID, pviews) != IPC_RESULT_OK) {
        return;
    }

    while (true) {
        ;
    }
}

static inline s32 _ES_GetTitleId(s32* fd, u64* tidOut) {
    s32 result;
    u64* pTid;
    // TODO: Hacky solution
    u8 tidWork[256] ATTRIBUTE_ALIGN(32);
    u8 vectorWork[32] ATTRIBUTE_ALIGN(32);
    IPCIOVector* pVectors = (IPCIOVector*)vectorWork;

    // Cast is necessary
    if (*fd < 0 || tidOut == ((void*)NULL)) {
        return -1017;
    }

    pTid = (u64*)tidWork;
    pVectors[0].base = pTid;
    pVectors[0].length = sizeof(u64);

    result = IOS_Ioctlv(*fd, ES_IOCTLV_GET_TITLE_ID, 0, 1, pVectors);
    if (result == IPC_RESULT_OK) {
        *tidOut = *pTid;
    }

    return result;
}

void __OSRelaunchTitle(void) {
    s32 result;
    void* pviews = &views;
    u32 count = 1;
    s32 fd = -1;
    u64 tid;

    if (_ES_InitLib(&fd) != IPC_RESULT_OK) {
        return;
    }

    if (_ES_GetTitleId(&fd, &tid) != IPC_RESULT_OK) {
        return;
    }

    result = _ES_GetTicketViews(&fd, MENU_TITLE_ID, NULL, &count);
    if (count != 1 || result != IPC_RESULT_OK) {
        return;
    }

    if (_ES_GetTicketViews(&fd, MENU_TITLE_ID, pviews, &count) != IPC_RESULT_OK) {
        return;
    }

    if (_ES_LaunchTitle(&fd, MENU_TITLE_ID, pviews) != IPC_RESULT_OK) {
        return;
    }

    while (true) {
        ;
    }
}

static inline void ReadDisc(void* addr, s32 length, s32 offset) {
    DVDCommandBlock block;

    DVDReadAbsAsyncPrio(&block, addr, length, offset, NULL, 0);

    while (DVDGetCommandBlockStatus(&block) != 0) {
        if (DVDGetCommandBlockStatus(&block) > 2 || DVDGetCommandBlockStatus(&block) < 0) {
            __OSLaunchMenu();
        }
    }
}

static inline int GetApploaderPosition(void) {
    static s32 apploaderPosition;
    u32* tgcHeader;
    s32 apploaderOffsetInTGC;

    if (apploaderPosition != 0) {
        return apploaderPosition;
    }

    if (__OSAppLoaderOffset != 0) {
        tgcHeader = OSAllocFromMEM1ArenaLo(0x40, 32);
        ReadDisc(tgcHeader, 0x40, __OSAppLoaderOffset);
        apploaderOffsetInTGC = tgcHeader[14];
        apploaderPosition = (__OSAppLoaderOffset + apploaderOffsetInTGC) >> 2;
    } else {
        apploaderPosition = 0x910;
    }
    return apploaderPosition;
}

static inline AppLoaderStruct* LoadApploader(void) {
    AppLoaderStruct* header;

    header = OSAllocFromMEM1ArenaLo(sizeof(AppLoaderStruct), 32);
    ReadDisc(header, sizeof(AppLoaderStruct), GetApploaderPosition());
    ReadDisc((void*)0x81200000, OSRoundUp32B(header->size), GetApploaderPosition() + 0x20);
    ICInvalidateRange((void*)0x81200000, OSRoundUp32B(header->size));

    return header;
}

static inline bool IsNewApploader(AppLoaderStruct* header) {
    return strncmp(header->date, "2004/02/01", 10) > 0 ? true : false;
}

static inline void* LoadDol(const OSExecParams* params, AppLoaderCallback getInterface) {
    appInitCallback appInit;
    appGetNextCallback appGetNext;
    appGetEntryCallback appGetEntry;
    void* addr;
    u32 length;
    u32 offset;
    OSExecParams* paramsWork;

    getInterface(&appInit, &appGetNext, &appGetEntry);
    paramsWork = (OSExecParams*)OSAllocFromMEM1ArenaLo(sizeof(OSExecParams), 1);
    __OSSetExecParams(params, paramsWork);
    appInit((void (*)(char*))OSReport);
    OSSetArenaLo(paramsWork);

    while (appGetNext(&addr, &length, &offset) != 0) {
        ReadDisc(addr, length, offset);
    }

    return appGetEntry();
}

static inline void StartDol(const OSExecParams* params, void* entry) {
    OSExecParams* paramsWork = OSAllocFromMEM1ArenaLo(sizeof(OSExecParams), 1);

    __OSSetExecParams(params, paramsWork);
    PI_HW_REGS[9] = 7;

    OSDisableInterrupts();
    Run(entry);
}

extern void fn_8008CBBC(void);

void __OSBootDolSimple(u32 doloffset, u32 restartCode, void* regionStart, void* regionEnd, bool argsUseDefault,
                       s32 argc, char** argv) {
    OSExecParams* params;
    void* dolEntry;
    AppLoaderStruct* header;

    OSDisableInterrupts();
    params = (OSExecParams*)OSAllocFromMEM1ArenaLo(sizeof(OSExecParams), 1);
    params->valid = true;
    params->restartCode = restartCode;
    params->regionStart = regionStart;
    params->regionEnd = regionEnd;
    params->argsUseDefault = argsUseDefault;

    if (!argsUseDefault) {
        params->argsAddr = OSAllocFromMEM1ArenaLo(0x2000, 1);
        PackArgs(params->argsAddr, argc, argv);
    }

    DVDInit();
    DVDSetAutoInvalidation(true);
    DVDResume();
    __OSSetPrepared(false);
    __DVDPrepareResetAsync(Callback);
    __OSMaskInterrupts(0xFFFFFFF0);
    __OSUnmaskInterrupts(0x10);
    OSEnableInterrupts();

    while (Prepared != true) {}
    fn_8008CBBC();
    header = LoadApploader();

    if (IsNewApploader(header)) {
        if (doloffset == 0xFFFFFFFF) {
            doloffset = (GetApploaderPosition() + 0x20) + header->size;
        }

        params->bootDol = doloffset;
        dolEntry = LoadDol(params, (AppLoaderCallback)header->entry);
        StartDol(params, dolEntry);
    } else {
        BOOT_REGION_START = (u32)regionStart;
        BOOT_REGION_END = (u32)regionEnd;
        OS_REBOOT_BOOL = true;

        ReadDisc((void*)0x81330000, OSRoundUp32B(header->rebootSize), (GetApploaderPosition() + 0x20) + header->size);
        ICInvalidateRange((void*)0x81330000, OSRoundUp32B(header->rebootSize));
        OSDisableInterrupts();
        ICFlashInvalidate();
        Run((void*)0x81330000);
    }
}

void __OSBootDol(u32 doloffset, u32 restartCode, const char** argv) {
    char doloffInString[20];
    s32 argvlen;
    char** argvToPass;
    s32 i;
    void* saveStart;
    void* saveEnd;

    OSGetSaveRegion(&saveStart, &saveEnd);
    sprintf(doloffInString, "%d", doloffset);
    argvlen = 0;

    if (argv != 0) {
        while (argv[argvlen] != 0) {
            argvlen++;
        }
    }

    argvlen++;
    argvToPass = OSAllocFromMEM1ArenaLo((argvlen + 1) * 4, 1);
    *argvToPass = doloffInString;

    for (i = 1; i < argvlen; i++) {
        argvToPass[i] = (char*)argv[i - 1];
    }

    __OSBootDolSimple(-1, restartCode, saveStart, saveEnd, false, argvlen, argvToPass);
}

static s32 _ES_GetTicketViews(s32* fd, u64 tid, void* pViews, u32* count) {
    s32 result;
    // TODO: Hacky solution
    u8 work[0x120] ATTRIBUTE_ALIGN(32);
    IPCIOVector* pVectors = (IPCIOVector*)(work + 0x0);
    u64* pTid = (u64*)(work + 0x20);
    u32* pCount = (u32*)(work + 0x40);

    // Cast is necessary
    if (*fd < 0 || count == ((void*)NULL)) {
        return -1017;
    }

    if ((u32)pViews % 32 != 0) {
        return -1017;
    }

    *pTid = tid;

    // NULL views ptr = get num views
    if (pViews == (void*)NULL) {
        pVectors[0].base = pTid;
        pVectors[0].length = sizeof(u64);
        pVectors[1].base = pCount;
        pVectors[1].length = sizeof(u32);

        result = IOS_Ioctlv(*fd, ES_IOCTLV_GET_NUM_TICKET_VIEWS, 1, 1, pVectors);
        if (result == IPC_RESULT_OK) {
            *count = *pCount;
        }

        return result;
    }

    if (*count == 0) {
        return -1017;
    }

    *pCount = *count;

    pVectors[0].base = pTid;
    pVectors[0].length = sizeof(u64);
    pVectors[1].base = pCount;
    pVectors[1].length = sizeof(u32);
    pVectors[2].base = pViews;
    pVectors[2].length = *count * TICKET_VIEW_SIZE;
    return IOS_Ioctlv(*fd, ES_IOCTLV_GET_TICKET_VIEWS, 2, 1, pVectors);
}

static s32 _ES_LaunchTitle(s32* fd, u64 tid, void* pViews) {
    // TODO: Hacky solution
    u8 tidWork[256] ATTRIBUTE_ALIGN(32);
    u8 vectorWork[32] ATTRIBUTE_ALIGN(32);
    IPCIOVector* pVectors = (IPCIOVector*)vectorWork;
    u64* pTid = (u64*)tidWork;

    if (*fd < 0) {
        return -1017;
    }

    if ((u32)pViews % 32 != 0) {
        return -1017;
    }

    *pTid = tid;

    pVectors[0].base = pTid;
    pVectors[0].length = sizeof(u64);
    pVectors[1].base = pViews;
    pVectors[1].length = TICKET_VIEW_SIZE;

    return IOS_IoctlvReboot(*fd, ES_IOCTLV_LAUNCH_TITLE, 2, 0, pVectors);
}
