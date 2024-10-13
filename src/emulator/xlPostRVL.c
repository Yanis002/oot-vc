#include "emulator/xlPostRVL.h"
#include "emulator/xlCoreRVL.h"
#include "emulator/frame.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "revolution/vi.h"
#include "macros.h"

#if IS_MM

//! TODO: document these
typedef struct struct_8017B1E0 {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ void* unk_04[5];
} struct_8017B1E0; // size = 0x18

extern struct_8017B1E0 lbl_8017B1E0;
extern s32 lbl_80200654;
u32 lbl_801FF7DC = 2;
extern s32 lbl_80201700;
extern s32 lbl_80201704;

void fn_8008745C(void) {
    SYSTEM_FRAME(gpSystem)->nMode = 0;
    SYSTEM_FRAME(gpSystem)->nModeVtx = -1;
    frameDrawReset(SYSTEM_FRAME(gpSystem), 0x5FFED);

    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    GXSetColorUpdate(GX_ENABLE);
    GXCopyDisp(lbl_8017B1E0.unk_04[lbl_80200654 * 2], GX_TRUE);
    GXDrawDone();
    fn_800A42A4(rmode);
    VISetNextFrameBuffer(lbl_8017B1E0.unk_04[lbl_80200654 * 2]);
    VIFlush();
    VIWaitForRetrace();

    lbl_80200654 = (lbl_80200654 + 1) % lbl_801FF7DC;
}

bool fn_80087534(void) {
    u32 temp_r3;

    xlCoreInitGX();
    VISetNextFrameBuffer(lbl_8017B1E0.unk_04[lbl_80200654 * 2]);
    temp_r3 = lbl_80200654 + 1;
    lbl_80200654 = temp_r3;

    if (temp_r3 >= lbl_801FF7DC) {
        lbl_80200654 = 0;
    }

    VIFlush();
    VIWaitForRetrace();

    if (rmode->viTVmode & 1) {
        VIWaitForRetrace();
    }

    fn_800A42A4(rmode);
    return true;
}

bool xlPostText(const char* fmt, const char* file, s32 line, ...) { return true; }

#endif

bool xlPostSetup(void) { return true; }

bool xlPostReset(void) { return true; }
