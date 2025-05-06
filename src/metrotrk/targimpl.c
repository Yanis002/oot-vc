#include "metrotrk/targimpl.h"
#include "macros.h"
#include "metrotrk/dolphin_trk_glue.h"
#include "metrotrk/flush_cache.h"
#include "metrotrk/memmap.h"
#include "metrotrk/mpc_7xx_603e.h"
#include "metrotrk/msghndlr.h"
#include "metrotrk/ppc_except.h"
#include "metrotrk/ppc_targimpl.h"
#include "metrotrk/support.h"
#include "revolution/os.h"

#define BOOTINFO 0x80000000
#define MEM2_CACHED 0x90000000
#define DB_EXCEPTION_MASK 0x44

typedef struct TRKExceptionStatus {
    StopInfo_PPC exceptionInfo;
    u8 inTRK;
    u8 exceptionDetected;
    u8 pad[2];
} TRKExceptionStatus;

typedef struct TRKStepStatus {
    bool active; // 0x0
    DSMessageStepOptions type; // 0x4
    u32 count; // 0x8
    u32 rangeStart; // 0xC
    u32 rangeEnd; // 0x10
} TRKStepStatus;

ProcessorState_PPC gTRKCPUState;

ProcessorRestoreFlags_PPC gTRKRestoreFlags = {false, false};

TRKState_PPC gTRKState;

static TRKExceptionStatus gTRKExceptionStatus = {{0, 0, 0}, true, 0};

static TRKStepStatus gTRKStepStatus = {0, kDSStepIntoCount, 0, 0, 0};

u128 TRKvalue128_temp;
static u16 TRK_saved_exceptionID = 0;
Default_PPC gTRKSaveState;

typedef void (*RegAccessFunc)(void* srcDestPtr, u128 val);

static void TRKExceptionHandler(u16);
void TRKInterruptHandlerEnableInterrupts();
static void GetThreadInfo(int*, int*);

// Instruction macros
#define INSTR_NOP 0x60000000
#define INSTR_BLR 0x4E800020
#define INSTR_PSQ_ST(psr, offset, rDest, w, gqr) \
    (0xF0000000 | (psr << 21) | (rDest << 16) | (w << 15) | (gqr << 12) | offset)
#define INSTR_PSQ_L(psr, offset, rSrc, w, gqr) \
    (0xE0000000 | (psr << 21) | (rSrc << 16) | (w << 15) | (gqr << 12) | offset)
#define INSTR_STW(rSrc, offset, rDest) (0x90000000 | (rSrc << 21) | (rDest << 16) | offset)
#define INSTR_LWZ(rDest, offset, rSrc) (0x80000000 | (rDest << 21) | (rSrc << 16) | offset)
#define INSTR_STFD(fprSrc, offset, rDest) (0xD8000000 | (fprSrc << 21) | (rDest << 16) | offset)
#define INSTR_LFD(fprDest, offset, rSrc) (0xC8000000 | (fprDest << 21) | (rSrc << 16) | offset)
#define INSTR_MFSPR(rDest, spr) (0x7C000000 | (rDest << 21) | ((spr & 0xFE0) << 6) | ((spr & 0x1F) << 16) | 0x2A6)
#define INSTR_MTSPR(spr, rSrc) (0x7C000000 | (rSrc << 21) | ((spr & 0xFE0) << 6) | ((spr & 0x1F) << 16) | 0x3A6)

ASM u32 __TRK_get_MSR(void){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mfmsr r3
    blr
#endif // clang-format on
}

ASM void __TRK_set_MSR(register u32 val){
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mtmsr val
    blr
#endif // clang-format on
}

DSError TRKValidMemory32(const void* addr, size_t length, ValidMemoryOptions readWriteable) {
    DSError err = kInvalidMemory;
    const u8* start;
    const u8* end;
    int i;

    start = (const u8*)addr;
    end = ((const u8*)addr + (length - 1));

    if (end < start) {
        return kInvalidMemory;
    }

    for (i = 0; i < (int)(sizeof(gTRKMemMap) / sizeof(memRange)); i++) {
        if (start <= (const u8*)gTRKMemMap[i].end && end >= (const u8*)gTRKMemMap[i].start) {
            if ((readWriteable == kValidMemoryReadable && !gTRKMemMap[i].readable) ||
                (readWriteable == kValidMemoryWriteable && !gTRKMemMap[i].writeable)) {
                err = kInvalidMemory;
            } else {
                err = kNoError;

                if (start < (const u8*)gTRKMemMap[i].start) {
                    err = TRKValidMemory32(start, (u32)((const u8*)gTRKMemMap[i].start - start), readWriteable);
                }

                if (err == kNoError && end > (const u8*)gTRKMemMap[i].end) {
                    err = TRKValidMemory32((const u8*)gTRKMemMap[i].end, (u32)(end - (const u8*)gTRKMemMap[i].end),
                                           readWriteable);
                }
            }

            break;
        }
    }

    return err;
}

// This is a certified metrowerks moment
#include "metrotrk/ppc_mem.h"

ASM static void TRK_ppc_memcpy(void* dest, const void* src, int n, u32 param_4, u32 param_5){
#ifdef __MWERKS__ // clang-format off
	nofralloc

	mfmsr	r8
	li		r10, 0

loop:
	cmpw	r10, r5
	beq		end
	mtmsr	r7
	sync
	lbzx	r9, r10, r4
	mtmsr	r6
	sync
	stbx	r9, r10, r3
	addi 	r10, r10, 1
	b		loop

end:
	mtmsr	r8
	sync
	blr
#endif // clang-format on
}

DSError TRKTargetAccessMemory(void* data, void* start, size_t* length, MemoryAccessOptions accessOptions, bool read) {
    DSError error;
    u32 uVar5;
    void* addr;
    u32 param4;
    TRKExceptionStatus tempExceptionStatus;

    tempExceptionStatus = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = false;

    addr = TRKTargetTranslate((u32*)start);
    error = TRKValidMemory32(addr, *length, read ? kValidMemoryReadable : kValidMemoryWriteable);

    if (error != kNoError) {
        *length = 0;
    } else {
        uVar5 = __TRK_get_MSR();
        param4 = uVar5 | (gTRKCPUState.Extended1.MSR & MSR_DR);

        if (read) {
            TRK_ppc_memcpy(data, addr, *length, uVar5, param4);
        } else {
            TRK_ppc_memcpy(addr, data, *length, param4, uVar5);
            TRK_flush_cache((u32)addr, *length);
            if ((void*)start != addr) {
                TRK_flush_cache((u32)start, *length);
            }
        }
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        *length = 0;
        error = kCWDSException;
    }

    gTRKExceptionStatus = tempExceptionStatus;
    return error;
}

DSError TRKTargetReadInstruction(void* data, void* start) {
    DSError error = kNoError;
    size_t registersLength = sizeof(InstructionType);

    error = TRKTargetAccessMemory(data, start, &registersLength, kUserMemory, true);

    if (error == kNoError && registersLength != sizeof(InstructionType)) {
        error = kInvalidMemory;
    }

    return error;
}

DSError TRKTargetAccessDefault(u32 firstRegister, u32 lastRegister, MessageBuffer* b, size_t* registersLengthPtr,
                               bool read) {
    DSError error;
    u32 count;
    u32* data;
    TRKExceptionStatus tempExceptionStatus;

    if (lastRegister > TRK_DEFAULT_XER) {
        return kInvalidRegister;
    }

    tempExceptionStatus = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = false;

    data = gTRKCPUState.Default.GPR + firstRegister;
    count = (lastRegister - firstRegister) + 1;
    *registersLengthPtr = count * sizeof(DefaultType);

    if (read) {
        error = TRKAppendBuffer_ui32(b, data, count);
    } else {
        error = TRKReadBuffer_ui32(b, data, count);
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        *registersLengthPtr = 0;
        error = kCWDSException;
    }

    gTRKExceptionStatus = tempExceptionStatus;
    return error;
}

DSError TRKTargetAccessFP(u32 firstRegister, u32 lastRegister, MessageBuffer* b, size_t* registerStorageSize,
                          s32 read) {
    DSError err;
    u32 current;
    FloatType value;
    TRKExceptionStatus savedException;

    if (lastRegister > 33) {
        return kInvalidRegister;
    }

    savedException = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = 0;

    __TRK_set_MSR(__TRK_get_MSR() | 0x2000);

    for (current = firstRegister, *registerStorageSize = 0, err = kNoError;
         (current <= lastRegister) && (err == kNoError); *registerStorageSize += sizeof(FloatType), current++) {

        if (read) {
            err = TRKPPCAccessFPRegister((void*)&value, current, read);

            err = TRKAppendBuffer1_ui64(b, value);
        } else {
            err = TRKReadBuffer1_ui64(b, &value);

            err = TRKPPCAccessFPRegister((void*)&value, current, read);
        }
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        err = kCWDSException;
        *registerStorageSize = 0;
    }

    gTRKExceptionStatus = savedException;

    return err;
}

DSError TRKTargetAccessExtended1(u32 firstRegister, u32 lastRegister, MessageBuffer* b, size_t* registerStorageSize,
                                 s32 read) {
    DSError err;
    DefaultType* start;
    s32 count;
    TRKExceptionStatus savedException;

    if (lastRegister > 0x60) {
        return kInvalidRegister;
    }

    savedException = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = 0;

    *registerStorageSize = 0;

    if (firstRegister <= lastRegister) {
        start = (((u32*)&gTRKCPUState.Extended1) + firstRegister);
        count = (s32)(lastRegister - firstRegister + 1);

        *registerStorageSize += (count * 1);

        if (read) {
            err = TRKAppendBuffer_ui32(b, start, (s32)count);
        } else {
            if ((start <= (u32*)&gTRKCPUState.Extended1.TBU) && // TBU
                ((start + count - 1) >= (u32*)&gTRKCPUState.Extended1.TBL)) { // TBL
                gTRKRestoreFlags.TBR = 1;
            }

            if ((start <= (u32*)&gTRKCPUState.Extended1.DEC) &&
                ((start + count - 1) >= (u32*)&gTRKCPUState.Extended1.DEC)) {
                gTRKRestoreFlags.DEC = 1;
            }

            err = TRKReadBuffer_ui32(b, start, (s32)count);
        }
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        err = kCWDSException;
        *registerStorageSize = 0;
    }

    gTRKExceptionStatus = savedException;

    return err;
}

/*
** Useful defines.
*/
#define ALTIVEC_VECTOR_REG_ACCESS_MIN 0
#define ALTIVEC_VECTOR_REG_ACCESS_MAX 31
#define ALTIVEC_VSCR_ACCESS 32
#define ALTIVEC_VRSAVE_SPR_ACCESS 33

DSError TRKTargetAccessExtended2(u32 firstRegister, u32 lastRegister, MessageBuffer* b, size_t* registersLengthPtr,
                                 bool read) {
    TRKExceptionStatus tempExceptionStatus;
    FloatType temp;
    u32 temp2;
    DSError error;

    if (lastRegister > ALTIVEC_VECTOR_REG_ACCESS_MAX) {
        return kInvalidRegister;
    }

    tempExceptionStatus = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = false;

    TRKPPCAccessSPR(&temp2, SPR_HID2, true);
    temp2 |= 0xA0000000;
    TRKPPCAccessSPR(&temp2, SPR_HID2, false);
    temp2 = 0;
    TRKPPCAccessSPR(&temp2, SPR_GQR0, false);
    *registersLengthPtr = 0;
    error = kNoError;

    while (firstRegister <= lastRegister && error == kNoError) {
        if (read) {
            error = TRKPPCAccessPairedSingleRegister(&temp, firstRegister, read);
            error = TRKAppendBuffer1_ui64(b, temp);
        } else {
            error = TRKReadBuffer1_ui64(b, &temp);
            error = TRKPPCAccessPairedSingleRegister(&temp, firstRegister, read);
        }

        firstRegister++;
        *registersLengthPtr += sizeof(FloatType);
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        *registersLengthPtr = 0;
        error = kCWDSException;
    }

    gTRKExceptionStatus = tempExceptionStatus;

    return error;
}

void TRKUARTInterruptHandler(void);

ASM void TRKInterruptHandler() {
#ifdef __MWERKS__ // clang-format off
	nofralloc
	mtsrr0 r2
	mtsrr1 r4
	mfsprg r4, 3
	mfcr r2
	mtsprg 3, r2
	lis r2, gTRKState@h
	ori r2, r2, gTRKState@l
	lwz r2, TRKState_PPC.MSR(r2)
	ori r2, r2, 0x8002
	xori r2, r2, 0x8002
	sync
	mtmsr r2
	sync
	lis r2, TRK_saved_exceptionID@h
	ori r2, r2, TRK_saved_exceptionID@l
	sth r3, 0(r2)
	cmpwi r3, 0x500
	bne L_802CF694
	lis r2, gTRKCPUState@h
	ori r2, r2, gTRKCPUState@l
	mflr r3
	stw r3, ProcessorState_PPC.transport_handler_saved_ra(r2)
	bl TRKUARTInterruptHandler
	lis r2, gTRKCPUState@h
	ori r2, r2, gTRKCPUState@l
	lwz r3, ProcessorState_PPC.transport_handler_saved_ra(r2)
	mtlr r3
	lis r2, gTRKState@h
	ori r2, r2, gTRKState@l
	lwz r2, TRKState_PPC.inputPendingPtr(r2)
	lbz r2, TRKState_PPC.GPR[0](r2)
	cmpwi r2, 0
	beq L_802CF678
	lis r2, gTRKExceptionStatus@h
	ori r2, r2, gTRKExceptionStatus@l
	lbz r2, TRKExceptionStatus.inTRK(r2)
	cmpwi r2, 1
	beq L_802CF678
	lis r2, gTRKState@h
	ori r2, r2, gTRKState@l
	li r3, 1
	stb r3, TRKState_PPC.inputActivated(r2)
	b L_802CF694
L_802CF678:
	lis r2, gTRKSaveState@h
	ori r2, r2, gTRKSaveState@l
	lwz r3, Default_PPC.CR(r2)
	mtcrf 0xff, r3
	lwz r3, Default_PPC.GPR[3](r2)
	lwz r2, Default_PPC.GPR[2](r2)
	rfi 
L_802CF694:
	lis r2, TRK_saved_exceptionID@h
	ori r2, r2, TRK_saved_exceptionID@l
	lhz r3, 0(r2)
	lis r2, gTRKExceptionStatus@h
	ori r2, r2, gTRKExceptionStatus@l
	lbz r2, TRKExceptionStatus.inTRK(r2)
	cmpwi r2, 0
	bne TRKExceptionHandler
	lis r2, gTRKCPUState@h
	ori r2, r2, gTRKCPUState@l
	stw r0, ProcessorState_PPC.Default.GPR[0](r2)
	stw r1, ProcessorState_PPC.Default.GPR[1](r2)
	mfsprg r0, 1
	stw r0, ProcessorState_PPC.Default.GPR[2](r2)
	sth r3, ProcessorState_PPC.Extended1.exceptionID(r2)
	sth r3, (ProcessorState_PPC.Extended1.exceptionID + 2)(r2)
	mfsprg r0, 2
	stw r0, ProcessorState_PPC.Default.GPR[3](r2)
	stmw r4, ProcessorState_PPC.Default.GPR[4](r2)
	mfsrr0 r27
	mflr r28
	mfsprg r29, 3
	mfctr r30
	mfxer r31
	stmw r27, ProcessorState_PPC.Default.PC(r2)
	bl TRKSaveExtended1Block
	lis r2, gTRKExceptionStatus@h
	ori r2, r2, gTRKExceptionStatus@l
	li r3, 1
	stb r3, TRKExceptionStatus.inTRK(r2)
	lis r2, gTRKState@h
	ori r2, r2, gTRKState@l
	lwz r0, TRKState_PPC.MSR(r2)
	sync
	mtmsr r0
	sync
	lwz r0, TRKState_PPC.LR(r2)
	mtlr r0
	lwz r0, TRKState_PPC.CTR(r2)
	mtctr r0
	lwz r0, TRKState_PPC.XER(r2)
	mtxer r0
	lwz r0, TRKState_PPC.DSISR(r2)
	mtdsisr r0
	lwz r0, TRKState_PPC.DAR(r2)
	mtdar r0
	lmw r3, TRKState_PPC.GPR[3](r2)
	lwz r0, TRKState_PPC.GPR[0](r2)
	lwz r1, TRKState_PPC.GPR[1](r2)
	lwz r2, TRKState_PPC.GPR[2](r2)
	b TRKPostInterruptEvent
#endif // clang-format on
}

static ASM void TRKExceptionHandler(u16 r3) {
#ifdef __MWERKS__ // clang-format off
	nofralloc

	addis     	r2, r0, gTRKExceptionStatus@h
	ori       	r2, r2, gTRKExceptionStatus@l
	sth       	r3, TRKExceptionStatus.exceptionInfo.exceptionID(r2)
	mfspr		r3, 26
	stw       	r3, TRKExceptionStatus.exceptionInfo.PC(r2)
	lhz       	r3, TRKExceptionStatus.exceptionInfo.exceptionID(r2)
	cmpwi 		r3, 0x200
	bt			2, skip_instr
	cmpwi 		r3, 0x300
	bt			2, skip_instr
	cmpwi 		r3, 0x400
	bt			2, skip_instr
	cmpwi 		r3, 0x600
	bt			2, skip_instr
	cmpwi 		r3, 0x700
	bt			2, skip_instr
	cmpwi 		r3, 0x800
	bt			2, skip_instr
	cmpwi 		r3, 0x1000
	bt			2, skip_instr
	cmpwi 		r3, 0x1100
	bt			2, skip_instr
	cmpwi 		r3, 0x1200
	bt			2, skip_instr
	cmpwi 		r3, 0x1300
	bt			2, skip_instr
	b			set

skip_instr:
	mfspr		r3, 26
	addi 		r3, r3, 4
	mtspr		26, r3

set:
	addis     	r2, r0, gTRKExceptionStatus@h
	ori       	r2, r2, gTRKExceptionStatus@l
	addi      	r3, r0, 1
	stb       	r3, TRKExceptionStatus.exceptionDetected(r2)

	mfspr		r3, 275
	mtcrf		0xFF, r3

	mfspr		r2, 273
	mfspr		r3, 274
	rfi
#endif // clang-format on
}

#define SUPPORT_TRAP 0x0FE00000

void TRKPostInterruptEvent(void) {
    NubEventType eventType;
    u32 inst;
    NubEvent event;

    if (gTRKState.inputActivated) {
        gTRKState.inputActivated = false;
        return;
    }

    switch ((u16)(gTRKCPUState.Extended1.exceptionID & 0xFFFF)) {
        case PPC_PROGRAMERROR:
        case PPC_TRACE:
            TRKTargetReadInstruction((void*)&inst, (void*)gTRKCPUState.Default.PC);

            if (inst == SUPPORT_TRAP) {
                eventType = kSupportEvent;
            } else {
                eventType = kBreakpointEvent;
            }
            break;
        default:
            eventType = kExceptionEvent;
            break;
    }

    TRKConstructEvent(&event, eventType);
    TRKPostEvent(&event);
}

ASM void TRKSwapAndGo() {
#ifdef __MWERKS__ // clang-format off
	nofralloc

	addis     r3, r0, gTRKState@h
    ori       r3, r3, gTRKState@l

    stmw      r0, TRKState_PPC.GPR[0](r3)

    mfmsr     r0
    stw       r0, TRKState_PPC.MSR(r3)

    mflr      r0
    stw       r0, TRKState_PPC.LR(r3)

    mfctr     r0
    stw       r0, TRKState_PPC.CTR(r3)

    mfxer     r0
    stw       r0, TRKState_PPC.XER(r3)

    mfspr     r0, 18
    stw       r0, TRKState_PPC.DSISR(r3)

    mfspr     r0, 19
    stw       r0, TRKState_PPC.DAR(r3)
	
    addi      r1, r0, 0x8002
    nor       r1, r1, r1
    mfmsr     r3
    and       r3, r3, r1
    mtmsr     r3

    addis     r2, r0, gTRKState@h
    ori       r2, r2, gTRKState@l
    lwz       r2, TRKState_PPC.inputPendingPtr(r2)
    lbz       r2, 0(r2)
    cmpwi     r2, 0
    beq       NoOutgoingInput

    addis     r2, r0, gTRKState@h
    ori       r2, r2, gTRKState@l
    li        r3, 1
    stb       r3, TRKState_PPC.inputActivated(r2)

    b         TRKInterruptHandlerEnableInterrupts

NoOutgoingInput:
    addis     r2, r0, gTRKExceptionStatus@h
    ori       r2, r2, gTRKExceptionStatus@l
    addi      r3, r0, 0
    stb       r3, TRKExceptionStatus.inTRK(r2)
    bl        TRKRestoreExtended1Block

    addis     r2, r0, gTRKCPUState@h
    ori       r2, r2, gTRKCPUState@l

    lmw       r27, ProcessorState_PPC.Default.PC(r2)
    mtspr     26, r27
    mtspr     8, r28
    mtcr      r29
    mtspr     9, r30
    mtspr     1, r31

    // Restore all GPR's

    lmw       r3, ProcessorState_PPC.Default.GPR[3](r2)     // restore GPR3-31
    lwz       r0, ProcessorState_PPC.Default.GPR[0](r2)
    lwz       r1, ProcessorState_PPC.Default.GPR[1](r2)
    lwz       r2, ProcessorState_PPC.Default.GPR[2](r2)

    rfi
#endif // clang-format on
}

ASM void TRKInterruptHandlerEnableInterrupts() {
#ifdef __MWERKS__ // clang-format off
	nofralloc

    addis     r2, r0, gTRKState@h
    ori       r2, r2, gTRKState@l
    lwz       r0, TRKState_PPC.MSR(r2)
    sync
    mtmsr     r0
    sync

    lwz       r0, TRKState_PPC.LR(r2)
    mtlr      r0

    lwz       r0, TRKState_PPC.CTR(r2)
    mtctr     r0

    lwz       r0, TRKState_PPC.XER(r2)
    mtxer     r0

    lwz       r0, TRKState_PPC.DSISR(r2)
    mtspr     18, r0

    lwz       r0, TRKState_PPC.DAR(r2)
    mtspr     19, r0

    lmw       r3, TRKState_PPC.GPR[3](r2)
    lwz       r0, TRKState_PPC.GPR[0](r2)
    lwz       r1, TRKState_PPC.GPR[1](r2)
    lwz       r2, TRKState_PPC.GPR[2](r2)

    b         TRKPostInterruptEvent
#endif // clang-format on
}

DSError TRKTargetInterrupt(NubEvent* event) {
    DSError error = kNoError;

    switch (event->fType) {
        case kBreakpointEvent:
        case kExceptionEvent:
            if (TRKTargetCheckStep() == false) {
                TRKTargetSetStopped(true);
                error = TRKDoNotifyStopped(kDSNotifyStopped);
            }
            break;
        default:
            break;
    }

    return error;
}

#define CURRENT_CONTEXT_ADDR 0xD4
#define FPU_CONTEXT_ADDR 0xD8
#define ROOT_THREAD_ADDR 0xDC
#define PREVIOUS_THREAD_ADDR 0xE0
#define CURRENT_THREAD_ADDR 0xE4

DSError TRKTargetAddStopInfo(MessageBuffer* b) {
    u32 instruction;
    DSError err;
    u32 buf[16];
    u32 v12;
    u32 v13;

    err = kNoError;

    TRK_memset(&buf[0], 0, TRK_MSG_HEADER_LENGTH);
    buf[0] = TRK_MSG_HEADER_LENGTH;
    DSFetch_u8(&buf[1]) = kDSNotifyStopped;
    buf[2] = gTRKCPUState.Default.PC;
    TRKTargetReadInstruction((void*)&instruction, (void*)gTRKCPUState.Default.PC);
    buf[3] = instruction;
    buf[4] = (gTRKCPUState.Extended1.exceptionID & 0xffff);
    return TRKAppendBuffer_ui8(b, (const u8*)buf, TRK_MSG_HEADER_LENGTH);
}

void TRKTargetAddExceptionInfo(MessageBuffer* b) {
    u32 local_54;
    msgbuf_t reply;

    TRK_memset(&reply, 0, sizeof(msgbuf_t));
    reply.msgLength = sizeof(msgbuf_t);
    reply.commandId = kDSNotifyException;
    reply.replyErrorInt = gTRKExceptionStatus.exceptionInfo.PC;
    TRKTargetReadInstruction((void*)&local_54, (void*)gTRKExceptionStatus.exceptionInfo.PC);
    *(u32*)&reply.unk0C = local_54;
    *(u32*)reply.unk10 = gTRKExceptionStatus.exceptionInfo.exceptionID;
    TRKAppendBuffer_ui8(b, (u8*)&reply, sizeof(msgbuf_t));
}

static DSError TRKTargetEnableTrace(bool enable) {
    if (enable) {
        gTRKCPUState.Extended1.MSR |= 0x400;
    } else {
        gTRKCPUState.Extended1.MSR = ((gTRKCPUState.Extended1.MSR & (0xFFFFFFFF ^ 0x400)));
    }

    return kNoError;
}

bool TRKTargetStepDone() {
    bool result = true;

    if (gTRKStepStatus.active && ((u16)gTRKCPUState.Extended1.exceptionID) == PPC_TRACE) {
        switch (gTRKStepStatus.type) {
            case kDSStepIntoCount:
                if (gTRKStepStatus.count > 0) {
                    result = false;
                }
                break;
            case kDSStepIntoRange:
                if (gTRKCPUState.Default.PC >= gTRKStepStatus.rangeStart &&
                    gTRKCPUState.Default.PC <= gTRKStepStatus.rangeEnd) {
                    result = false;
                }
                break;
            default:
                break;
        }
    }

    return result;
}

DSError TRKTargetDoStep() {
    DSError err = kNoError;
    gTRKStepStatus.active = 1;
    TRKTargetEnableTrace(true);

    if ((gTRKStepStatus.type == kDSStepIntoCount) || (gTRKStepStatus.type == kDSStepOverCount)) {
        gTRKStepStatus.count--;
    }

    TRKTargetSetStopped(false);

    return err;
}

static bool TRKTargetCheckStep() {
    if (gTRKStepStatus.active) {
        TRKTargetEnableTrace(false);

        if (TRKTargetStepDone()) {
            gTRKStepStatus.active = false;
        } else {
            TRKTargetDoStep();
        }
    }

    return gTRKStepStatus.active;
}

DSError TRKTargetSingleStep(u32 count, bool stepOver) {
    DSError err = kNoError;

    if (stepOver) {
        err = kUnsupportedError;
    } else {
        gTRKStepStatus.type = kDSStepIntoCount;
        gTRKStepStatus.count = count;

        err = TRKTargetDoStep();
    }

    return err;
}

DSError TRKTargetStepOutOfRange(u32 rangeStart, u32 rangeEnd, bool stepOver) {
    DSError error = kNoError;

    if (stepOver) {
        // Stepping over isn't supported for PowerPC
        error = kUnsupportedError;
    } else {
        gTRKStepStatus.type = kDSStepIntoRange;
        gTRKStepStatus.rangeStart = rangeStart;
        gTRKStepStatus.rangeEnd = rangeEnd;

        error = TRKTargetDoStep();
    }

    return error;
}

u32 TRKTargetGetPC() { return gTRKCPUState.Default.PC; }

DSError TRKTargetSupportRequest(void) {
    size_t* length;
    DSIOResult io_result;
    s32 msg_length;
    MessageCommandID command;
    NubEvent event;
    DSError err = kNoError;

    command = (MessageCommandID)gTRKCPUState.Default.GPR[3];

    if ((command != kDSReadFile) && (command != kDSWriteFile) && (command != kDSOpenFile) &&
        (command != kDSCloseFile) && (command != kDSPositionFile)) {
        TRKConstructEvent(&event, kExceptionEvent);
        TRKPostEvent(&event);
        return err;
    }

    if (command == kDSOpenFile) {
        err = HandleOpenFileSupportRequest((char*)gTRKCPUState.Default.GPR[4], (u8)gTRKCPUState.Default.GPR[5],
                                           (u32*)gTRKCPUState.Default.GPR[6], &io_result);

        if (io_result == kDSIONoError && err != kNoError) {
            io_result = kDSIOError;
        }

        gTRKCPUState.Default.GPR[3] = (DefaultType)io_result;
    } else if (command == kDSCloseFile) {
        err = HandleCloseFileSupportRequest((DSFileHandle)gTRKCPUState.Default.GPR[4], &io_result);

        if (io_result == kDSIONoError && err != kNoError) {
            io_result = kDSIOError;
        }

        gTRKCPUState.Default.GPR[3] = (DefaultType)io_result;
    } else if (command == kDSPositionFile) {
        msg_length = DSFetch_u32(gTRKCPUState.Default.GPR[5]);
        err = HandlePositionFileSupportRequest((DSFileHandle)gTRKCPUState.Default.GPR[4], (u32*)&msg_length,
                                               (u8)gTRKCPUState.Default.GPR[6], &io_result);

        if (io_result == kDSIONoError && err != kNoError) {
            io_result = kDSIOError;
        }

        gTRKCPUState.Default.GPR[3] = (DefaultType)io_result;
        DSFetch_s32(gTRKCPUState.Default.GPR[5]) = msg_length;
    } else {
        length = (size_t*)gTRKCPUState.Default.GPR[5];

        err = TRK_SuppAccessFile((DSFileHandle)gTRKCPUState.Default.GPR[4], (u8*)gTRKCPUState.Default.GPR[6], length,
                                 &io_result, 1, (command == kDSReadFile));

        if ((io_result == kDSIONoError) && (err != kNoError)) {
            io_result = kDSIOError;
        }

        gTRKCPUState.Default.GPR[3] = (DefaultType)io_result;

        if (command == kDSReadFile) {
            TRK_flush_cache((u32)gTRKCPUState.Default.GPR[6], *length);
        }
    }

    gTRKCPUState.Default.PC += 4;
    return err;
}

bool TRKTargetStopped() { return gTRKState.stopped; }

void TRKTargetSetStopped(bool val) { gTRKState.stopped = val; }

DSError TRKTargetStop() {
    TRKTargetSetStopped(true);
    return kNoError;
}

DSError TRKPPCAccessSPR(void* srcDestPtr, u32 spr, bool read) {
    // all nop by default
    InstructionType instructionData[] = {INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                                         INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP};

    if (read) {
        instructionData[0] = INSTR_MFSPR(4, spr); // mfspr r4, spr
        instructionData[1] = INSTR_STW(4, 0, 3); // stw r4, 0(r3)
    } else {
        instructionData[0] = INSTR_LWZ(4, 0, 3); // lwz r4, 0(r3)
        instructionData[1] = INSTR_MTSPR(spr, 4); // mtspr spr, r4
    }

    return TRKPPCAccessSpecialReg(srcDestPtr, instructionData, read);
}

DSError TRKPPCAccessPairedSingleRegister(void* srcDestPtr, u32 psr, bool read) {
    // all nop by default
    InstructionType instructionData[] = {INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                                         INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP};

    if (read) {
        instructionData[0] = INSTR_PSQ_ST(psr, 0, 3, 0, 0); // psq_st psr, 0(r3), 0, 0
    } else {
        instructionData[0] = INSTR_PSQ_L(psr, 0, 3, 0, 0); // psq_l psr, 0(r3), 0, 0
    }

    return TRKPPCAccessSpecialReg(srcDestPtr, instructionData, read);
}

#define FP_FPSCR_ACCESS 32
#define FP_FPECR_ACCESS 33

ASM s32 ReadFPSCR(){
#ifdef __MWERKS__ // clang-format off
    nofralloc

    stwu      r1, -0x40(r1)
    stfd      f31, 0x10(r1)
    psq_st    f31, 0x20(r1), 0, 0
    mffs      f31
    stfd      f31, 0(r3)
    psq_l     f31, 0x20(r1), 0, 0
    lfd       f31, 0x10(r1)
    addi      r1, r1, 0x40
    blr
#endif // clang-format on
}

ASM void WriteFPSCR(){
#ifdef __MWERKS__ // clang-format off
    nofralloc

    stwu      r1, -0x40(r1)
    stfd      f31, 0x10(r1)
    psq_st    f31, 0x20(r1), 0, 0
    lfd       f31, 0(r3)
    mtfsf     0xff, f31
    psq_l     f31, 0x20(r1), 0, 0
    lfd       f31, 0x10(r1)
    addi      r1, r1, 0x40
    blr
#endif // clang-format on
}

DSError TRKPPCAccessFPRegister(void* srcDestPtr, u32 fpr, bool read) {
    DSError error = kNoError;
    // all nop by default
    InstructionType instructionData1[] = {INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                                          INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP};

    if (fpr < FP_FPSCR_ACCESS) {
        if (read) {
            instructionData1[0] = INSTR_STFD(fpr, 0, 3); // stfd fpr, 0(r3)
        } else {
            instructionData1[0] = INSTR_LFD(fpr, 0, 3); // lfd fpr, 0(r3)
        }

        error = TRKPPCAccessSpecialReg(srcDestPtr, instructionData1, read);
    } else if (fpr == FP_FPSCR_ACCESS) {
        if (read) {
            ReadFPSCR();
        } else {
            WriteFPSCR();
        }

        *(u64*)srcDestPtr &= 0xFFFFFFFF;
    } else if (fpr == FP_FPECR_ACCESS) {
        if (!read) {
            *(u32*)srcDestPtr = *(u32*)((u32)srcDestPtr + 4);
        }

        error = TRKPPCAccessSPR(srcDestPtr, SPR_FPECR, read);

        if (read) {
            *(u64*)srcDestPtr = (*(u64*)srcDestPtr >> 32) & 0xFFFFFFFF;
        }
    }

    return error;
}

DSError TRKPPCAccessSpecialReg(void* srcDestPtr, u32* instructionData, bool read) {
    RegAccessFunc accessFunc;
    instructionData[9] = INSTR_BLR; // Set the last entry as blr
    TRK_flush_cache((u32)instructionData, 0x28);
    // Call the instruction data array as code
    accessFunc = (RegAccessFunc)instructionData;
    accessFunc(srcDestPtr, TRKvalue128_temp);
    return kNoError;
}

void TRKTargetSetInputPendingPtr(void* ptr) { gTRKState.inputPendingPtr = ptr; }

u32 ConvertAddress(u32 addr) { return (addr | BOOTINFO); }

#define ACTIVE_THREAD_QUEUE (BOOTINFO + ROOT_THREAD_ADDR) // 8 bytes
#define CURRENT_THREAD (BOOTINFO + CURRENT_THREAD_ADDR) // 4 bytes

#define INVALID_THREAD(thread) ((u32)thread == 0xFFFFFFFF || thread == NULL || (u32)thread == 0x80000000)

static void GetThreadInfo(int* r3, int* r4) {
    int i;
    OSThread* thread;

    *r3 = 1;
    *r4 = 0;

    if (INVALID_THREAD(OS_THREAD_QUEUE.head)) {
        return;
    }

    i = 0;
    thread = OS_THREAD_QUEUE.head;

    while (thread != NULL) {
        if (thread == OS_CURRENT_THREAD) {
            *r4 = i;
        }

        i++;
        thread = (OSThread*)ConvertAddress((u32)thread->nextActive);
        if (INVALID_THREAD(thread)) {
            break;
        }
    }

    *r3 = i;
}
