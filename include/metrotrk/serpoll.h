#ifndef _METROTRK_SERPOLL_H
#define _METROTRK_SERPOLL_H

#include "metrotrk/msgbuf.h"
#include "metrotrk/trk.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char FCSType;

typedef enum ReceiverState {
    kWaitFlag = 0,
    kFoundFlag = 1,
    kInFrame = 2,
    kFrameOverflow = 3
} ReceiverState;

typedef struct FramingState {
    /* 0x00 */ MessageBufferID fBufferID;
    /* 0x04 */ MessageBuffer* fBuffer;
    /* 0x08 */ ReceiverState fReceiveState;
    /* 0x0C */ int fEscape;
    /* 0x10 */ FCSType fFCS;
} FramingState; // size = 0x11

void TRKGetInput(void);
void TRKProcessInput(int);
DSError TRKInitializeSerialHandler(void);
DSError TRKTerminateSerialHandler(void);

extern void* gTRKInputPendingPtr;

#ifdef __cplusplus
}
#endif

#endif
