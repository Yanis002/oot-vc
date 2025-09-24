#ifndef NW4R_SND_DISPOSE_CALLBACK_MANAGER_H
#define NW4R_SND_DISPOSE_CALLBACK_MANAGER_H

#include "revolution/hbm/nw4hbm/snd/DisposeCallback.h"
#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include "revolution/types.h" // u32

namespace nw4hbm {
namespace snd {
namespace detail {
class DisposeCallbackManager {
  public:
    static DisposeCallbackManager& GetInstance();

    void RegisterDisposeCallback(DisposeCallback* callback);
    void UnregisterDisposeCallback(DisposeCallback* callback);

    static void Dispose(void* mem, u32 size, void* arg);
    static void DisposeWave(void* mem, u32 size, void* arg);

  private:
    DisposeCallbackManager();

    DisposeCallbackList mCallbackList; // 0x00
};
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_DISPOSE_CALLBACK_MANAGER_H