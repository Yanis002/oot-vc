#ifndef _METROTRK_NUBEVENT_H
#define _METROTRK_NUBEVENT_H

#include "metrotrk/dserror.h"
#include "metrotrk/msgbuf.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long NubEventID;

typedef enum NubEventType {
    kNullEvent = 0,
    kShutdownEvent = 1,
    kRequestEvent = 2,
    kBreakpointEvent = 3,
    kExceptionEvent = 4,
    kSupportEvent = 5
} NubEventType;

typedef struct NubEvent {
    /* 0x00 */ NubEventType fType;
    /* 0x04 */ NubEventID fID;
    /* 0x08 */ MessageBufferID fMessageBufferID;
} NubEvent; // size = 0xC

DSError TRKInitializeEventQueue(void);
bool TRKGetNextEvent(NubEvent*);
DSError TRKPostEvent(NubEvent*);
void TRKConstructEvent(NubEvent*, int);
void TRKDestructEvent(NubEvent*);

#ifdef __cplusplus
}
#endif

#endif
