#ifndef NW4R_SND_TASK_H
#define NW4R_SND_TASK_H

#include "revolution/hbm/nw4hbm/ut.h"
#include "revolution/hbm/nw4hbm/ut/LinkList.h"

namespace nw4hbm {
namespace snd {
namespace detail {

class Task : private ut::NonCopyable {
    friend class TaskManager;

public:
    Task() :
        mTaskId(0) {}

    Task(u32 taskId) :
        mTaskId(taskId) {}

    virtual ~Task() {} // 0x08

    /* 0x0C */ virtual void Execute() = 0;
    /* 0x10 */ virtual void Cancel() = 0;

    u32 GetTaskId() const { return mTaskId; }

public:
    /* 0x04 */ ut::LinkListNode mTaskLink;

private:
    /* 0x0C */ u32 mTaskId;
};

typedef ut::LinkList<Task, offsetof(Task, mTaskLink)> TaskList;

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
