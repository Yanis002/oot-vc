#ifndef NW4R_SND_ENV_GENERATOR_H
#define NW4R_SND_ENV_GENERATOR_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {

// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x28b11
class EnvGenerator {
    // enums
  public:
    // [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x28a77
    enum Status {
        STATUS_ATTACK,
        // STATUS_HOLD,
        STATUS_DECAY,
        STATUS_SUSTAIN,
        STATUS_RELEASE,
    };

    // methods
  public:
    // cdtors
    EnvGenerator();

    // methods
    void Init();
    void Update(int msec);
    void Reset();

    Status GetStatus() const { return mStatus; }
    f32 GetValue() const;

    void SetStatus(Status status) { mStatus = status; }
    void SetAttack(int attack);
    void SetHold(int hold);
    void SetDecay(int decay);
    void SetSustain(int sustain);
    void SetRelease(int release);

  private:
    f32 CalcRelease(int release);
    s16 CalcDecibelSquare(int scale);

    // static members
  public:
    static int const CALC_DECIBEL_SCALE_MAX;

    static int const DECIBEL_SQUARE_TABLE_SIZE = 128;

    static int const RELEASE_INIT = 127;
    static int const SUSTAIN_INIT = 127;
    static int const DECAY_INIT = 127;
    static int const ATTACK_INIT = 127;

  private:
    static s16 const DecibelSquareTable[DECIBEL_SQUARE_TABLE_SIZE];

    // members
  private:
    Status mStatus; // size 0x04, offset 0x00
    f32 mValue; // size 0x04, offset 0x04
    f32 mDecay; // size 0x04, offset 0x08
    f32 mRelease; // size 0x04, offset 0x0c
    f32 mAttack; // size 0x04, offset 0x10
    u8 mSustain; // size 0x01, offset 0x14
    /* 2 bytes padding */
}; // size 0x1c

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_ENV_GENERATOR_H
