#ifndef NW4R_SND_WAVE_SOUND_HANDLE_H
#define NW4R_SND_WAVE_SOUND_HANDLE_H

#include "revolution/hbm/nw4hbm/snd/WaveSound.h"
#include "revolution/hbm/nw4hbm/snd/snd_Types.hpp"
#include "revolution/hbm/ut.hpp"

// forward declarations
namespace nw4hbm {
namespace snd {

class WaveSoundHandle : private ut::NonCopyable {
  public:
    ~WaveSoundHandle() { DetachSound(); }

    void DetachSound();

    bool IsAttachedSound() const { return mSound != NULL; }

  private:
    detail::WaveSound* mSound; // 0x00
};

} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_WAVE_SOUND_HANDLE_H
