#ifndef VCMV_H
#define VCMV_H

#include "revolution/gx/GXTexture.h"
#include "revolution/gx/GXTypes.h"
#include "revolution/hbm/HBMCommon.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/os/OSTime.h"
#include "revolution/tpl/TPL.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Rect {
    /* 00 */ f32 x0;
    /* 04 */ f32 y0;
    /* 08 */ f32 x1;
    /* 0C */ f32 y1;
} Rect; // size = 0x10

typedef struct UnkStack20 {
    /* 00 */ f32 x1;
    /* 04 */ f32 x2;
    /* 08 */ f32 x3;
    /* 0C */ f32 x4;
    /* 10 */ f32 y1;
    /* 14 */ f32 y2;
    /* 18 */ f32 y3;
    /* 1C */ f32 y4;
} UnkStack20; // size = 0x20

typedef struct UnkStruct_801CA6B0 {
    /* 00 */ u32 unk_00;
    /* 04 */ UNKWORD unk_04;
    /* 08 */ UNKWORD unk_08;
    /* 0C */ UNKWORD unk_0C;
    /* 10 */ UNKWORD unk_10;
    /* 14 */ UNKWORD unk_14;
    /* 18 */ UNKWORD unk_18;
    /* 1C */ f32 unk_1C;
    /* 20 */ f32 unk_20;
    /* 24 */ f32 unk_24;
    /* 28 */ f32 unk_28;
    /* 2C */ f32 unk_2C;
    /* 30 */ f32 unk_30;
    /* 34 */ f32 unk_34;
    /* 38 */ f32 unk_38;
    /* 3C */ f32 unk_3C;
    /* 40 */ UNKWORD unk_40;
    /* 44 */ UNKWORD unk_44;
    /* 48 */ UNKWORD unk_48;
    /* 4C */ UNKWORD unk_4C;
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
} UnkStruct_801CA5D8; // size = 0x54

typedef struct UnkStruct_8008461C_Arg4 {
    /* 00 */ char* unk_00;
    /* 04 */ char* unk_04;
    /* 08 */ UNKWORD unk_08;
} UnkStruct_8008461C_Arg4; // size = 0x0C

typedef struct UnkStruct_80175290 {
    /* 00 */ f32 unk_00;
    /* 04 */ f32 unk_04;
    /* 08 */ f32 unk_08;
    /* 0C */ f32 unk_0C;
    /* 10 */ TPLPalette* unk_10;
    /* 14 */ GXColor unk_14;
} UnkStruct_80175290; // size = 0x18

// vcmv_cursor.cpp
void VCMV_80083070(UNKWORD param1);
void VCMV_80083140(void);
void VCMV_80083154(void);
void VCMV_80083158(void);
void VCMV_8008315C(void** param1);
void VCMV_8008345C(UNKWORD param1);

extern s32 lbl_8025D224;

// vcmv_800838C0.cpp
UNKWORD VCMV_800838C0(UnkStruct_800838C0* param1, f32* param2);
BOOL VCMV_80083A60(UnkStruct_800838C0* param1, s32* param2);
BOOL VCMV_80083D18(UnkStruct_800838C0* param1, s32* param2);
void VCMV_80083FC4(UnkStruct_800838C0* param1, s32 param2);
void VCMV_800840B4(UnkStruct_800838C0* param1, s32 param2);
void VCMV_80084268(void);
UNKWORD VCMV_8008461C(void* param1, UNKWORD param2, UNKWORD param3, UnkStruct_8008461C_Arg4* param4);
void VCMV_80084A18(void* param1);
void VCMV_80084A34(UNKWORD param1, UNKWORD param2);
void VCMV_80084A40(u16 param1);
s32 VCMV_80084AAC(s32 param1, s32 param2);
void VCMV_80084DF4(void);
void VCMV_80084E54(void);
void VCMV_80084F34(s32 param1);
void VCMV_80085164(void);
void VCMV_800855BC(GXTexObj* param1, GXColor param2);
void VCMV_80085798(const UnkStack20* param1);
void VCMV_80085874(void);
void VCMV_80086670(void);
void VCMV_800867D0(void);

extern char* lbl_8025D228;
extern char* lbl_8025D22C;
extern u32* lbl_8025D230;
extern u16 lbl_8025D23C;
extern u16 lbl_8025D23E;
extern u16 lbl_8025D240;
extern u16 lbl_8025D242;
extern f32 lbl_8025D248;
extern UNKWORD lbl_8025D258;
extern UNKWORD lbl_8025D260;
extern bool lbl_8025D26E;
extern bool lbl_8025D26F;
extern bool lbl_8025D279;
extern s8 lbl_8025D280;
extern volatile bool lbl_8025D288;
extern f32 lbl_8025C8E8;
extern f32 lbl_8025C8EC;
extern UNKWORD lbl_8025C8F0;

// vcmv_javascript.cpp
typedef struct UnkStruct_CommonFuncArg {
    /* 00 */ UNKWORD unk_00;
    /* 04 & 08 */ union {
        f64 value_double;
        void* ptr;
        s32 value_int;
    };
} UnkStruct_CommonFuncArg; // size = 0x0C

typedef struct UnkStruct_8025D298 {
    /* 00 */ UNKWORD (*unk_00)(UNKWORD, UNKWORD, UNKWORD, void*, void*, const char*, UNKWORD, void*);
    /* 04 */ UNKWORD (*unk_04)(UNKWORD, void*, UNKWORD, UNKWORD, void*);
} UnkStruct_8025D298;

typedef struct UnkStruct_801CA650 {
    /* 00 */ const char** unk_00;
    /* 00 */ UNKWORD unk_04;
    /* 00 */ void* unk_08;
    /* 00 */ UNKWORD unk_0C;
    /* 00 */ UNKWORD unk_10;
    /* 00 */ UNKWORD unk_14;
    /* 00 */ UNKWORD unk_18;
    /* 00 */ void* unk_1C;
} UnkStruct_801CA650;

typedef struct UnkStruct_8025D2B0 {
    /* 00 */ UNKWORD unk_00;
    /* 04 */ UNKWORD unk_04;
    /* 08 */ UNKWORD unk_08;
} UnkStruct_8025D2B0;

s32 VCMV_800867DC();
void VCMV_800867E4(void);
s32 VCMV_80086864(UNKWORD param1, const char* param2, UnkStruct_CommonFuncArg* param3);
s32 VCMV_8008691C(UNKWORD param1, s32 param2, int param3, UNKWORD param4, UnkStruct_CommonFuncArg* param5);
s32 VCMV_800869A8(s32 param1, const char* param2, UnkStruct_CommonFuncArg* param3);
s32 VCMV_80086CCC(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4);
s32 VCMV_80086D08(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4);
s32 VCMV_80086D60(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4);
s32 VCMV_80086DB4(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4);
s32 VCMV_80086E08(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4);
UNKWORD VCMV_80086E18(void);
UNKWORD VCMV_80086E20(void);
void VCMV_80086E28(void);

extern bool lbl_8025D2A4;

// vcmv_main.cpp
typedef void (*Callback_8025D2D0)(s32, s32, u16, u16, s32, u32);

void VCMV_80086E38(volatile UnkStruct_801CA6B0* param1);
void VCMV_80086FDC(volatile UnkStruct_801CA6B0* param1, KPADStatus* param2);
void VCMV_800870E8(volatile UnkStruct_801CA6B0* param1, KPADStatus* param2);
void VCMV_8008725C(int index);
void VCMV_80087654(void);
void VCMV_80087734(s32 param1, s32 param2, s32 param3, s32 param4, volatile UnkStruct_801CA6B0* param5);
void VCMV_80087918(s32 param1, s32 param2, s32 param3, volatile UnkStruct_801CA6B0* param4);
void VCMV_800879E8(void);
bool VCMV_80087E34(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2);
void VCMV_80087EE4(void** param1);
void VCMV_80087F3C(void);
void VCMV_80088090(OSAlarm* alarm, OSContext* ctx);
void* VCMV_80088098(void* arg);
char* VCMV_8008844C(Callback_8025D2D0 param1, char* param2, u8 param3);
void VCMV_80088654(MEMAllocator* param1, MEMAllocator* param2);
void VCMV_80088660(void);
void VCMV_80088664(void);
void VCMV_80088668(UNKWORD param1, UNKWORD param2);
void VCMV_8008866C(s32 param1, s32 param2);
void VCMV_80088670(u16 param1);
void VCMV_80088674(void);
bool VCMV_80088678(u32 param1);
void VCMV_8008876C(void);
void VCMV_800887C4(void* param1);
char* VCMVRun(Callback_8025D2D0 param1, char* param2, u8 param3);
void VCMV_800887CC(char* param1);
void VCMV_800887D4(UNKWORD param1);
bool VCMV_8008882C(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2);
void VCMV_800888DC(void** param1);
void VCMV_80088934(void);
void VCMV_80088994(void** param1);
BOOL NETGetUniversalCalendar(OSCalendarTime* time);

extern HBMControllerData lbl_801CA670;
extern volatile UnkStruct_801CA6B0 lbl_801CA6B0[4];
extern volatile bool lbl_8025D2BE;
extern volatile bool lbl_8025D2BF;
extern bool lbl_8025D2C0;
extern u8 lbl_8025D2C1;
extern u8 lbl_8025D2C2;
extern UNKWORD lbl_8025D2C4;
extern MEMAllocator* lbl_8025D2C8;
extern MEMAllocator* lbl_8025D2CC;
extern u8 lbl_8025D2D4;
extern volatile UNKWORD lbl_8025D2D8;
extern bool lbl_8025D2DC;
extern UNKWORD lbl_8025D2E0;
extern UNKWORD lbl_8025D2E8;
extern UNKWORD lbl_8025D2FC;

#ifdef __cplusplus
}
#endif

#endif
