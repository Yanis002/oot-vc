#ifndef NW4R_SND_SEQ_TRACK_ALLOCATOR_H
#define NW4R_SND_SEQ_TRACK_ALLOCATOR_H

namespace nw4hbm {
namespace snd {
namespace detail {

class SeqPlayer;
class SeqTrack;

class SeqTrackAllocator {
  public:
    virtual ~SeqTrackAllocator() {} // 0x08

    /* 0x0C */ virtual SeqTrack* AllocTrack(SeqPlayer* player) = 0;
    /* 0x10 */ virtual void FreeTrack(SeqTrack* track) = 0;
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
