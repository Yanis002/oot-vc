#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "emulator/errordisplay.h"
#include "emulator/system.h"
#include "emulator/xlObject.h"
#include "macros.h"
#include "revolution/pad.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ControllerStickAxis {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_MAX = 2
} ControllerStickAxis;

typedef struct ControllerThread {
    /* 0x000 */ OSThread thread;
    /* 0x318 */ u8 unk_318[0xA];
} ControllerThread; // size = 0x328

#if IS_MM
typedef struct ControllerUnknown {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ struct Store* pStore;
} ControllerUnknown; // size = 0xC
#endif

typedef struct Controller {
    /*  OoT   MM  */
#if IS_MM
    /*  N/A  0x000 */ u32 MM_unk_00;
    /*  N/A  0x004 */ ControllerUnknown* MM_unk_04[4];
#endif
    /* 0x000 0x014 */ u32 unk_00[19];
    /* 0x04C 0x060 */ s32 unk_4C[PAD_MAX_CONTROLLERS];
    /* 0x05C 0x070 */ s32 analogTriggerLeft[PAD_MAX_CONTROLLERS];
    /* 0x06C 0x080 */ s32 analogTriggerRight[PAD_MAX_CONTROLLERS];
#if IS_MM
    /*  N/A  0x090 */ u8 pad1[0x10];
#endif
    /* 0x07C 0x0A0 */ s32 stickLeft[PAD_MAX_CONTROLLERS][AXIS_MAX];
    /* 0x09C 0x0B0 */ s32 stickRight[PAD_MAX_CONTROLLERS][AXIS_MAX];
    /* 0x0BC 0x0D0 */ s32 unk_BC[PAD_MAX_CONTROLLERS];
    /* 0x0CC 0x0E0 */ s32 unk_CC[PAD_MAX_CONTROLLERS];
    /* 0x0DC 0x0F0 */ u32 controllerConfiguration[PAD_MAX_CONTROLLERS][GCN_BTN_COUNT];
    /* 0x21C 0x250 */ ErrorIndex iString;
    /* 0x220 0x254 */ s32 unk_220;
    /* 0x224 0x258 */ s32 unk_224;
    /* 0x228 0x25C */ u32 unk_228[PAD_MAX_CONTROLLERS];
    /* 0x238 0x26C */ u32 unk_238[PAD_MAX_CONTROLLERS];
    /* 0x248 0x280 */ s64 unk_248;
    /* 0x250 0x288 */ u8 unk_250[0x20];
    /* 0x270 0x2A8 */ u32 unk_270[PAD_MAX_CONTROLLERS];
    /* 0x280 0x2B8 */ u32 unk_280[PAD_MAX_CONTROLLERS];
} Controller; // size = 0x290 ; 0x2C8

s32 fn_80062028(EDString* pSTString);
bool simulatorSetControllerMap(Controller* pController, s32 channel, u32* mapData);
// bool simulatorCopyControllerMap(Controller* pController, u32* mapDataOutput, u32* mapDataInput);
bool fn_80062E5C(Controller* pController, s32, s32*) NO_INLINE;
bool simulatorDetectController(Controller* pController, s32 arg1);
bool fn_800622B8(Controller* pController) NO_INLINE;
bool fn_800623F4(Controller* pController) NO_INLINE;
bool fn_80062C18(Controller* pController, s32 iController, s32* arg2, s32* arg3, s32* arg4, s32* arg5, s32* arg6,
                 s32* arg7);
bool fn_80062CE4(Controller* pController, s32 iController, bool bUnknown);
bool fn_800631B8(Controller* pController, s32 arg1);
bool controllerEvent(Controller* pController, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassController;

#ifdef __cplusplus
}
#endif

#endif
