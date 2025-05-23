#include "metrotrk/mainloop.h"
#include "metrotrk/dispatch.h"
#include "metrotrk/msgbuf.h"
#include "metrotrk/nubevent.h"
#include "metrotrk/serpoll.h"

void TRKNubMainLoop(void) {
    MessageBuffer* msg;
    NubEvent event;
    bool var_r31 = false;
    bool var_r30 = false;

    while (var_r31 == false) {
        if (TRKGetNextEvent(&event) != false) {
            var_r30 = false;
            switch (event.fType) {
                case kNullEvent:
                    break;
                case kRequestEvent:
                    msg = TRKGetBuffer(event.fMessageBufferID);
                    TRKDispatchMessage(msg);
                    break;
                case kShutdownEvent:
                    var_r31 = true;
                    break;
                case kBreakpointEvent:
                case kExceptionEvent:
                    TRKTargetInterrupt(&event);
                    break;
                case kSupportEvent:
                    TRKTargetSupportRequest();
                    break;
            }
            TRKDestructEvent(&event);
        } else if (var_r30 == false || *(u8*)gTRKInputPendingPtr != 0) {
            var_r30 = true;
            TRKGetInput();
        } else {
            if (TRKTargetStopped() == false) {
                TRKTargetContinue();
            }
            var_r30 = false;
        }
    }
}
