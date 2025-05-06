#ifndef _METROTRK_MUTEX_TRK_H
#define _METROTRK_MUTEX_TRK_H

#include "macros.h"
#include "metrotrk/dserror.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int DSMutex;

DSError TRKReleaseMutex(void* p1);
DSError TRKAcquireMutex(void* p1);
DSError TRKInitializeMutex(void* p1);

#ifdef __cplusplus
}
#endif

#endif
