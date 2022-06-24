#include "fy.h"

bool Fy_hadExitSignal = false;

static void Fy_HandleSignal(int signo) {
    (void)signo;
    Fy_hadExitSignal = true;
}

/* Returns whether the operation was successful */
bool Fy_SetSignalHandlers(void) {
    if (signal(SIGINT, Fy_HandleSignal) == SIG_ERR)
        return false;

    if (signal(SIGTERM, Fy_HandleSignal) == SIG_ERR)
        return false;

    return true;
}
