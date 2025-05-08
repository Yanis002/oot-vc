#ifndef _RVL_SDK_VI_VI3IN1_H
#define _RVL_SDK_VI_VI3IN1_H

#include "macros.h"
#include "revolution/types.h"
#include "revolution/vi/vitypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void __VISetRGBModeImm(void);

void __VISetRevolutionModeSimple(void);
void __VISetYUVSEL(VIBool outsel) NO_INLINE;
void fn_8009C5B0(VIGamma gamma);

#ifdef __cplusplus
}
#endif

#endif
