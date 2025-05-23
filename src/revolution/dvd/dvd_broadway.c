#include "macros.h"
#include "revolution/dvd.h"
#include "revolution/esp.h"
#include "revolution/ipc.h"
#include "revolution/os.h"
#include "string.h"

#define DVD_LOW_CTX_MAX 4
#define DVD_LOW_CMD_MAX 4
#define DVD_LOW_CTX_MAGIC 0xFEEBDAED

// Your guess is as good as mine
#define DVD_LOW_CTX_IN_USE(ctx) ((u8)((ctx).inUse != false))

typedef enum {
    DVD_IOCTL_INQUIRY = 0x12,
    DVD_IOCTL_READ_DISK_ID = 0x70,
    DVD_IOCTL_READ = 0x71,
    DVD_IOCTL_PREPARE_COVER_REGISTER = 0x7A,
    DVD_IOCTL_CLEAR_COVER_INTERRUPT = 0x86,
    DVD_IOCTL_RESET = 0x8A,
    DVD_IOCTL_CLOSE_PARTITION = 0x8C,
    DVD_IOCTL_UNENCRYPTED_READ = 0x8D,
    DVD_IOCTL_SEEK = 0xAB,
    DVD_IOCTL_SET_MAX_ROTATION = 0xDD,
    DVD_IOCTL_REQUEST_ERROR = 0xE0,
    DVD_IOCTL_STOP_MOTOR = 0xE3,
    DVD_IOCTL_AUDIO_BUFFER_CONFIG = 0xE4,
} DVDLowIoctl;

typedef enum {
    DVD_IOCTLV_OPEN_PARTITION = 0x8B,
} DVDLowIoctlv;

typedef struct DVDLowContext {
    DVDLowCallback callback; // at 0x0
    UNKWORD WORD_0x4;
    u8 inUse; // at 0x8
    u32 magic; // at 0xC
    u32 id; // at 0x10
    char padding[32 - 0x14];
} DVDLowContext;

typedef struct DVDLowDICommand {
    u8 command; // at 0x0
    u32 arg1; // at 0x4
    u32 arg2; // at 0x8
    u32 arg3; // at 0xC
    u32 arg4; // at 0x10
    u32 arg5; // at 0x14
    u32 arg6; // at 0x18
    u32 arg7; // at 0x1C
} DVDLowDICommand;

typedef struct DVDLowDiRegs {
    u32 diimmbuf; // at 0x0
    u32 dicvr; // at 0x4
    char padding[32 - 0x8];
} DVDLowDiRegs;

typedef struct DVDLowRegBuffer {
    u32 reg; // at 0x0
    char padding[32 - 0x4];
} DVDLowRegBuffer;

static s32 DiFD = -1;

static u8 breakRequested;
static DVDLowDICommand* diCommand;
static char* pathBuf;
static u32 readLength;
static bool spinUpValue;
static u8 DVDLowInitCalled;
static u8 dvdContextsInited;
static s32 freeDvdContext;
static s32 freeCommandBuf;
static u8 callbackInProgress;
static u8 requestInProgress;

static DVDLowContext dvdContexts[DVD_LOW_CTX_MAX] ATTRIBUTE_ALIGN(32);
static DVDLowDiRegs diRegValCache ATTRIBUTE_ALIGN(32);
static DVDLowRegBuffer registerBuf ATTRIBUTE_ALIGN(32);
static IPCIOVector ioVec[5] ATTRIBUTE_ALIGN(32);
static u8 lastTicketError[32] ATTRIBUTE_ALIGN(32);

FORCE_ACTIVE(dvd_broadway, dvdContexts);

static void nextCommandBuf(void);
static DVDLowContext* newContext(DVDLowCallback callback, UNKWORD arg2);

static void* ddrAllocAligned32(size_t size) {
    u8* lo = IPCGetBufferLo();
    u8* hi = IPCGetBufferHi();

    // @bug Incorrect rounding (& 31) results in pointer mod 32
    if ((u32)lo % 32 != 0) {
        lo = (u8*)(((u32)lo + 31) & 31);
    }

    if (lo + size > hi) {
        OSReport("(ddrAllocAligned32) Not enough space to allocate %d bytes\n", size);
    }

    IPCSetBufferLo(lo + size);
    return lo;
}

static bool allocateStructures(void) {
    diCommand = ddrAllocAligned32(sizeof(DVDLowDICommand) * DVD_LOW_CMD_MAX);
    if (diCommand == 0) {
        OSReport("Allocation of diCommand blocks failed\n");
        return false;
    }

    pathBuf = ddrAllocAligned32(32);
    if (pathBuf == 0) {
        OSReport("Allocation of pathBuf failed\n");
        return false;
    }

    return true;
}

static void initDvdContexts(void) {
    int i;
    for (i = 0; i < DVD_LOW_CTX_MAX; i++) {
        dvdContexts[i].callback = 0;
        dvdContexts[i].WORD_0x4 = 0;
        dvdContexts[i].inUse = false;
        dvdContexts[i].magic = DVD_LOW_CTX_MAGIC;
        dvdContexts[i].id = i;
    }

    freeDvdContext = 0;
}

static s32 doTransactionCallback(s32 intType, void* arg) {
    DVDLowContext* ctx = (DVDLowContext*)arg;

    if (ctx->magic != DVD_LOW_CTX_MAGIC) {
        OSReport("(doTransactionCallback) Error - context mangled!\n");
        ctx->magic = DVD_LOW_CTX_MAGIC;
    } else {
        requestInProgress = false;

        if (ctx->callback != 0) {
            callbackInProgress = true;

            if (breakRequested == true) {
                breakRequested = false;
                intType |= DVD_INTTYPE_BR;
            }

            if (intType & DVD_INTTYPE_TC) {
                readLength = 0;
            }

            ctx->callback(intType);

            callbackInProgress = false;
        }
    }

    ctx->inUse = false;
    return IPC_RESULT_OK;
}

FORCE_ACTIVE(dvd_broadway, "(doCoverCallback) Error - context mangled!\n");

static s32 doPrepareCoverRegisterCallback(s32 intType, void* arg) {
    DVDLowContext* ctx = (DVDLowContext*)arg;

    requestInProgress = false;
    diRegValCache.dicvr = registerBuf.reg;

    if (ctx->magic != DVD_LOW_CTX_MAGIC) {
        OSReport("(doTransactionCallback) Error - context mangled!\n");
        ctx->magic = DVD_LOW_CTX_MAGIC;
    } else {
        if (ctx->callback != 0) {
            callbackInProgress = true;

            if (breakRequested == true) {
                breakRequested = false;
                intType |= DVD_INTTYPE_BR;
            }

            ctx->callback(intType);

            callbackInProgress = false;
        }
    }

    ctx->inUse = false;
    return IPC_RESULT_OK;
}

bool DVDLowInit(void) {
    IPCResult result;

    if (!DVDLowInitCalled) {
        DVDLowInitCalled = true;

        result = IPCCltInit();
        if (result != IPC_RESULT_OK) {
            OSReport("IPCCltInit returned error: %d\n", result);
            return false;
        }

        if (!allocateStructures()) {
            return false;
        }

        if (!dvdContextsInited) {
            initDvdContexts();
            dvdContextsInited = true;
        }
    }

    strncpy(pathBuf, "/dev/di", 32);
    DiFD = IOS_Open(pathBuf, IPC_OPEN_NONE);

    if (DiFD >= 0) {
        return true;
    }

    switch (DiFD) {
        case IPC_RESULT_NOEXISTS_INTERNAL:
            OSReport("(DVDLowInit) Error: IOS_Open failed - pathname '/dev/di' "
                     "does not exist\n");
            return false;
        case IPC_RESULT_ACCESS_INTERNAL:
            OSReport("(DVDLowInit) Error: IOS_Open failed - calling thread lacks "
                     "permission\n");
            return false;
        case IPC_RESULT_CONN_MAX_INTERNAL:
            OSReport("(DVDLowInit) Error: IOS_Open failed - connection limit has "
                     "been reached\n");
            return false;
        default:
            OSReport("(DVDLowInit) IOS_Open failed, errorcode = %d\n", DiFD);
            return false;
    }
}

bool DVDLowReadDiskID(DVDDiskID* out, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    if (out == 0) {
        OSReport("@@@@@@ WARNING - Calling DVDLowReadDiskId with NULL ptr\n");
    }

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_READ_DISK_ID;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_READ_DISK_ID, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), out,
                            sizeof(DVDDiskID), doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowReadDiskID) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

static void nextCommandBuf(void) {
    if (++freeCommandBuf >= DVD_LOW_CMD_MAX) {
        freeCommandBuf = 0;
    }
}

static inline DVDLowContext* newContext(DVDLowCallback callback, UNKWORD arg2) {
    s32 id;

    // The last operation somehow did not complete
    if (DVD_LOW_CTX_IN_USE(dvdContexts[freeDvdContext]) == true) {
        OSReport("(newContext) ERROR: freeDvdContext.inUse (#%d) is true\n", freeDvdContext);
        OSReport("(newContext) Now spinning in infinite loop\n");

        while (true) {
            ;
        }
    }

    // Memory corruption
    if (dvdContexts[freeDvdContext].magic != DVD_LOW_CTX_MAGIC) {
        OSReport("(newContext) Something overwrote the context magic - spinning \n");

        while (true) {
            ;
        }
    }

    dvdContexts[freeDvdContext].callback = callback;
    dvdContexts[freeDvdContext].WORD_0x4 = arg2;
    dvdContexts[freeDvdContext].inUse = true;

    id = freeDvdContext;

    if (++freeDvdContext >= DVD_LOW_CTX_MAX) {
        freeDvdContext = 0;
    }

    return &dvdContexts[id];
}

bool DVDLowOpenPartition(u32 offset, const ESPTicket* ticket, u32 certsSize, const void* certs, ESPTmd* tmd,
                         DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    if (ticket != 0 && (u32)ticket % 32 != 0) {
        OSReport("(DVDLowOpenPartition) eTicket memory is unaligned\n");
        return false;
    }

    if (certs != 0 && (u32)certs % 32 != 0) {
        OSReport("(DVDLowOpenPartition) certificates memory is unaligned\n");
        return false;
    }

    if (tmd != 0 && (u32)tmd % 32 != 0) {
        OSReport("(DVDLowOpenPartition) certificates memory is unaligned\n");
        return false;
    }

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTLV_OPEN_PARTITION;
    diCommand[freeCommandBuf].arg1 = offset;

    // Input vector 1: DI command
    ioVec[0].base = &diCommand[freeCommandBuf];
    ioVec[0].length = sizeof(DVDLowDICommand);

    // Input vector 2: eTicket
    ioVec[1].base = (void*)ticket;
    if (ticket == 0) {
        ioVec[1].length = 0;
    } else {
        ioVec[1].length = sizeof(ESPTicket);
    }

    // Input vector 3: Shared certs
    ioVec[2].base = (void*)certs;
    if (certs == 0) {
        ioVec[2].length = 0;
    } else {
        ioVec[2].length = certsSize;
    }

    // Output vector 1: TMD
    ioVec[3].base = tmd;
    ioVec[3].length = sizeof(ESPTmd);

    // Output vector 2: Ticket error
    ioVec[4].base = &lastTicketError;
    ioVec[4].length = sizeof(lastTicketError);

    result = IOS_IoctlvAsync(DiFD, DVD_IOCTLV_OPEN_PARTITION, 3, 2, ioVec, doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowOpenPartition) IOS_IoctlvAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool DVDLowClosePartition(DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_CLOSE_PARTITION;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_CLOSE_PARTITION, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), 0, 0,
                            doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowClosePartition) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool DVDLowUnencryptedRead(void* dst, u32 size, u32 offset, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    readLength = size;

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_UNENCRYPTED_READ;
    diCommand[freeCommandBuf].arg1 = size;
    diCommand[freeCommandBuf].arg2 = offset;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_UNENCRYPTED_READ, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), dst,
                            size, doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowUnencryptedRead) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool DVDLowStopMotor(bool eject, bool kill, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_STOP_MOTOR;
    diCommand[freeCommandBuf].arg1 = eject;
    diCommand[freeCommandBuf].arg2 = kill;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_STOP_MOTOR, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand),
                            &diRegValCache, sizeof(DVDLowDiRegs), doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowStopMotor) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

FORCE_ACTIVE(dvd_broadway, "@@@ (DVDLowWaitForCoverClose) IOS_IoctlAsync returned error: %d\n");

bool DVDLowInquiry(DVDDriveInfo* out, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_INQUIRY;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_INQUIRY, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), out,
                            sizeof(DVDDriveInfo), doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowInquiry) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool DVDLowRequestError(DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_REQUEST_ERROR;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_REQUEST_ERROR, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand),
                            &diRegValCache, sizeof(DVDLowDiRegs), doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowRequestError) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

FORCE_ACTIVE(dvd_broadway, "(DVDLowSetSpinupFlag): Synch functions can't be called in callbacks\n",
             "@@@ (DVDLowNotifyReset) IOS_IoctlAsync returned error: %d\n");

bool DVDLowSetSpinupFlag(bool enable) {
    spinUpValue = enable;
    return true;
}

bool DVDLowReset(DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_RESET;
    diCommand[freeCommandBuf].arg1 = spinUpValue;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_RESET, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), 0, 0,
                            doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowReset) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool DVDLowAudioBufferConfig(bool enable, u32 size, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_AUDIO_BUFFER_CONFIG;
    diCommand[freeCommandBuf].arg1 = enable;
    diCommand[freeCommandBuf].arg2 = size;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_AUDIO_BUFFER_CONFIG, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand),
                            &diRegValCache, sizeof(DVDLowDiRegs), doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowAudioBufferConfig) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

FORCE_ACTIVE(dvd_broadway, "(DVDLowGetCoverStatus): Synch functions can't be called in callbacks\n",
             "@@@ (DVDLowGetCoverStatus) IOS_Ioctl returned error: %d\n",
             "@@@ (DVDLowReadDVD) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowReadDVDConfig) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowReadDvdCopyright) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowReadDvdPhysical) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowReadDvdDiscKey) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowReportKey) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowOffset) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowStopLaser) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowReadDiskBca) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowSerMeasControl) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowRequestDiscStatus) IOS_IoctlAsync returned error: %d\n",
             "@@@ (DVDLowRequestRetryNumber) IOS_IoctlAsync returned error: %d\n");

bool DVDLowSetMaximumRotation(u32 speed, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_SET_MAX_ROTATION;
    diCommand[freeCommandBuf].arg1 = speed >> 16 & 3;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_SET_MAX_ROTATION, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), 0, 0,
                            doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowSetMaxRotation) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool DVDLowRead(void* dst, u32 size, u32 offset, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    if ((u32)dst % 32 != 0) {
        OSReport("(DVDLowRead): ERROR - destAddr buffer is not 32 byte aligned\n");
        return false;
    }

    requestInProgress = true;
    ctx = newContext(callback, 1);

    readLength = size;

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_READ;
    diCommand[freeCommandBuf].arg1 = size;
    diCommand[freeCommandBuf].arg2 = offset;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_READ, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), dst, size,
                            doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowRead) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool DVDLowSeek(u32 offset, DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_SEEK;
    diCommand[freeCommandBuf].arg1 = offset;

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_SEEK, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand), 0, 0,
                            doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowSeek) IOS_IoctlAsync returned error: %d\n", result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

FORCE_ACTIVE(dvd_broadway, "(DVDLowGetCoverReg): Synch functions can't be called in callbacks\n",
             "@@@ (DVDLowGetCoverReg) IOS_Ioctl returned error: %d\n");

u32 DVDLowGetCoverRegister(void) { return diRegValCache.dicvr; }

bool DVDLowPrepareCoverRegister(DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_PREPARE_COVER_REGISTER;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_PREPARE_COVER_REGISTER, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand),
                            &registerBuf, sizeof(DVDLowRegBuffer), doPrepareCoverRegisterCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowPrepareCoverRegsiter) IOS_IoctlAsync returned "
                 "error: %d\n",
                 result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

u32 DVDLowGetImmBufferReg(void) { return diRegValCache.diimmbuf; }

bool DVDLowUnmaskStatusInterrupts(void) { return true; }

bool DVDLowMaskCoverInterrupt(void) { return true; }

bool DVDLowClearCoverInterrupt(DVDLowCallback callback) {
    DVDLowContext* ctx;
    IPCResult result;

    nextCommandBuf();
    diCommand[freeCommandBuf].command = DVD_IOCTL_CLEAR_COVER_INTERRUPT;

    requestInProgress = true;
    ctx = newContext(callback, 1);

    result = IOS_IoctlAsync(DiFD, DVD_IOCTL_CLEAR_COVER_INTERRUPT, &diCommand[freeCommandBuf], sizeof(DVDLowDICommand),
                            0, 0, doTransactionCallback, ctx);

    if (result != IPC_RESULT_OK) {
        OSReport("@@@ (DVDLowClearCoverInterrupt) IOS_IoctlAsync returned "
                 "error: %d\n",
                 result);
        ctx->inUse = false;

        return false;
    }

    return true;
}

bool __DVDLowTestAlarm(const OSAlarm* alarm) {
#pragma unused(alarm)

    return false;
}

FORCE_ACTIVE(dvd_broadway, "@@@ (DVDLowEnableDvdVideo) IOS_IoctlAsync returned error: %d\n");
