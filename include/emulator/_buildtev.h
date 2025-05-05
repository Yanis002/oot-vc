#ifndef _BUILDTEV_H
#define _BUILDTEV_H

#include "revolution/gx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TevOrder {
    /* 0x0 */ GXTexCoordID coordID;
    /* 0x4 */ GXTexMapID mapID;
    /* 0x8 */ GXChannelID chanID;
} TevOrder; // size = 0xC

typedef struct TevColorOp {
    /* 0x00 */ GXTevOp op;
    /* 0x04 */ GXTevBias bias;
    /* 0x08 */ GXTevScale scale;
    /* 0x0C */ u8 clamp;
    /* 0x10 */ GXTevRegID out_reg;
} TevColorOp; // size = 0x14

typedef struct CombineModeTev {
    /* 0x000 */ u32 ccCodes[2][2];
    /* 0x010 */ u8 numCycles;
    /* 0x011 */ u8 numStages;
    /* 0x012 */ u8 numTexGen;
    /* 0x013 */ u8 numChan;
    /* 0x014 */ u32 flags;
    /* 0x018 */ TevOrder tevOrder[9];
    /* 0x084 */ TevColorOp tevColorOpP[9][2];
    /* 0x1F8 */ GXTevColorArg tevColorArg[9][4];
    /* 0x288 */ GXTevAlphaArg tevAlphaArg[9][4];
} CombineModeTev; // size = 0x2B8

void SetColor(u8* stageValues, u32 colorVal, u8 cycle);
void SetAlpha(u8* stageValues, u32 alphaVal, u8 cycle, u32 colorVal, u32 color2Val, u32 alpha2Val);
s32 SetupStage(CombineModeTev* tvP, u8* stageValues, s32 type);
void BuildCycle(CombineModeTev* tvP, u8 (*stageValues)[4]);
CombineModeTev* BuildCombineModeTev(u32 color1, u32 alpha1, u32 color2, u32 alpha2, u32 numCycles, u32 arg5, u32 arg6);

extern GXTevAlphaArg gAlphaArgs[10];
extern GXTevColorArg gColorArgs[16];

#ifdef __cplusplus
}
#endif

#endif
