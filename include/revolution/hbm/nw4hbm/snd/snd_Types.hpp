#ifndef NW4R_SND_TYPES_H
#define NW4R_SND_TYPES_H

#include "revolution/types.h"

namespace nw4hbm {
namespace snd {

static const int CHANNEL_MIN = 1;
static const int CHANNEL_MAX = 2;

static const int REMOTE_FILTER_MAX = 127;

// Volume in range [-90.4db, 6.0db]
static const f32 VOLUME_MIN_DB = -90.4f;
static const f32 VOLUME_MAX_DB = 6.0f;
static const f32 VOLUME_RANGE_DB = -(VOLUME_MIN_DB - VOLUME_MAX_DB);
static const int VOLUME_RANGE_MB = static_cast<int>(10 * VOLUME_RANGE_DB);

enum OutputLineFlag {
    OUTPUT_LINE_MAIN = (1 << 0),
    OUTPUT_LINE_REMOTE_N = (1 << 1),
};

namespace detail {

struct AdpcmParam {
    u16 coef[16]; // at 0x0
    u16 gain; // at 0x20
    u16 pred_scale; // at 0x22
    u16 yn1; // at 0x24
    u16 yn2; // at 0x26
};

struct AdpcmLoopParam {
    u16 loop_pred_scale; // at 0x0
    u16 loop_yn1; // at 0x2
    u16 loop_yn2; // at 0x4
};

struct AdpcmInfo {
    AdpcmParam param; // at 0x0
    AdpcmLoopParam loopParam; // at 0x28
    u16 PADDING_0x2E; // at 0x2E
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
