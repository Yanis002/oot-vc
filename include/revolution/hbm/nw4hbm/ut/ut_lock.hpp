#ifndef NW4R_UT_LOCK_H
#define NW4R_UT_LOCK_H
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"

#include "revolution/os.h"

namespace nw4hbm {
namespace ut {
namespace detail {

/******************************************************************************
 *
 * Mutex lock functions
 *
 ******************************************************************************/
inline void Lock(OSMutex& rMutex) { OSLockMutex(&rMutex); }
inline void Unlock(OSMutex& rMutex) { OSUnlockMutex(&rMutex); }

/******************************************************************************
 *
 * AutoLock
 *
 ******************************************************************************/
template <typename T> class AutoLock : private NonCopyable {
  public:
    explicit AutoLock(T& rLockObj) : mLockObj(rLockObj) { Lock(mLockObj); }
    ~AutoLock() { Unlock(mLockObj); }

  private:
    T& mLockObj; // at 0x0
};

} // namespace detail

/******************************************************************************
 *
 * AutoInterruptLock
 *
 ******************************************************************************/
class AutoInterruptLock : private NonCopyable {
  public:
    AutoInterruptLock() : mOldState(OSDisableInterrupts()) {}
    ~AutoInterruptLock() { OSRestoreInterrupts(mOldState); }

  private:
    bool mOldState; // at 0x0
};

} // namespace ut
} // namespace nw4hbm

#endif
