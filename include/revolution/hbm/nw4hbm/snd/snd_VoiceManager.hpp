#ifndef NW4R_SND_VOICE_MANAGER_H
#define NW4R_SND_VOICE_MANAGER_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // u32

#include "revolution/hbm/nw4hbm/snd/snd_Voice.hpp"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {
// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x2ffb36
class VoiceManager {
    // methods
  public:
    // instance accessors
    static VoiceManager& GetInstance();

    // methods
    void Setup(void* mem, u32 memSize);
    void Shutdown();

    Voice::LinkList const& GetVoiceList() const { return mPrioVoiceList; }
    u32 GetRequiredMemSize(int voiceCount);

    Voice* AllocVoice(int voiceChannelCount, int voiceOutCount, int priority, Voice::Callback* callback,
                      void* callbackData);
    void FreeVoice(Voice* voice);

    void StopAllVoices();

    void ChangeVoicePriority(Voice* voice);

    void UpdateAllVoices();
    void UpdateAllVoicesSync(byte4_t syncFlag);

    void NotifyVoiceUpdate();

  private:
    // cdtors
    VoiceManager();

    // methods
    void AppendVoiceList(Voice* voice);
    void RemoveVoiceList(Voice* voice);

    void UpdateEachVoicePriority(Voice::LinkList::Iterator const& beginItr, Voice::LinkList::Iterator const& endItr);

    int DropLowestPriorityVoice(int priority);

    // members
  private:
    bool mInitialized; // size 0x01, offset 0x00
    /* 3 bytes padding */
    Voice::LinkList mPrioVoiceList; // size 0x0c, offset 0x04
    Voice::LinkList mFreeVoiceList; // size 0x0c, offset 0x10
}; // size 0x1c
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_VOICE_MANAGER_H
