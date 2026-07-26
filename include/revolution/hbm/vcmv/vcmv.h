#ifndef VCMV_H
#define VCMV_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UnkStack20 {
    f32 x1, x2, x3, x4;
    f32 y1, y2, y3, y4;
} UnkStack20;

typedef struct UnkStruct_801CA6B0 {
    /* 00 */ u32 unk_00;
    /* 04 */ char pad1[0x2C - 0x04];
    /* 2C */ f32 unk_2C;
    /* 30 */ f32 unk_30;
    /* 34 */ f32 unk_34;
    /* 38 */ f32 unk_38;
    /* 3C */ char pad3[0x50 - 0x3C];
    /* 50 */ bool unk_50;
    /* 51 */ char pad2[0x54 - 0x51];
} UnkStruct_801CA6B0; // size = 0x54

extern UnkStruct_801CA6B0 lbl_801CA6B0[];

// vcmv_cursor.cpp
void VCMV_80083070(UNKWORD param1);
void VCMV_80083140(void);
void VCMV_80083154(void);
void VCMV_80083158(void);
void VCMV_8008315C(void** param1);
void VCMV_8008345C(UNKWORD param1);

// unk
void VCMV_80085798(UnkStack20* param1);

extern s32 lbl_8025D224;
extern bool lbl_8025D2BF;
extern u8 lbl_8025D2D4;
extern volatile UNKWORD lbl_8025D2D8;
extern bool lbl_8025D2DC;
extern UNKWORD lbl_8025D2E0;

#ifdef __cplusplus
}
#endif

#endif
