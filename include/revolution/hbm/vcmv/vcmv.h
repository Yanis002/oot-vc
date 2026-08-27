#ifndef VCMV_H
#define VCMV_H

#include "revolution/gx/GXTexture.h"
#include "revolution/gx/GXTypes.h"
#include "revolution/tpl/TPL.h"
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
} UnkStruct_801CA6B0; // size = 0x54

typedef struct UnkStruct_800838C0 {
    /* 00 */ UNKWORD unk_00;
    /* 04 */ UNKWORD unk_04;
    /* 08 */ UNKWORD unk_08;
    /* 0C */ UNKWORD unk_0C;
} UnkStruct_800838C0; // size = 0x10

typedef struct UnkStruct_801CA5D8_00 {
    /* 00 */ UNKWORD unk_00;
    /* 04 */ UNKWORD unk_04;
} UnkStruct_801CA5D8_00; // size = 0x08

typedef struct UnkStruct_801CA5D8 {
    /* 00 */ UnkStruct_801CA5D8_00 unk_00[4];
    /* 20 */ UnkStruct_800838C0 unk_20;
    /* 30 */ UNKWORD unk_30;
    /* 34 */ UNKWORD unk_34;
    /* 38 */ UNKWORD unk_38;
    /* 3C */ UNKWORD unk_3C;
    /* 40 */ f32 unk_40;
    /* 44 */ f32 unk_44;
    /* 48 */ f32 unk_48;
    /* 4C */ UNKWORD unk_4C;
    /* 50 */ UNKWORD unk_50;
    /* 51 */ u8 unk_51;
    /* 52 */ u8 unk_52;
} UnkStruct_801CA5D8;

typedef struct UnkStruct_8008461C_Arg4 {
    /* 00 */ char* unk_00;
    /* 04 */ char* unk_04;
    /* 08 */ UNKWORD unk_08;
} UnkStruct_8008461C_Arg4;

typedef struct UnkStruct_80175290 {
    /* 00 */ f32 unk_00;
    /* 04 */ f32 unk_04;
    /* 08 */ f32 unk_08;
    /* 0C */ f32 unk_0C;
    /* 10 */ TPLPalette* unk_10;
    /* 14 */ GXColor unk_14;
    /* 18 */
} UnkStruct_80175290;

// vcmv_cursor.cpp
void VCMV_80083070(UNKWORD param1);
void VCMV_80083140(void);
void VCMV_80083154(void);
void VCMV_80083158(void);
void VCMV_8008315C(void** param1);
void VCMV_8008345C(UNKWORD param1);

// vcmv_800838C0.cpp
UNKWORD VCMV_800838C0(UnkStruct_800838C0* param1, f32* param2);
BOOL VCMV_80083A60(UnkStruct_800838C0* param1, s32* param2);
BOOL VCMV_80083D18(UnkStruct_800838C0* param1, s32* param2);
void VCMV_80083FC4(UnkStruct_800838C0* param1, s32 param2);
void VCMV_800840B4(UnkStruct_800838C0* param1, s32 param2);
void VCMV_80085798(const UnkStack20* param1);
void VCMV_80084268();
UNKWORD VCMV_8008461C(void* param1, UNKWORD param2, UNKWORD param3, UnkStruct_8008461C_Arg4* param4);
void VCMV_80084A18(void* param1);
void VCMV_80084A34(UNKWORD param1, UNKWORD param2);
void VCMV_80084A40(u16 param1);
s32 VCMV_80084AAC(s32 param1, s32 param2);
void VCMV_80084DF4(void);
void VCMV_80084E54(void);
void VCMV_80084F34(s32 param1);
void VCMV_800855BC(GXTexObj* param1, GXColor param2);
void VCMV_80086670(void);
void VCMV_800867D0(void);

extern UnkStruct_801CA6B0 lbl_801CA6B0[];
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
