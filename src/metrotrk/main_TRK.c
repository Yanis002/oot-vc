#include "metrotrk/main_TRK.h"
#include "metrotrk/mainloop.h"

static int TRK_mainError;

int TRK_main(void) {
    TRK_mainError = TRKInitializeNub();

    if (!TRK_mainError) {
        TRKNubWelcome();
        TRKNubMainLoop();
    }

    TRK_mainError = TRKTerminateNub();

    return TRK_mainError;
}
