#ifndef NW4R_SND_DISPOSE_CALLBACK_MANAGER_H
#define NW4R_SND_DISPOSE_CALLBACK_MANAGER_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // u32

#include "revolution/hbm/nw4hbm/ut/LinkList.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {
// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x26773
class DisposeCallback {
    // typedefs
  public:
    typedef ut::LinkList<DisposeCallback, 0x00> LinkList;

    // methods
  public:
    // cdtors
    /* virtual ~DisposeCallback() {} */ // virtual function ordering

    // members
  public:
    ut::LinkListNode mDisposeLink; // size 0x08, offset 0x00
    /* vtable */ // size 0x04, offset 0x08

    // late virtual methods
  public:
    // virtual function ordering
    // vtable DisposeCallback
    virtual ~DisposeCallback() {}
    virtual void InvalidateData(void const* pStart, void const* pEnd) = 0;
    virtual void InvalidateWaveData(void const* pStart, void const* pEnd) = 0;
}; // size 0x0c

// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x2bb987
class DisposeCallbackManager {
    // methods
  public:
    // instance accessors
    static DisposeCallbackManager& GetInstance();

    // methods
    void RegisterDisposeCallback(DisposeCallback* callback);
    void UnregisterDisposeCallback(DisposeCallback* callback);

    void Dispose(void* mem, u32 size, void* arg);
    void DisposeWave(void* mem, u32 size, void* arg);

  private:
    // cdtors
    DisposeCallbackManager();

    // members
  private:
    DisposeCallback::LinkList mCallbackList; // size 0x0c, offset 0x00
}; // size 0x0c
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_DISPOSE_CALLBACK_MANAGER_H